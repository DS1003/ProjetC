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
    fprintf(fichier, "%s %d/%d/%d %dh%dmn%ds\n", matricule, timeinfo->tm_mday, timeinfo->tm_mon + 1, 
        timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

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
                printf("\n--- ✅ Presence marquee pour l'etudiant de matricule %s\n", choix);
                present = 1;
            }
        }

        fclose(fichier);

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
    char *saisieMotDePasse;

    // Authentification
    do {
        printf("---------------- Connexion ----------------\n\n");
        
        printf("----- login : ");
        fgets(saisieLogin, LONGUEUR_MAX_LOGIN, stdin);
        saisieLogin[strcspn(saisieLogin, "\n")] = 0; // Supprime le caractère de nouvelle ligne
        if (strlen(saisieLogin) == 0) {
            printf("\nVous avez laissé le champ vide. Veuillez rentrer votre login.\n");
            continue;
        }
        
        saisieMotDePasse = getpass("----- Mot de passe: ");
        if (strlen(saisieMotDePasse) == 0) {
            printf("\nVous avez laissé le champ vide. Veuillez entrer votre mot de passe.\n");
            continue;
        }

        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) && !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse))) {
            printf("Login ou mot de passe invalides.\n");
        }
        if ((verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse))) {
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
                        if  (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse))) {
                            printf("Mot de passe incorrect.\nVeuillez réessayer: ")  ;
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
            menuEtudiant();
        }
    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) || !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)));

    return 0;
}