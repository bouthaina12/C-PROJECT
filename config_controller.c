#include "config_controller.h"
#include <ctype.h>
#include <time.h>



void configuration_module(Configuration *configuration)
{


    printf("1. seuil_min=%d\n", configuration->seuil_min);
    printf("2. seuil_max=%d\n", configuration->seuil_max);
    printf("3. intervalle_mesure=%d\n", configuration->intervalle_mesure);

    char entree;
    do{
    printf("Voulez-vous modifier la configuration ? (o/n)");
    printf("> ");


    scanf(" %c", &entree);
    if (tolower(entree) == 'n')
    {
         printf("aucun mise à jour.");
    }

    else if (tolower(entree) == 'o')
    {
        printf("Entrez le nouveau seuil_min : ");
        scanf("%d", &configuration->seuil_min);

        printf("Entrez le nouveau seuil_max : ");
        scanf("%d", &configuration->seuil_max);

        printf("Entrez la nouvelle intervalle_mesure en seconde : ");
        scanf("%d", &configuration->intervalle_mesure);

        if (ecrire_configuration(configuration) == 0)
        {
            printf("Configuration mise à jour.");

        }
        else
        {
            printf("Échec de l'écriture de la configuration.");
        }
    }
    else
    {
       printf("\nChoix invalide! Veuillez choisir (o/n).\n");
    }
    }while (tolower(entree)!= 'o'&& tolower(entree)!= 'n');


}

char choisir_mode_test()
{
    char entree;
    do
    {
        printf("Choisissez un mode de test :\n");
        printf(" 1. Test aleatoire\n");
        printf(" 2. Test statique\n");
        getchar();
        scanf("%c",&entree);
        if (entree == '1' || entree == '2')
        {

           return entree ;
        }

        printf("Choix invalide.");
    }while(entree != '1' && entree != '2');
}


int lire_configuration(Configuration *configuration)
{
    FILE *fichier = fopen(CONFIG_PATH, "r");
    if (!fichier)
    {
        return -1;
    }

    char ligne[INPUT_BUFFER];
    int champs_lus = 0;

    while (fgets(ligne, sizeof(ligne), fichier))
    {
        if (sscanf(ligne, "seuil_min=%d", &configuration->seuil_min) == 1)
        {
            champs_lus++;
        }
        else if (sscanf(ligne, "seuil_max=%d", &configuration->seuil_max) == 1)
        {
            champs_lus++;
        }
        else if (sscanf(ligne, "intervalle_mesure=%d", &configuration->intervalle_mesure) == 1)
        {
            champs_lus++;
        }
    }

    fclose(fichier);
    return (champs_lus == 3) ? 0 : -1;
}

int ecrire_configuration(const Configuration *configuration)
{
    FILE *fichier = fopen(CONFIG_PATH, "w");
    if (!fichier)
    {
        return -1;
    }

    fprintf(fichier, "seuil_min=%d\n", configuration->seuil_min);
    fprintf(fichier, "seuil_max=%d\n", configuration->seuil_max);
    fprintf(fichier, "intervalle_mesure=%d\n", configuration->intervalle_mesure);

    fclose(fichier);
    return 0;
}




