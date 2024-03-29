#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>

#define MAX_CLASSES 3
#define MAX_APPRENANTS 100
#define MAX_LENGTH 100

//-------------------------------------------
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

char getch()
{
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

#define LONGUEUR_MAX_LOGIN 10
#define LONGUEUR_MAX_MDP 10
#define MAX_STUDENTS_PER_CLASS 50

typedef struct
{
    int jour;
    int mois;
    int annee;
} Date;

typedef struct
{
    char login[LONGUEUR_MAX_LOGIN];
    char motDePasse[LONGUEUR_MAX_MDP];
} Identifiants;

typedef struct {
    char matricule[10];
    char motdepasse[100];
    char prenom[30];
    char nom[20];
    char classe[6];
    int retard_cumule;
    int statut;
} Apprenant;


typedef struct
{
    int id;
    int status;
    char matricule[10];
    char dateHeur[20];
    char contenu[1000];
} Message;

Identifiants identifiantsAdmin;
int nombreIdentifiantsAdmin = 1;

void enregistrerPresence(char *matricule) {
    FILE *fichierPresence = fopen("presence.txt", "r");
    if (fichierPresence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    // Récupérer la date actuelle
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    int jour = timeinfo->tm_mday;
    int mois = timeinfo->tm_mon + 1;      // Le mois commence à 0, donc on ajoute 1
    int annee = timeinfo->tm_year + 1900; // L'année est le nombre d'années depuis 1900
    fclose(fichierPresence);

    // Vérifier si l'étudiant est déjà marqué présent à la date actuelle
    fichierPresence = fopen("presence.txt", "r");
    if (fichierPresence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }
    char matricule_presence[10];
    int jour_presence, mois_presence, annee_presence, h_presence, mn_presence, s_presence;
    int present = 0;
    while (fscanf(fichierPresence, "%s %d/%d/%d %d:%d:%d", matricule_presence, &jour_presence, &mois_presence, &annee_presence, &h_presence, &mn_presence, &s_presence) != EOF) {
            if (strcmp(matricule_presence, matricule) == 0 && jour_presence == jour && mois_presence == mois && annee_presence == annee)
            {
                printf("\n--- ❌ L'étudiant de matricule %s est déjà marqué présent aujourd'hui.\n", matricule);
                present = 1;
                break;
            }
        }
        fclose(fichierPresence);
        if (!present)
        {
                FILE *fichier = fopen("presence.txt", "a");
                if (fichier == NULL)
                {
                    printf("Erreur lors de l'ouverture du fichier de présence.\n");
                    return;
                }

                // Récupérer l'heure actuelle
                int heure = timeinfo->tm_hour;
                int minute = timeinfo->tm_min;
                int seconde = timeinfo->tm_sec;

                // Écrire dans le fichier la date et l'heure actuelles
                fprintf(fichier, "%s %d/%d/%d %02d:%02d:%02d\n", matricule, jour, mois, annee, heure, minute, seconde);
                fclose(fichier);
                printf("\n--- ✅ Presence marquee pour l'etudiant de matricule %s\n", matricule);
        }
}

void generer_fichier_par_date(int jour, int mois, int annee)
{
    FILE *fichier_presence = fopen("presence.txt", "r");
    if (fichier_presence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    char nom_fichier[30];
    sprintf(nom_fichier, "Présence_%d_%02d_%02d", annee, mois, jour);

    FILE *fichier_apprenants = fopen(nom_fichier, "w");
    if (fichier_apprenants == NULL)
    {
        printf("Erreur lors de la création du fichier d'apprenants pour la date spécifiée.\n");
        fclose(fichier_presence);
        return;
    }

    // En-tête du tableau
    fprintf(fichier_apprenants, "+------------+-------------+----------+------------+--------------+\n");
    fprintf(fichier_apprenants, "| Matricule  | Nom      | Préom      | Classe     |     Heure     |\n");
    fprintf(fichier_apprenants, "+------------+-------------+----------+------------+--------------+\n");

    char matricule_presence[10];
    int jour_presence, mois_presence, annee_presence;
    char heure[15];
    int aucun_present = 1; // Indicateur pour vérifier s'il y a au moins un étudiant présent

    // Lire tous les étudiants présents à cette date
    while (fscanf(fichier_presence, "%s %d/%d/%d %s", matricule_presence, &jour_presence, &mois_presence, &annee_presence, heure) != EOF)
    {
        if (jour_presence == jour && mois_presence == mois && annee_presence == annee)
        {
            // Lire les détails de l'apprenant
            FILE *fichier_etudiant = fopen("etudiant.txt", "r");
            if (fichier_etudiant == NULL)
            {
                printf("Erreur lors de l'ouverture du fichier des étudiants.\n");
                fclose(fichier_presence);
                fclose(fichier_apprenants);
                return;
            }

            char matricule_etudiant[10];
            char nom[20], prenom[20], classe[10];
            // Parcourir tous les étudiants
            while (fscanf(fichier_etudiant, "%s %*s %s %s %s %*d %*s", matricule_etudiant, nom, prenom, classe) != EOF)
            {
                if (strcmp(matricule_presence, matricule_etudiant) == 0)
                {
                    // Écrire les détails de l'apprenant dans le fichier
                    fprintf(fichier_apprenants, "| %-10s | %-10s | %-8s | %-10s |   %-10s   |\n", matricule_presence, prenom, nom, classe, heure);
                    aucun_present = 0; // Indiquer qu'au moins un étudiant est présent
                }
            }
            fclose(fichier_etudiant);
        }
    }

    // Pied de tableau
    fprintf(fichier_apprenants, "+------------+-------------+----------+------------+---------+\n");

    fclose(fichier_presence);
    fclose(fichier_apprenants);

    if (aucun_present)
    {
        remove(nom_fichier); // Supprimer le fichier si aucun étudiant n'a été enregistré
        printf("Aucun apprenant présent à la date spécifiée.\n");
    }
    else
    {
        printf("Fichier généré avec succès : %s\n", nom_fichier);
    }
}


// Fonction pour vérifier si une année est bissextile
int est_bissextile(int annee)
{
    if ((annee % 4 == 0 && annee % 100 != 0) || (annee % 400 == 0))
    {
        return 1; //
    }
    else
    {
        return 0;
    }
}

int est_date_valide(int jour, int mois, int annee) {
    // Vérification de la validité de la date
    if (mois < 1 || mois > 12)
        return 0; // Mois invalide

    if (jour < 1)
        return 0; // Jour invalide

    if ((mois == 2 && est_bissextile(annee) && jour > 29) || (mois == 2 && !est_bissextile(annee) && jour > 28))
        return 0; // Février avec jour invalide

    if ((mois == 4 || mois == 6 || mois == 9 || mois == 11) && jour > 30)
        return 0; // Mois avec 30 jours et jour invalide

    if (jour > 31)
        return 0;

    return 1;
}

void saisir_et_verifier_date() {
    int jour, mois, annee;
    int result;

    do
    {
        printf("Entrez une date au format jj/mm/aaaa : ");
        result = scanf("%d/%d/%d", &jour, &mois, &annee);

        // Vérification de la saisie
        if (result != 3)
        {
            printf("Format de date invalide ou saisie incorrecte.\n");
            // Nettoyage du buffer d'entrée en cas de saisie incorrecte
            while (getchar() != '\n')
                ;
            continue; // Reprend la boucle pour une nouvelle saisie
        }

        // Vérification de la validité de la date
        if (!est_date_valide(jour, mois, annee))
        {
            printf("Date invalide.\n");
            continue; // Reprend la boucle pour une nouvelle saisie
        }

        // Sortir de la boucle si la date est valide
        break;

    } while (1); // Boucle infinie jusqu'à ce qu'une date valide soit saisie

    // Une fois que la date valide est saisie, appeler la fonction pour générer le fichier
    generer_fichier_par_date(jour, mois, annee);
}


void verifier_presence_et_generer_fichier() {
    FILE *fichier_presence = fopen("presence.txt", "r");
    if (fichier_presence == NULL) {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    FILE *fichier_etudiants = fopen("etudiant.txt", "r");
    if (fichier_etudiants == NULL) {
        printf("Erreur lors de l'ouverture du fichier des étudiants.\n");
        fclose(fichier_presence);
        return;
    }

    // Obtenir la date actuelle
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date[20];
    strftime(date, sizeof(date), "%d-%m-%Y", tm); // Format de la date : JJ-MM-AAAA

    char nom_fichier_sortie[50];
    sprintf(nom_fichier_sortie, "Présence_Globale.txt");

    FILE *fichier_sortie = fopen(nom_fichier_sortie, "w");
    if (fichier_sortie == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de sortie.\n");
        fclose(fichier_presence);
        fclose(fichier_etudiants);
        return;
    }
    printf("\n");
    char matricule_presence[10], matricule_etudiant[10], nom[20], prenom[20], classe[10];
    int jour, mois, annee;
    char heure[15];

    char date_precedente[20] = ""; // Stockage de la date précédente
    int premier_tableau = 1;       // Indicateur pour vérifier si c'est le premier tableau

    while (fscanf(fichier_presence, "%s %d/%d/%d %s", matricule_presence, &jour, &mois, &annee, heure) != EOF){
        // Vérifier si la date actuelle est différente de la date précédente
        char date_actuelle[20];
        sprintf(date_actuelle, "%02d/%02d/%04d", jour, mois, annee);
        if (strcmp(date_actuelle, date_precedente) != 0)
        {
            // Si ce n'est pas le premier tableau, fermer le précédent
            if (!premier_tableau)
            {
                fprintf(fichier_sortie, "+------------+-------------+----------+------------+------------+\n\n");
            }

            // Commencer un nouveau tableau avec la date comme titre
            fprintf(fichier_sortie, "Liste de présence : %s\n", date_actuelle);
            fprintf(fichier_sortie, "+------------+------------------+---------------+------------+------------+\n");
            fprintf(fichier_sortie, "| Matricule  |      Heure       |    Prénom     |   Nom      |  Classe    |\n");
            fprintf(fichier_sortie, "+------------+------------------+---------------+------------+------------+\n");

            // Mettre à jour la date précédente
            strcpy(date_precedente, date_actuelle);
            premier_tableau = 0; // Marquer que ce n'est plus le premier tableau
        }

        // Recherche de l'étudiant correspondant au matricule de présence
        rewind(fichier_etudiants);
        while (fscanf(fichier_etudiants, "%s %*s %s %s %s %*d %*s", matricule_etudiant, nom, prenom, classe) != EOF)
        {
            if (strcmp(matricule_presence, matricule_etudiant) == 0)
            {
                fprintf(fichier_sortie, "| %-10s |  %s    | %-7s       | %-8s   | %-6s     |\n", matricule_presence, heure, nom, prenom, classe);
                break;
            }
        }
    }

    fprintf(fichier_sortie, "+------------+-------------+-----------------+------------+---------------+\n");

    fclose(fichier_sortie);
    fclose(fichier_presence);
    fclose(fichier_etudiants);
    printf("\n");
    printf("\nLe fichier de liste de présence a été généré avec succès.\n");
    printf("\n");
    printf("Appuyez sur une touche pour continuer...\n");
    getchar();
}

//-------------------- Présence dans le mois ------------------------
void genererPresenceDansLeMois() {
    int mois_souhaite;
    printf("Entrez le mois pour lequel vous souhaitez générer le fichier de présence : ");
    scanf("%d", &mois_souhaite);

    FILE *fichier_presence = fopen("presence.txt", "r");
    if (fichier_presence == NULL) {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    FILE *fichier_etudiants = fopen("etudiant.txt", "r");
    if (fichier_etudiants == NULL) {
        printf("Erreur lors de l'ouverture du fichier des étudiants.\n");
        fclose(fichier_presence);
        return;
    }

    // Obtenir la date actuelle
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date[20];
    strftime(date, sizeof(date), "%d-%m-%Y", tm); // Format de la date : JJ-MM-AAAA

    char nom_fichier_sortie[50];
    sprintf(nom_fichier_sortie, "Présence_Mois_%d.txt", mois_souhaite);

    FILE *fichier_sortie = fopen(nom_fichier_sortie, "w");
    if (fichier_sortie == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de sortie.\n");
        fclose(fichier_presence);
        fclose(fichier_etudiants);
        return;
    }

    char matricule_presence[10], matricule_etudiant[10], nom[20], prenom[20], classe[10];
    int jour, mois, annee;
    char heure[15];

    fprintf(fichier_sortie, "+------------+------------------+-------------+------------+------------+\n");
    fprintf(fichier_sortie, "| Matricule  |      Heure       |    Prénom   |   Nom      |  Classe    |\n");
    fprintf(fichier_sortie, "+------------+------------------+-------------+------------+------------+\n");

    while (fscanf(fichier_presence, "%s %d/%d/%d %s", matricule_presence, &jour, &mois, &annee, heure) != EOF){
        // Vérifier si la date actuelle est dans le mois souhaité
        if (mois == mois_souhaite) {
            // Recherche de l'étudiant correspondant au matricule de présence
            rewind(fichier_etudiants);
            while (fscanf(fichier_etudiants, "%s %*s %s %s %s %*d %*s", matricule_etudiant, nom, prenom, classe) != EOF)
            {
                if (strcmp(matricule_presence, matricule_etudiant) == 0)
                {
                    fprintf(fichier_sortie, "| %-10s |  %s        | %-7s     | %-8s   | %-6s     |\n", matricule_presence, heure, nom, prenom, classe);
                    break;
                }
            }
        }
    }

    fprintf(fichier_sortie, "+------------+------------------+-------------+------------+------------+\n");
    fclose(fichier_sortie);
    fclose(fichier_presence);
    fclose(fichier_etudiants);
    printf("\nLe fichier de liste de présence pour le mois %d a été généré avec succès.\n", mois_souhaite);
}


void calculerRetardCumule() {
    FILE *fichierPresence = fopen("presence.txt", "r");
    if (fichierPresence == NULL) {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    FILE *fichierSortie = fopen("Retards_cumules.txt", "w");
    if (fichierSortie == NULL) {
        printf("Erreur lors de l'ouverture du fichier de sortie.\n");
        fclose(fichierPresence);
        return;
    }

    Apprenant etudiants[MAX_APPRENANTS];
    int nb_etudiants = 0;

    char matricule_presence[10];
    int jour_presence, mois_presence, annee_presence, h_presence, mn_presence, s_presence;

    while (fscanf(fichierPresence, "%s %d/%d/%d %d:%d:%d", matricule_presence, &jour_presence, &mois_presence, &annee_presence, &h_presence, &mn_presence, &s_presence) != EOF) {
        int heure_arrivee = h_presence * 60 + mn_presence;
        int heure_depart = 8 * 60;

        if (heure_arrivee > heure_depart) {
            int retard = heure_arrivee - heure_depart;
            int etudiant_trouve = 0;

            for (int i = 0; i < nb_etudiants; i++) {
                if (strcmp(matricule_presence, etudiants[i].matricule) == 0) {
                    etudiants[i].retard_cumule += retard;
                    etudiant_trouve = 1;
                    break;
                }
            }

            if (!etudiant_trouve) {
                strcpy(etudiants[nb_etudiants].matricule, matricule_presence);
                etudiants[nb_etudiants].retard_cumule = retard;
                nb_etudiants++;
            }
        }
    }

    if (nb_etudiants == 0) {
        fprintf(fichierSortie, "\nAucun retard cumulé pour le moment.\n");
    } else {
        fprintf(fichierSortie, "----------------------------------------\n");
        fprintf(fichierSortie, "| Retards cumulés pour chaque étudiant |\n\n");
        fprintf(fichierSortie, "----------------------------------------\n");
        fprintf(fichierSortie, "Matricule\t   Retard cumulé (minutes)\n");
        fprintf(fichierSortie, "-----------\t   ------------------------\n");
        for (int i = 0; i < nb_etudiants; i++) {
            fprintf(fichierSortie, "%s\t\t       %d\n", etudiants[i].matricule, etudiants[i].retard_cumule);
            fprintf(fichierSortie, "_______________________________________\n");
        }
        printf("\n----- ✅ Les cumules de retard des apprenants\n");
    }

    fclose(fichierSortie);
    fclose(fichierPresence);
}


//--------------------------------- cumul retard par étudiant ------------------------------------
/* void afficherCumulRetardParEtudiant() {
    FILE* fichierPresence = NULL;
    char nomFichier[] = "presences.txt";
    Apprenant etudiants[50];
    int nb_etudiants = 0;

    fichierPresence = fopen(nomFichier, "r+");

    // On lit le nombre d'étudiants dans le fichier
    fscanf(fichierPresence, "%d", &nb_etudiants);

    // Si on arrive pas à ouvrir le fichier ou qu'il est vide on sort de la fonction
    if (fichierPresence == NULL || nb_etudiants ==  0) {    
        printf("Impossible d'ouvrir le fichier %s !\n", nomFichier);
        return;
    }

    // On parcourt tous les élèves du fichier
    for (int i = 1; i <= nb_etudiants; i++) {
        AfficherEtudiant(&etudiants[i]);
        printf("\n");
        float retard;
        fscanf(fichierPresence, "%f", &retard);
        printf("Retard cumulé : %.2f jour(s)\n", retard);
        etudiants[i].retard_cumule = retard;
    }

    // On affiche l'ensemble des informations sur chaque étudiant

} */
void marquerPresence(){
    char choix[10];
    do {
        printf("\n--- Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
        scanf("%s", choix); 
        FILE *fichier = fopen("etudiant.txt", "r");
        if (fichier == NULL)
        {
            printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
            return;
        }

        char matricule[10];
        int found = 0;
        while (fscanf(fichier, "%s", matricule) != EOF)
        {
            if (strcmp(matricule, choix) == 0)
            {
                found = 1;
                break;
            }
        }
        fclose(fichier);

        if (!found)
        {
            printf("--- ❌ Matricule invalide. Veuillez reessayer ('Q' pour quitter) : ");
        }
        else
        {
            enregistrerPresence(choix);
        }
    }while (strcmp(choix, "Q") != 0 && strcmp(choix, "q") != 0);
}

int recupNbApprenant(Apprenant *apprenants) {
    FILE *fichier = fopen("etudiant.txt", "r");
    char ligne[50];

    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
        return -1;
    }
    int i = 0;
    while (fgets(ligne, sizeof(ligne), fichier) != NULL && i < MAX_APPRENANTS)
    {
        sscanf(ligne, "%s %s %s %s %s", apprenants[i].matricule, apprenants[i].motdepasse, apprenants[i].prenom, apprenants[i].nom, apprenants[i].classe);
        i++;
    }

    fclose(fichier);

    return i;
}

int recupNbmessage(Message *messages)
{
    FILE *fichier = fopen("message.bin", "rb");
    char ligne[100];

    if (fichier == NULL)
    {
        fichier = fopen("message.bin", "wb");
    }

    char date[10], heure[10];
    int i = 0;
    while (fread(&messages[i], sizeof(Message), 1, fichier) == 1)
        i++;

    fclose(fichier);

    return i;
}

int recupClasse(char classe[], Apprenant *etudiants)
{
    Apprenant apprenant[10];
    int nbApprenant = recupNbApprenant(apprenant), j = 0;
    for (int i = 0; i < nbApprenant; i++)
    {
        if (strcmp(apprenant[i].classe, classe) == 0)
        {
            etudiants[j++] = apprenant[i];
        }
    }
    return j;
}

// Fonction pour vérifier les identifiants de connexion
int verifierIdentifiants(Identifiants *identifiants, int nombreIdentifiants, char *login, char *motDePasse)
{
    for (int i = 0; i < nombreIdentifiants; i++)
    {
        if (strcmp(identifiants[i].login, login) == 0 && strcmp(identifiants[i].motDePasse, motDePasse) == 0)
        {
            return 1; // Identifiants valides
        }
    }
    return 0; // Identifiants invalides
}

int ajouterMessage(Message msg)
{
    FILE *fichier = fopen("message.bin", "ab");
    int rst = fwrite(&msg, sizeof(Message), 1, fichier);
    fclose(fichier);
    return rst;
}

int classeExiste(char *classeSaisie)
{
    char classes[3][7] = {"DEVWEB", "DATA", "REFDIG"};

    int valide = 0;
    while (!valide)
    {
        printf("Veuillez saisir une classe : ");
        scanf("%s", classeSaisie);

        for (int i = 0; i < MAX_CLASSES; i++)
        {
            if (strcmp(classes[i], classeSaisie) == 0)
            {
                return 1; // La classe existe
            }
        }

        if (!valide)
        {
            printf("La classe saisie n'est pas valide. Veuillez réessayer.\n");
        }
    }

    return 1; // La classe est valide
}

int recupMessageApprenant(char matricule[], Message *mesg)
{
    Message messages[50];
    int size = recupNbmessage(messages);
    int nbM = 0;
    for (int i = 0; i < size; i++)
    {
        if (strcmp(messages[i].matricule, matricule) == 0 && messages[i].status == 1)
        {
            mesg[nbM++] = messages[i];
        }
    }
    return nbM;
}

void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Lire et ignorer les caractères jusqu'à la fin de la ligne ou la fin du fichier
}
// fonction main
int main()
{

    // Création des fichiers pour stocker les identifiants
    FILE *fichierAdmin = fopen("admin.txt", "r");
    FILE *fichierEtudiant = fopen("etudiant.txt", "r");

    if (fichierAdmin == NULL || fichierEtudiant == NULL)
    {
        printf("Erreur lors de l'ouverture des fichiers.\n");
        return 1;
    }

    // Variables pour stocker les identifiants
    Identifiants identifiantsAdmin[100];    // Pour stocker jusqu'à 100 identifiants d'administrateur
    Identifiants identifiantsEtudiant[100]; // Pour stocker jusqu'à 100 identifiants d'étudiant

    int nombreIdentifiantsAdmin = 0;
    int nombreIdentifiantsEtudiant = 0;

    // Lecture des identifiants de l'admin
    while (fscanf(fichierAdmin, "%s %s", identifiantsAdmin[nombreIdentifiantsAdmin].login, identifiantsAdmin[nombreIdentifiantsAdmin].motDePasse) == 2)
    {
        nombreIdentifiantsAdmin++;
    }
    fclose(fichierAdmin);

    // Lecture des identifiants de l'étudiant
    while (fscanf(fichierEtudiant, "%s %s", identifiantsEtudiant[nombreIdentifiantsEtudiant].login, identifiantsEtudiant[nombreIdentifiantsEtudiant].motDePasse) == 2) {
        nombreIdentifiantsEtudiant++;
    }
    fclose(fichierEtudiant);

    int choix = 0;
    int choixMenu;
    char saisieLogin[LONGUEUR_MAX_LOGIN];
    char *saisieMotDePasse = malloc(LONGUEUR_MAX_MDP * sizeof(char)); // Allocation mémoire

    // Authentification
    do
    {
        //system("clear");
        printf("\n------- Connexion -------\n\n");
        saisieLogin[LONGUEUR_MAX_LOGIN] = '\0';
        printf("----- login : ");
        fgets(saisieLogin, LONGUEUR_MAX_LOGIN, stdin);
        saisieLogin[strcspn(saisieLogin, "\n")] = 0; // Supprime le caractère de nouvelle ligne
        if (strlen(saisieLogin) == 0)
        {
            printf("\nVous avez laissé le champ vide. Veuillez rentrer votre login.\n");
            continue;
        }
        
        printf("----- Mot de passe : ");

        int i = 0, c;
        while (i < LONGUEUR_MAX_MDP - 1 && (c = getch()) != '\n') {
            if (c == 127)
            { // ASCII value for backspace
                if (i > 0)
                {
                    printf("\b \b"); // Effacer le caractère précédent
                    i--;
                }
            }
            else
            {
                saisieMotDePasse[i++] = c;
                printf("*");
            }
        }
        saisieMotDePasse[i] = '\0';

        if (strlen(saisieMotDePasse) == 0) {
            printf("\nVous avez laissé le champ vide. Veuillez entrer votre mot de passe.\n");
            continue;
        }

        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) && !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)))
            printf("\nLogin ou mot de passe invalides.\n");

        if ((verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse))) {
            do {
                printf("\n-------------------------------------------------------------------------------\n");
                printf("\t\t\tBienvenue dans le menu de l'administrateur: \n");
                printf("-------------------------------------------------------------------------------\n");
                printf("1 ----- Gestion des étudiants\n");
                printf("2 ----- Génération de fichiers\n");
                printf("3 ----- Marquer les présences\n");
                printf("4 ----- Envoyer un message\n");
                printf("5 ----- Paramètres\n");
                printf("6 ----- Deconnexion\n");
                printf("\n----- Entrez votre choix : ");
                scanf("%d", &choix);
                if (choix == 3)
                {
                    marquerPresence();
                    do
                    {
                        viderBuffer();
                        printf(" Mot de passe : ");

                        int i = 0, c;
                        while (i < LONGUEUR_MAX_MDP - 1 && (c = getch()) != '\n')
                        {
                            if (c == 127)
                            { // ASCII value for backspace
                                if (i > 0)
                                {
                                    printf("\b \b"); // Effacer le caractère précédent
                                    i--;
                                }
                            }
                            else
                            {
                                saisieMotDePasse[i++] = c;
                                printf("*");
                            }
                        }
                        saisieMotDePasse[i] = '\0';
                        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)))
                        {
                            printf("\n--- ⛔ Vous n'êtes pas l'admin \n");
                            printf("Taper entrer pour continuer...\n");
                            getchar();
                            system("clear");
                            marquerPresence();
                            printf("\n");
                        }
                    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)));
                }
                if (choix == 4)
                {
                    printf("\n");
                    int choixmes;
                    char message[100];
                    time_t t = time(NULL);
                    struct tm tm = *localtime(&t);
                    do
                    {
                        char rep;
                        do
                        {
                            memset(message, '\0', sizeof(message));
                            printf("-------------------------------------------------------\n");
                            printf("-------------  Envoyer un message                     \n");
                            printf("*-----------------------------------------------------*\n");
                            printf("1 ----- Envoyer un message à tout le monde             \n");
                            printf("2 ----- Envoyer un message à une classe                \n");
                            printf("3 ----- Envoyer un message à un ou plusieurs étudiants \n");
                            printf("4 ----- Retour\n");
                            printf("\n----- Entrez votre choix");
                            scanf("%d", &choixmes);
                            if (choixmes == 1)
                            {
                                char m;
                                int i = 0;
                                getchar();
                                while (1)
                                {
                                    printf("Taper votre message : ");
                                    m = getchar();
                                    while (m != '\n')
                                    {
                                        message[i++] = m;
                                        m = getchar();
                                    }

                                    if (i != 0)
                                        break;
                                }

                                // Envoi du message à tous les étudiants
                                Message messages[50], msg;
                                Apprenant apprenants[50];
                                char date[20];
                                sprintf(date, "%02d/%02d/%04d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
                                int nbM = recupNbmessage(messages);
                                int nbE = recupNbApprenant(apprenants);

                                strcpy(msg.contenu, message);
                                strcpy(msg.dateHeur, date);
                                msg.status = 1;

                                for (int i = 0; i < nbE; i++)
                                {
                                    strcpy(msg.matricule, apprenants[i].matricule);
                                    msg.id = nbM + 1 + i;
                                    ajouterMessage(msg);
                                }
                            }
                            if (choixmes == 2)
                            {
                                char m;
                                int i = 0;
                                getchar();
                                while (1)
                                {
                                    printf("Taper votre message : ");
                                    m = getchar();
                                    while (m != '\n')
                                    {
                                        message[i++] = m;
                                        m = getchar();
                                    }

                                    if (i != 0)
                                        break;
                                }
                                char classeSaisie[MAX_LENGTH];
                                classeExiste(classeSaisie);
                                Apprenant etudiants[50];
                                char date[20];
                                Message messages[50], msg;

                                int nbE = recupClasse(classeSaisie, etudiants);
                                int nbM = recupNbmessage(messages);

                                sprintf(date, "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

                                strcpy(msg.contenu, message);
                                strcpy(msg.dateHeur, date);
                                msg.status = 1;

                                for (int i = 0; i < nbE; i++)
                                {
                                    strcpy(msg.matricule, etudiants[i].matricule);
                                    msg.id = nbM + 1 + i;
                                    ajouterMessage(msg);
                                }
                            }
                            if (choixmes == 4) {

                            }
                            if (choixmes == 3)
                            {
                                  Message messages[50], msg;
                                  Apprenant apprenants[50];
                                char matricules[100]; // pour stocker les matricules des étudiants
                                printf("Entrez le(s) matricule(s) de(s) étudiant(s) destinataire(s) séparés par des virgules : ");
                                scanf("%s", matricules); // lire les matricules entrés par l'utilisateur
                                getchar();               // Pour absorber le saut de ligne
                                printf("Taper votre message : \n");
                                fgets(message, sizeof(message), stdin); // lire le message de l'utilisateur

                                int nbM = recupNbmessage(messages);
                                int nbE = recupNbApprenant(apprenants);

                                char date[20];
                                sprintf(date, "%02d/%02d/%04d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

                                // Diviser les matricules entrés par l'utilisateur
                                char *token = strtok(matricules, ",");
                                while (token != NULL)
                                {
                                    // Rechercher l'étudiant correspondant au matricule donné
                                    int matricule_valide = 0;
                                    for (int i = 0; i < nbE; i++)
                                    {
                                        if (strcmp(token, apprenants[i].matricule) == 0)
                                        {
                                            // Ajouter le message pour cet étudiant
                                            Message msg;
                                            strcpy(msg.contenu, message);
                                            strcpy(msg.dateHeur, date);
                                            msg.status = 1;
                                            strcpy(msg.matricule, token);
                                            msg.id = nbM + 1; // Identifiant unique pour chaque message
                                            ajouterMessage(msg);
                                            matricule_valide = 1; // Matricule valide
                                            break;
                                        }
                                    }
                                    // Vérifier si le matricule est valide et afficher un message approprié
                                    if (matricule_valide)
                                        printf("Message envoyé avec succès pour l'étudiant de matricule %s.\n", token);
                                    else
                                        printf("Matricule invalide, message non envoyé pour le matricule %s.\n", token);

                                    token = strtok(NULL, ",");
                                }
                            }

                            printf("Taper entrer pour continuer \n");
                            printf("Voulez-vous envoyer un autre message ? (o/n) ");
                            scanf("%c", &rep);
                        } while (rep == 'o' || rep == 'O' && choixmes != 4);
                        getchar();
                        system("clear");
                        if (rep == 'n' || rep == 'N')
                            break;

                    } while (choix <= 0 || choix > 3);
                    system("clear");
                }
                if (choix == 6)
                {
                    printf("Vous êtes déconnecté !\n");
                }
                if (choix == 5) {
                    
                }
                if (choix == 2) {
                    int choixP;
                    do {
                        printf("-------------------------------------------------\n");
                        printf("          Génération de fichiers                  \n");
                        printf("-------------------------------------------------\n");
                        printf("1 --- Toutes les présences par date              \n");
                        printf("2 --- Présences par date                           \n");
                        printf("3 --- Présences dans le mois                       \n");
                        printf("4 --- Liste des retards               \n");
                        printf("5 --- Retour au menu                               \n");
                        printf("----------------------------------------------------\n");
                        printf("Entrez votre choix : ");
                        scanf("%d", &choixP);
                        system("clear");
                        printf("\n1 ----- Les cumules de retard des apprenants\n");
                        calculerRetardCumule();
                        if (choixP == 1)
                        {
                            verifier_presence_et_generer_fichier();
                        }
                        if (choixP == 2)
                        {
                            printf("Date : ");
                            saisir_et_verifier_date();
                        }
                        if (choixP == 3)
                        {
                            system("clear");
                            genererPresenceDansLeMois();
                        }
                        if (choixP == 4) {
                            system("clear");
                            calculerRetardCumule();
                        }
                    } while (choixP != 5);
                }
                if (choix < 1 || choix > 6)
                {
                    printf("Choix invalide. Veuillez entrer un choix entre 1 et 2.\n");
                }
            } while (choix != 6);
        }
        if ((verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)))
        {
            int choix = 0, nb;
            Message messages[50];
            system("clear");
            do
            {
                nb = recupMessageApprenant(saisieLogin, messages);
                printf("\n--------------------------------------------------------------------------\n");
                printf("\t\t\tBienvenue dans le menu de l'apprenant :\n");
                printf("--------------------------------------------------------------------------\n");
                printf("1 ---------- MARQUER MA PRÉSENCE\n");
                printf("2 ---------- Message (%d)\n", nb);
                printf("3 ---------- Cumul de retard\n");
                printf("4 ---------- Déconnexion\n");
                printf("\n---------- Entrez votre choix : ");
                printf("\n Entrez votre choix : ");
                scanf("%d", &choix);
                if (choix < 1 || choix > 4)
                {
                    printf("Choix invalide. Veuillez entrer un choix entre  1 et 5.\n");
                }
                if (choix == 3) {
                    calculerRetardCumule();
                }
                if (choix == 1)
                {
                    system("clear");

                    //----------------------- Doublons & Présence ------------------------------------
                    FILE *fichierPresence = fopen("presence.txt", "r");
                    if (fichierPresence == NULL)
                    {
                        printf("Erreur lors de l'ouverture du fichier de présence.\n");
                        return 1;
                    }

                    int present = 0;
                    char matricule[10];
                    char dateLu[MAX_LENGTH];
                    while (fscanf(fichierPresence, "%s %s", matricule, dateLu) != EOF)
                    {
                        if (strcmp(matricule, matricule) == 0)
                        {
                            // Vérifier si la date correspond à la date actuelle
                            time_t t = time(NULL);
                            struct tm *tm = localtime(&t);
                            char dateActuelle[MAX_LENGTH];
                            strftime(dateActuelle, sizeof(dateActuelle), "%d/%m/%Y", tm);
                            if (strcmp(dateLu, dateActuelle) == 0)
                            {
                                fclose(fichierPresence);
                                present = 1;
                                return 1; // L'étudiant est déjà marqué présent aujourd'hui
                            }
                        }
                    }

                    fclose(fichierPresence);

                    if (!present)
                    {
                        FILE *fichier = fopen("etudiant.txt", "r+");
                        if (fichier == NULL)
                        {
                            printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
                            return 1;
                        }

                        while (fscanf(fichier, "%s", matricule) != EOF)
                        {
                            if (strcmp(matricule, matricule) == 0)
                            {
                                // Enregistrer la présence dans le fichier
                                enregistrerPresence(matricule);
                                
                                present = 1;
                                break;
                            }
                        }

                        fclose(fichier);
                    }
                    //----------------------------------------  Fin --------------------------------------------------
                }
                if (choix == 4)
                {
                    printf("Vous êtes déconnecté !\n");
                    saisieLogin[LONGUEUR_MAX_LOGIN] = 'a';
                }
                if (choix == 2)
                {
                    for (int i = 0; i < nb; i++)
                    {
                        printf("\n----- Message %d : %s\n", i + 1, messages[i].contenu);
                    }

                    Message toutMessage[50], msg;
                    int nbMessage = recupNbmessage(toutMessage);
                    FILE *fichier = fopen("message.bin", "wb+");
                    for (int i = 0; i < nbMessage; i++)
                    {
                        msg = toutMessage[i];
                        for (int j = 0; j < nb; j++)
                        {
                            if (msg.id == messages[j].id)
                            {
                                msg.status = 0;
                                break;
                            }
                        }
                        fwrite(&msg, sizeof(Message), 1, fichier);
                    }

                    fclose(fichier);
                }
            } while (choix != 4);
        }
    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) || !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)));
    return 0;
}