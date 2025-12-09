#include "temperature_controller.h"
#include "config_controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand((unsigned int)time(NULL)); // Initialiser  générateur  temperature aléatoire

    Configuration config;
    int continu = 1;
    int choix;
    char mode = '1';
    int horlogue = 5; // 5 minutes par défaut pour les tests

    // Vérifier la configuration
    if (lire_configuration(&config) != 0) {
        printf("Echec d'inistialisation les variable de configuration,fichier config.txt invalide\n");

    }

    while (continu) {

        printf("\n\033[36m|----------------------------------------------------------|\033[0m\n");
        printf("      \033[36m SYSTEME DE CONTROLE DE TEMPERATURE INTELLIGENT  \033[0m\n");
        printf("\033[36m|----------------------------------------------------------|\033[0m\n\n\n");
        printf("\033[33m ** Configuration actuelle:\033[0m\n\n");
        printf("- Seuils: %d-%d C\n", config.seuil_min, config.seuil_max);
        printf("- Intervalle: %d secondes\n", config.intervalle_mesure);
        printf("- Mode capteur: %s\n\n\n", mode == '1' ? "Aleatoire" :
                                       mode == '2' ? "Statique" : "Non defini");
        printf("--------------------------------------------------------------------\n");

        printf("\n\033[33m** Voulez Vous :\033[0m\n\n");
        printf("1. Gerer configuration\n");
        printf("2. Choisir mode capteur\n");
        printf("3. Lancer le systeme de controle\n");
        printf("4. Generer et afficher les rapports\n");
        printf("0. Quitter\n");
        printf("\n Votre choix: ");

        scanf("%d", &choix);
        system("cls");
        switch (choix) {
            case 0:
                continu = 0;
                printf("\n Au revoir!\n");
                break;

            case 1:
                // Modifier configuration
                configuration_module(&config);
                break;

            case 2:
                // Choisir mode capteur
                mode = choisir_mode_test();
                if (mode != '\0') {
                    printf("Mode selectionne: %s\n",
                           mode == '1' ? "Aleatoire" : "Statique");
                }
                break;

            case 3:
                // Lancement  système
                if (mode == '\0') {
                    printf("Veuillez d'abord choisir un mode capteur (option 2).\n");
                } else {
                    printf("\n=== LANCEMENT DU SYSTEME ===\n");
                    printf("Entrez la duree de fonctionnement (en minutes): ");
                    int duree;
                    scanf("%d", &duree);
                    if (duree > 0) {
                        horlogue = duree;
                    }

                    demarrer_systeme(config.seuil_min, config.seuil_max,
                                    config.intervalle_mesure, mode, horlogue);
                }
                break;

            case 4:
                // Générer et afficher les rapports
                {


                    // Lire le journal
                    Mesure *mesures = lire_journal();
                    if (!mesures) {
                        printf("Aucune donnee dans le journal.\n");
                    } else {
                        // Calculer les statistiques
                        Stats *stats = calculer_stats(mesures, config.intervalle_mesure);


                        // Générer le rapport fichier
                        generer_rapport(stats, config.intervalle_mesure);



                        // Menu des rapports
                        char choix_rapport;
                        do {
                                printf("\n\n\033[33m ** Menu \033[0m\n");
                                printf("1. Afficher le rapport journalier complet\n");
                                printf("2. Courbe d'analyse de temperature  \n");
                                printf("3. Afficher resume et recommandations\n");
                                printf("4. Retour au menu principal\n");
                                printf("\n Votre choix (1-4): ");

                                scanf(" %c", &choix_rapport);
                                while (getchar() != '\n'); // Vider le buffer
                                system("cls");

                                switch (choix_rapport) {
                                    case '1':
                                        printf("\n");
                                        afficher_stats(stats, config.intervalle_mesure);
                                        printf("\nAppuyez sur Entree pour continuer...");
                                        getchar();
                                        system("cls");
                                        break;
                                    case '2':
                                        {
                                            afficher_courbe_ascii_complete(stats);
                                            printf("\nAppuyez sur Entree pour continuer...");
                                            getchar();
                                            system("cls");
                                        }
                                        break;

                                    case '3':
                                        {
                                            ResumeGlobal resume = calculer_resume_global(stats);
                                            afficher_recommandations(resume);
                                            printf("\nAppuyez sur Entree pour continuer...");
                                            getchar();
                                            system("cls");
                                        }
                                        break;

                                    case '4':
                                        printf("\nRetour au menu principal...\n");

                                        break;

                                    default:
                                        printf("\nChoix invalide!\n");
                                        printf("Appuyez sur Entree pour continuer...");
                                        getchar();

                                        break;
                                }

                            } while (choix_rapport != '4');

                        // Nettoyer la mémoire
                        liberer_mesures(mesures);
                        liberer_stats(stats);
                    }
                }
                break;

            default:
                printf("\nChoix invalide! Veuillez choisir 0-4.\n");
                break;
        }

        if (choix != 0) {

            printf("\nAppuyez sur Entree pour continuer...");
            getchar();
            system("cls");
        }
    }

    return 0;
}
