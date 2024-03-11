#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>

//---------------------- Cacher mot de passe par '*' ---------------------
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

typedef struct {
    char login[LONGUEUR_MAX_LOGIN];
    char motDePasse[LONGUEUR_MAX_MDP];
} Identifiants;

typedef struct {
    char matricule[10];
    char motdepasse[10];
    char prenom[20];
    char nom[20];
    char classe[6];
    Date dateNaiss;
    int etat;
} Apprenant;

Identifiants identifiantsAdmin;
int nombreIdentifiantsAdmin = 1;

void enregistrerPresence(char *matricule) {
    FILE *fichier = fopen("presence.txt", "a");
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }
    
    // Récupérer la date et l'heure  actuelle
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    // Écrire dans le fichier la date et l'heure
    fprintf(fichier, "%s %d/%d/%d %dh%dmn%ds\n", matricule, timeinfo->tm_mday, timeinfo->tm_mon + 1, 
        timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    fclose(fichier);
}

void marquerPresence() {
    char choix[10];
    printf("\n--- Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
    scanf("%s", choix);
    while (strcmp(choix, "Q") != 0 && strcmp(choix, "q") != 0) {
        FILE *fichierPresence = fopen("presence.txt", "r");
        if (fichierPresence == NULL) {
            printf("Erreur lors de l'ouverture du fichier de présence.\n");
            return;
        }
        int present = 0;
        char matricule[10];
        while (fscanf(fichierPresence, "%s", matricule) != EOF) {
            if (strcmp(matricule, choix) == 0) {
                printf("\n--- ❌ L'étudiant de matricule %s est déjà marqué présent.\n", choix);
                present = 1;
                break;
            }
        }
        fclose(fichierPresence);

        if (!present) {
            FILE *fichier = fopen("etudiant.txt", "r+");
            if (fichier == NULL) {
                printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
                return;
            }

            while (fscanf(fichier,"%s", matricule) != EOF) {
                if (strcmp(matricule, choix) == 0) {
                    // Enregistrer la présence dans le fichier
                    enregistrerPresence(choix);
                    printf("\n--- ✅ Presence marquee pour l'etudiant de matricule %s\n", choix);
                    present = 1;
                    break;
                }
            }
            fclose(fichier);
        }

        if (!present) {
            printf("--- ❌ Matricule invalide. Veuillez reessayer ('Q' pour quitter) : ");
        } else {
            printf("\n--- Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
        }

        scanf("%s", choix);
    }
}

int menuAdmin() {
    int choix = 0;
    do {
        printf("--------------------------------------------------------------------------\n");
        printf("\t\t\tBienvenue dans le menu de l'administrateur:\n");
        printf("--------------------------------------------------------------------------\n");
        printf("1 ---------- Gestion des étudiants\n");
        printf("2 ---------- Génération de fichiers\n");
        printf("3 ---------- Marquer les présences\n");
        printf("4 ---------- Envoyer un message\n");
        printf("5 ---------- Paramètres\n");
        printf("6 ---------- Deconnexion\n");
        printf("\n--- Entrez votre choix : ");
        scanf("%d", &choix);
        if (choix < 1 || choix > 6) {
            printf("Choix invalide. Veuillez entrer un choix entre 1 et 2.\n");
        }
    } while (choix != 6);
    return choix;
}

int menuEtudiant() {
    // Définition du menu de l'étudiant
    int choix = 0;
    do {
        printf("--------------------------------------------------------------------------\n");
        printf("\t\t\tBienvenue dans le menu de l'apprenant :\n");
        printf("--------------------------------------------------------------------------\n");
        printf("1 ---------- GESTION DES ÉTUDIANTS\n");
        printf("2 ---------- GÉNÉRATION DE FICHIERS\n");
        printf("3 ---------- MARQUER SA PRÉSENCE\n");
        printf("4 ---------- Message (0)\n");
        printf("5 ---------- Déconnexion\n");
        printf("\n---------- Entrez votre choix : ");
        scanf("%d", &choix);
        if (choix < 1 || choix > 5) {
            printf("Choix invalide. Veuillez entrer un choix entre  1 et 5.\n");
        }
    } while (choix < 1 || choix > 5);
    return choix;
}

// Fonction pour vérifier les identifiants de connexion
int verifierIdentifiants(Identifiants *identifiants, int nombreIdentifiants, char *login, char *motDePasse) {
    for (int i = 0; i < nombreIdentifiants; i++) {
        if (strcmp(identifiants[i].login, login) == 0 && strcmp(identifiants[i].motDePasse, motDePasse) == 0) {
            return 1; // Identifiants valides
        }
    }
    return 0; // Identifiants invalides
}

void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Lire et ignorer les caractères jusqu'à la fin de la ligne ou la fin du fichier
}


//-------------------------------------------------------- Main -------------------------------------------------------
int main() {
    // Création des fichiers pour stocker les identifiants
    FILE *fichierAdmin = fopen("admin.txt", "r");
    FILE *fichierEtudiant = fopen("etudiant.txt", "r");

    if (fichierAdmin == NULL || fichierEtudiant == NULL) {
        printf("Erreur lors de l'ouverture des fichiers.\n");
        return 1;
    }

    // Variables pour stocker les identifiants
    Identifiants identifiantsAdmin[100]; // Pour stocker jusqu'à 100 identifiants d'administrateur
    Identifiants identifiantsEtudiant[100]; // Pour stocker jusqu'à 100 identifiants d'étudiant

    int nombreIdentifiantsAdmin = 0;
    int nombreIdentifiantsEtudiant = 0;

    // Lecture des identifiants de l'admin
    while (fscanf(fichierAdmin, "%s %s", identifiantsAdmin[nombreIdentifiantsAdmin].login, identifiantsAdmin[nombreIdentifiantsAdmin].motDePasse) == 2) {
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
    do {
        system("clear");
        printf("---------------- Connexion ----------------\n\n");
        saisieLogin[LONGUEUR_MAX_LOGIN] = '\0';
        printf("----- login : ");
        fgets(saisieLogin, LONGUEUR_MAX_LOGIN, stdin);
        saisieLogin[strcspn(saisieLogin, "\n")] = 0; // Supprime le caractère de nouvelle ligne
        if (strlen(saisieLogin) == 0) {
            printf("\nVous avez laissé le champ vide. Veuillez rentrer votre login.\n");
            continue;
        }

        printf("----- Mot de passe : ");

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

        if (strlen(saisieMotDePasse) == 0) {
            printf("\nVous avez laissé le champ vide. Veuillez entrer votre mot de passe.\n");
            continue;
        }

        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) && !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse))) {
            printf("\nLogin ou mot de passe invalides.\n");
        }
        if ((verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse))) {
            do {
                system("clear");
                printf("\n--------------------------------------------------------------------------\n");
                printf("\t\t\tBienvenue dans le menu de l'administrateur:\n");
                printf("--------------------------------------------------------------------------\n");
                printf("1 ---------- Gestion des étudiants\n");
                printf("2 ---------- Génération de fichiers\n");
                printf("3 ---------- Marquer les présences\n");
                printf("4 ---------- Envoyer un message\n");
                printf("5 ---------- Paramètres\n");
                printf("6 ---------- Deconnexion\n");
                printf("\n--- Entrez votre choix : ");
                scanf("%d", &choix);
                if (choix == 3) {
                    system("clear");
                    marquerPresence();
                    do {
                        viderBuffer();
                        printf("----- Mot de passe : ");
                        int i = 0, c;
                        while (i < LONGUEUR_MAX_MDP - 1 && (c = getch()) != '\n') {
                            if (c == 127) { 
                                if (i > 0) {
                                    printf("\b \b"); // Effacer le caractère précédent
                                    i--;
                                }
                            } else {
                                saisieMotDePasse[i++] = c;
                                printf("*");
                            }
                        }
                        saisieMotDePasse[i] = '\0';

                        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse))) {
                            printf("\n--- ⛔ Vous n'êtes pas l'admin !!!\n");
                            getchar();
                            system("clear");
                            marquerPresence();
                        } else { // Appel de la fonction après la saisie du mot de passe correct
                        }
                    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)));
                }

                if (choix == 6) {
                    printf("Vous êtes déconnecté !\n");
                }
                if (choix < 1 || choix > 6) {
                    printf("Choix invalide. Veuillez entrer un choix entre 1 et 2.\n");
                }
            } while (choix != 6);
        }
        if ((verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse))) {
            int choix = 0;
            do {
                printf("\n--------------------------------------------------------------------------\n");
                printf("\t\t\tBienvenue dans le menu de l'apprenant :\n");
                printf("--------------------------------------------------------------------------\n");
                printf("1 ---------- GESTION DES ÉTUDIANTS\n");
                printf("2 ---------- GÉNÉRATION DE FICHIERS\n");
                printf("3 ---------- MARQUER SA PRÉSENCE\n");
                printf("4 ---------- Message (0)\n");
                printf("5 ---------- Déconnexion\n");
                printf("\n---------- Entrez votre choix : ");
                scanf("%d", &choix);
                if (choix < 1 || choix > 5) {
                    printf("Choix invalide. Veuillez entrer un choix entre  1 et 5.\n");
                    system("clear");
                }
                if (choix == 3) {

                    //----------------------- Doublons & Présence ---------------------------------------------------------
                    FILE *fichierPresence = fopen("presence.txt", "r");
                    if (fichierPresence == NULL) {
                        printf("Erreur lors de l'ouverture du fichier de présence.\n");
                        return 1;
                    }

                    int present = 0;
                    char matricule[10];
                    while (fscanf(fichierPresence, "%s", matricule) != EOF) {
                        if (strcmp(matricule, matricule) == 0) {
                            printf("\n--- ❌ L'étudiant de matricule %s est déjà marqué présent.\n", matricule);
                            present = 1;
                            break;
                        } 
                    }

                    fclose(fichierPresence);

                    if (!present) {
                        FILE *fichier = fopen("etudiant.txt", "r+");
                        if (fichier == NULL) {
                            printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
                            return 1;
                        }

                        while (fscanf(fichier,"%s", matricule) != EOF) {
                            if (strcmp(matricule, matricule) == 0) {
                                // Enregistrer la présence dans le fichier
                                enregistrerPresence(matricule);
                                printf("\n--- ✅ Presence marquee pour l'etudiant de matricule %s\n", matricule);
                                present = 1;
                                break;
                            }
                        }

                        fclose(fichier);
                    }
                    //----------------------------------------  Fin -----------------------------------------------------


                }
                if (choix == 5) {
                    system("clear");
                    printf("Vous êtes déconnecté !\n");
                    saisieLogin[LONGUEUR_MAX_LOGIN] = 'a';
                }
            } while (choix != 5);
        }
    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) || !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)));

    return 0;
}


