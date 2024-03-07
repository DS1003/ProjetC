#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

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
    fprintf(fichier, "%s %d/%d/%d %dh%dmn%ds\n", matricule, timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    fclose(fichier);
}

void marquerPresence() {
    char choix[10];
    printf("Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
    scanf("%s", choix);

    while (strcmp(choix, "Q") != 0 && strcmp(choix, "q") != 0) {
        FILE *fichier = fopen("etudiant.txt", "r+");
        if (fichier == NULL) {
            printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
            return;
        }

        int present = 0;
        char matricule[10];

        while (fscanf(fichier,"%s", matricule) != EOF) {
            if (strcmp(matricule, choix) == 0) {
                // Enregistrer la présence dans le fichier
                enregistrerPresence(choix);
                printf("Presence marquee pour l'etudiant de matricule %s\n", choix);
                present = 1;
            }
        }

        fclose(fichier);

        if (!present) {
            printf("Matricule invalide. Veuillez reessayer ('Q' pour quitter) : ");
        } else {
            printf("Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
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

int main() {
    // Initialisation des identifiants de l'administrateur
    strcpy(identifiantsAdmin.login, "admin");
    strcpy(identifiantsAdmin.motDePasse, "admin");

    int choixMenu;
    char saisieLogin[LONGUEUR_MAX_LOGIN];
    char *saisieMotDePasse;

    // Authentification
    do {
        printf("---------------- Connexion ----------------\n\n");
        
        printf("Login : ");
        scanf("%s", saisieLogin);

        saisieMotDePasse = getpass("Mot de passe: ");

        if (strcmp(identifiantsAdmin.login, saisieLogin) != 0 || strcmp(identifiantsAdmin.motDePasse, saisieMotDePasse) != 0) {
            printf("Login ou mot de passe incorrects. Veuillez réessayer.\n");
        } else {
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
                 if (choix == 3) {
                    marquerPresence();
                    do {
                        saisieMotDePasse = getpass("\n--- Mot de passe: ");
                        if  (strcmp(identifiantsAdmin.motDePasse, saisieMotDePasse) != 0) {
                            printf("Mot de passe incorrect.\nVeuillez réessayer: ")  ;
                        }
                    } while (strcmp(identifiantsAdmin.motDePasse, saisieMotDePasse) != 0);
                }
                if (choix == 6) {
                    printf("Vous êtes déconnecté !\n");
                }
                if (choix < 1 || choix > 6) {
                    printf("Choix invalide. Veuillez entrer un choix entre 1 et 2.\n");
                }
            } while (choix != 6);
        }
    } while (strcmp(identifiantsAdmin.login, saisieLogin) != 0 || strcmp(identifiantsAdmin.motDePasse, saisieMotDePasse) != 0);

    return 0;
}
