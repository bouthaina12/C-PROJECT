#include "temperature_controller.h"
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif


// Fonction pour compter les mesures (debug)
int compter_mesures(Mesure *m) {
    int count = 0;
    while (m) {
        count++;
        m = m->suivant;
    }
    return count;
}

// Ajouter une mesure à la liste
Mesure* ajouter_mesure(Mesure *tete, char *date, char *heure, float temp, int alerte) {
    Mesure *nouveau = (Mesure*)malloc(sizeof(Mesure));
    strcpy(nouveau->date, date);
    strcpy(nouveau->heure, heure);
    nouveau->temperature = temp;
    nouveau->alerte = alerte;
    nouveau->suivant = tete;
    return nouveau;
}

// Lire le fichier journal //
Mesure* lire_journal() {
    FILE *fichier = fopen(FICHIER_JOURNAL, "r");
    if (!fichier) {
        printf(" ERREUR: Impossible d'ouvrir journal.txt\n");
        return NULL;
    }

    Mesure *tete = NULL;
    char ligne[256];
    int compteur_mesures = 0;

    while (fgets(ligne, sizeof(ligne), fichier)) {


        char date[11] = "", heure[9] = "";
        float temp = 0;
        int alerte = -1;

        int resultat = sscanf(ligne, "%10s %8s - Temperature: %f C - Niveau alerte: %d",
                             date, heure, &temp, &alerte);

        if (resultat == 4) {
            tete = ajouter_mesure(tete, date, heure, temp, alerte);
            compteur_mesures++;
        }
    }

    fclose(fichier);

    if (compteur_mesures == 0) {
        printf(" CRITIQUE: Aucune mesure valide trouvee!\n");
    } else {
        printf(" SUCCES: %d mesures chargees\n", compteur_mesures);
    }

    return tete;
}

// Trouver ou créer les stats pour une date (ajoute à la FIN)
Stats* obtenir_stats(Stats **tete, char *date) {
    Stats *courant = *tete;

    // Chercher si la date existe déjà
    while (courant) {
        if (strcmp(courant->date, date) == 0) {
            return courant;
        }
        courant = courant->suivant;
    }

    // Créer nouvelle stats
    Stats *nouveau = (Stats*)malloc(sizeof(Stats));
    strcpy(nouveau->date, date);
    nouveau->min = 1000;
    nouveau->max = -1000;
    nouveau->somme = 0;
    nouveau->nb_mesures = 0;
    nouveau->alertes1 = nouveau->alertes2 = nouveau->alertes3 = 0;
    nouveau->duree_alerte_sec = 0;
    nouveau->suivant = NULL;

    // Ajouter à la fin de la liste
    if (*tete == NULL) {
        *tete = nouveau;  // Première élément
    } else {
        Stats *dernier = *tete;
        while (dernier->suivant != NULL) {
            dernier = dernier->suivant;
        }
        dernier->suivant = nouveau;  // Ajoute a la fin
    }

    return nouveau;
}
// Calculer les statistiques
Stats* calculer_stats(Mesure *mesures,int intervalle_mesure) {
    if (!mesures) return NULL;

    Stats *stats_tete = NULL;
    Mesure *courant = mesures;


    while (courant) {
        Stats *s = obtenir_stats(&stats_tete, courant->date);

        // Mettre à jour min/max/moyenne
        if (courant->temperature < s->min) s->min = courant->temperature;
        if (courant->temperature > s->max) s->max = courant->temperature;
        s->somme += courant->temperature;
        s->nb_mesures++;

        // Compter les alertes
        if (courant->alerte == 1) s->alertes1++;
        else if (courant->alerte == 2) s->alertes2++;
        else if (courant->alerte == 3) s->alertes3++;

        // Calculer durée d'alerte
        if (courant->alerte > 0) {
            s->duree_alerte_sec += intervalle_mesure;
        }

        courant = courant->suivant;
    }

    return stats_tete;
}

// Afficher les statistiques à l'écran
void afficher_stats(Stats *stats,int intervalle_mesure) {
    printf("\n");
    printf("\033[36m==========================================================\033[0m\n");
    printf("\033[36m|                  RAPPORTS JOURNALIERS                   |\033[0m \n");
    printf("\033[36m==========================================================\033[0m\n");
    printf("Intervalle de mesure: %d secondes\n\n", intervalle_mesure);

    printf("+--------------+----------+----------+----------+---------+-----------------------+\n");
    printf("|     Date     |  Mesures |  Moyenne |    Min   |   Max   |  Duree Alerte         |\n");
    printf("+--------------+----------+----------+----------+---------+-----------------------+\n");

    int total_jours = 0;
    int total_alertes=0;
    float moyenne;
    int min_alerte;
    int sec_alerte ;
    char indicateur[4] = "   ";
    int espaces;
    Stats *courant = stats;

    while (courant) {
         moyenne = courant->nb_mesures > 0 ? courant->somme / courant->nb_mesures : 0;
         min_alerte = courant->duree_alerte_sec / 60;
         sec_alerte = courant->duree_alerte_sec % 60;
         total_alertes += courant->alertes1 + courant->alertes2 + courant->alertes3;

        // Indicateur visuel d'alerte

        if (courant->alertes3 > 0) {
            strcpy(indicateur, "[!]");
        } else if (courant->alertes2 > 0) {
            strcpy(indicateur, "[~]");
        } else if (courant->alertes1 > 0) {
            strcpy(indicateur, "[.]");
        }

        printf("| %-12s | %-8d | %7.1fC | %7.1fC | %6.1fC | %3d:%02d min   |\n",
               courant->date,
               courant->nb_mesures,
               moyenne,
               courant->min,
               courant->max,
               min_alerte,
               sec_alerte);

        // Ligne supplémentaire pour les alertes
        if (courant->alertes1 + courant->alertes2 + courant->alertes3 > 0) {
            printf("| %s Alertes: N1=%d, N2=%d, N3=%d",
                   indicateur,
                   courant->alertes1,
                   courant->alertes2,
                   courant->alertes3);

            // Espacement pour aligner
              espaces = 60 - (strlen(indicateur) + 20);
            for (int i = 0; i < espaces; i++) printf(" ");
            printf("     |\n");
        }

        printf("--------------+----------+----------+----------+---------+--------------+\n");

        total_jours++;
        courant = courant->suivant;
    }

    // Résumé
    //printf("\nRESUME:\n");
    printf("-------\n");
    printf("\033[33m Jours analyses : %d \033[0m\n", total_jours);

}

// Générer le rapport fichier
void generer_rapport(Stats *stats,int intervalle_mesure) {
    FILE *f = fopen(FICHIER_RAPPORT, "w");
    if (!f) {
        printf(" Erreur creation rapport\n");
        return;
    }

    fprintf(f, "RAPPORT Journaliere COMPLET - CONTROLEUR TEMPERATURE\n");
    fprintf(f, "--------------------------------------------------------\n\n");
    fprintf(f, "Intervalle de mesure: %d secondes\n\n",intervalle_mesure);

    time_t maintenant = time(NULL);
    char date_rapport[50];
    strftime(date_rapport, sizeof(date_rapport), "%d/%m/%Y %H:%M",
             localtime(&maintenant));
    fprintf(f, "Genere le: %s\n\n", date_rapport);

    int total_jours = 0;
    Stats *courant = stats;
    float moyenne;
    int min_alerte;
    int sec_alerte;
    while (courant) {
         moyenne = courant->nb_mesures > 0 ? courant->somme / courant->nb_mesures : 0;
         min_alerte = courant->duree_alerte_sec / 60;
         sec_alerte = courant->duree_alerte_sec % 60;

        fprintf(f, "JOUR: %s\n", courant->date);
        fprintf(f, "----------------------------------------\n");
        fprintf(f, "nb Mesures: %d\n", courant->nb_mesures);
        fprintf(f, "Température: Moy=%.1f°C, Min=%.1f°C, Max=%.1f°C\n",
                moyenne, courant->min, courant->max);
        fprintf(f, "Alertes: N1=%d, N2=%d, N3=%d\n",
                courant->alertes1, courant->alertes2, courant->alertes3);
        fprintf(f, "Durée alerte: %d min %d sec\n\n", min_alerte, sec_alerte);

        total_jours++;
        courant = courant->suivant;
    }

    fprintf(f, "=========================================\n");
    fprintf(f, "Total jours analyses: %d\n", total_jours);
    fprintf(f, "Fin du rapport\n");

    fclose(f);
    printf(" \n + Rapport sauvegarde: %s\n", FICHIER_RAPPORT);
}

// Libérer la mémoire
void liberer_mesures(Mesure *m) {
    int compteur = 0;
    while (m) {
        Mesure *temp = m;
        m = m->suivant;
        free(temp);
        compteur++;
    }

}

void liberer_stats(Stats *s) {
    int compteur = 0;
    while (s) {
        Stats *temp = s;
        s = s->suivant;
        free(temp);
        compteur++;
    }
}


// FONCTION : CALCULER RÉSUMÉ GLOBAL

ResumeGlobal calculer_resume_global(Stats *stats) {
    ResumeGlobal resume = {0};

    if (stats == NULL) {
        return resume;
    }

    Stats *courant = stats;
    resume.temp_min_globale = 2000;
    resume.temp_max_globale = -2000;

    while (courant) {
        resume.total_jours++;
        resume.total_mesures += courant->nb_mesures;
        resume.total_alertes1 += courant->alertes1;
        resume.total_alertes2 += courant->alertes2;
        resume.total_alertes3 += courant->alertes3;
        resume.total_duree_alerte_sec += courant->duree_alerte_sec;
        resume.somme_temperatures += courant->somme;

        if (courant->min < resume.temp_min_globale)
            resume.temp_min_globale = courant->min;
        if (courant->max > resume.temp_max_globale)
            resume.temp_max_globale = courant->max;

        courant = courant->suivant;
    }

    resume.total_alertes = resume.total_alertes1 + resume.total_alertes2 + resume.total_alertes3;
    resume.moyenne_globale = resume.total_mesures > 0 ?
                           resume.somme_temperatures / resume.total_mesures : 0;

    return resume;
}


void afficher_recommandations(ResumeGlobal resume) {
    printf("\n");
    printf("\033[32m----------------------------------------------------------\033[0m\n");
    printf(" \033[32m                   Resume ET RECOMMANDATIONS\033[0m\n");
    printf("\033[32m-----------------------------------------------------------\033[0m\n\n");

    // Vérifier si des données existent
    if (resume.total_jours == 0) {
        printf("Aucune donnee disponible pour l'analyse.\n");
        return;
    }

    int total_min = resume.total_duree_alerte_sec / 60;
    int total_sec = resume.total_duree_alerte_sec % 60;

    // Résumé statistique: aide pour le retro_controle de config
    printf("\033[36m RESUME STATISTIQUE :\033[0m\n");
    printf("--------------------\033[0m\n\n");
    printf("Periodes analysees  : %d jours\n", resume.total_jours);
    //printf("Mesures totales     : %d\n", resume.total_mesures);
    printf("Temperature moyenne : %.1f C\n", resume.moyenne_globale);
    printf("Plage temperature   : %.1f C à %.1f C\n\n",
           resume.temp_min_globale, resume.temp_max_globale);
    printf("\n");

    // Analyse des alertes
    printf("\033[36m RESUME DES ALERTES :\033[0m\n");
    printf("---------------------\n\n");
    printf("Alertes niveau 1    : %d\n", resume.total_alertes1);
    printf("Alertes niveau 2    : %d\n", resume.total_alertes2);
    printf("Alertes niveau 3    : %d\n", resume.total_alertes3);
    printf("Duree totale alerte : %d min %d sec\n", total_min, total_sec);

    float moyenne_alertes_par_jour = (float)resume.total_alertes / resume.total_jours;
    printf("Moyenne alertes par jour: %.1f\n\n", moyenne_alertes_par_jour);

    printf("\n");

    // RECOMMANDATIONS :

    printf("\033[36m RECOMMANDATIONS SYSTEME :\033[0m\n");
    printf("-------------------------\n");

    // Basées sur les alertes niveau 3
    if (resume.total_alertes3 > 3*resume.total_jours) {
        printf("   [!] critique: Plusieurs alertes niveau 3 sont detectees , Verifier le systeme de refroidissement/chauffage\n");
    }else if (resume.total_alertes > 5*resume.total_jours){
        printf("   [~] Plus d'attention necessaire ,verifier les regulations de configuration\n");
    }else{
        printf("   [.] Systeme stable\n");

    }

    // Basées sur la durée d'alerte
    if (resume.total_duree_alerte_sec > 300*resume.total_jours) { // Plus de 5 minutes
        printf("   Analyser les causes des alertes prolongees\n");
        printf("   Considerer ajuster les seuils d'alerte\n");
    }

    // Basées sur la plage de température
    float amplitude = resume.temp_max_globale - resume.temp_min_globale;
    if (amplitude > 15.0 && resume.total_jours > 1) {
        printf("   Forte variation de temperature (%.1f C)\n", amplitude);
        printf("   Optimiser les cycles de regulation de configuration\n");
    }

    printf("\n");

    printf("\033[31m Note: Ces recommandations sont basees sur l'analyse de %d mesures\033[0m\n",
           resume.total_mesures);
    printf("    \033[31m  sur une periode de %d jours.\033[0m\n", resume.total_jours);

}

void afficher_courbe_ascii_complete(Stats *stats) {
    if (!stats) return;

    printf("\n\033[35m COURBE DES TEMPERATURES \033[0m\n\n");

    // Stocker les données dans un tableau
    Stats *courant = stats;
    int nb_jours = 0;
    while (courant) { nb_jours++; courant = courant->suivant; }

    float moyennes[nb_jours];
    char dates[nb_jours][11];


    courant = stats;
    int idx = 0;
    while (courant) {
        moyennes[idx] = courant->nb_mesures > 0 ? courant->somme / courant->nb_mesures : 0;
        strcpy(dates[idx], courant->date);
        idx++;
        courant = courant->suivant;
    }

    // Trouver min et max
    float temp_min = moyennes[0], temp_max = moyennes[0];
    for (int i = 1; i < nb_jours; i++) {
        if (moyennes[i] < temp_min) temp_min = moyennes[i];
        if (moyennes[i] > temp_max) temp_max = moyennes[i];
    }

    // Ajouter un peu de marge
    float range = temp_max - temp_min;
    if (range < 5.0f) {
        temp_min -= 2.0f;
        temp_max += 2.0f;
    }

    // Hauteur du graphique
    int hauteur = 12;

    // Afficher chaque ligne
    for (int ligne = hauteur; ligne >= 0; ligne--) {
        float valeur = temp_min + (temp_max - temp_min) * ligne / hauteur;
        printf("%6.1fC |", valeur);

        // Pour chaque point de données
        for (int j = 0; j < nb_jours; j++) {
            // Calculer où placer ce point
            float ratio = (moyennes[j] - temp_min) / (temp_max - temp_min);
            int pos = (int)(ratio * hauteur + 0.5);

            if (pos == ligne) {
                printf(" * ");
            } else {
                printf("   ");
            }
        }
        printf("\n");
    }

    // Ligne de séparation
    printf("         ");
    for (int j = 0; j < nb_jours; j++) printf("---");
    printf("-\n");

    // Numéros des jours
    printf("          ");
    for (int j = 0; j < nb_jours; j++) {
        printf("%-3d", j+1);
    }
    printf("\n");


    // Résumé
    printf("\nResume:\n");
    for (int j = 0; j < nb_jours; j++) {
        printf("  Jour %d (%s): %.1f°C\n", j+1, dates[j], moyennes[j]);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// Implémentation de la file (Queue)
Queue* queue_new(void) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->tete = NULL;
    q->queue = NULL;
    q->taille = 0;
    return q;
}

void queue_free(Queue *q) {
    Node *courant = q->tete;
    while (courant) {
        Node *temp = courant;
        courant = courant->suivant;
        free(temp);
    }
    free(q);
}

int queue_send(Queue *q, float *temp) {
    Node *nouveau = (Node*)malloc(sizeof(Node));
    if (!nouveau) return -1;

    nouveau->temperature = *temp;
    nouveau->suivant = NULL;

    if (q->queue) {
        q->queue->suivant = nouveau;
    } else {
        q->tete = nouveau;
    }
    q->queue = nouveau;
    q->taille++;
    return 0;
}

float* queue_receive(Queue *q) {
    if (!q->tete) return NULL;

    Node *temp = q->tete;
    float *result = (float*)malloc(sizeof(float));
    *result = temp->temperature;

    q->tete = q->tete->suivant;
    if (!q->tete) q->queue = NULL;

    free(temp);
    q->taille--;
    return result;
}

float queue_front(Queue *q) {
    if (!q->tete) return 0.0f;
    return q->tete->temperature;
}

int queue_size(Queue *q) {
    return q->taille;
}


// Nouvelles fonctions pour le système
char* get_current_datetime_string(void) {
    time_t rawtime;
    struct tm *info;
    char *buffer = (char*)malloc(80 * sizeof(char));

    if (buffer == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);
    return buffer;
}

int niveau_temp(float temp, int seuil_min, int seuil_max) {
    if (temp < seuil_min) {
        float difference = seuil_min - temp;
        if (difference <= 5) return 1;
        if (difference <= 10) return 2;
        return 3;
    }
    if (temp > seuil_max) {
        float difference = temp - seuil_max;
        if (difference <= 5) return 1;
        if (difference <= 10) return 2;
        return 3;
    }
    return 0;
}

void journalisation(float temp, int seuil_min, int seuil_max) {
    FILE* journal = fopen(FICHIER_JOURNAL, "a");
    if (!journal) return;

    char *datetime_str = get_current_datetime_string();
    if (datetime_str) {
        int niv = niveau_temp(temp, seuil_min, seuil_max);

        // Extraire date et heure
        char date[11], heure[9];
        sscanf(datetime_str, "%10s %8s", date, heure);

        fprintf(journal, "%s %s - Temperature: %.2f C - Niveau alerte: %d\n",
                date, heure, temp, niv);
        free(datetime_str);
    }
    fclose(journal);
}

void alarme(const char *msg) {
    printf("\n \033[31m=== ALERTE ===\033[0m\n");
    printf("\033[31m %s\n\033[0m\n",msg);
    printf("\033[31m==============\033[0m\n");

    // un bip sonore ou flash visuel
    #ifdef _WIN32
        Beep(1000, 1000);
    #endif
}

void gerer_alert(Queue *F, float temp, int seuil_min, int seuil_max) {
    int niv = niveau_temp(temp, seuil_min, seuil_max);

    if (niv > 0) {
        // Ajouter à la file
        if (queue_send(F, &temp) == 0) {
            if (queue_size(F) == MAX_ALERTE) {
                int test = 1;
                Queue *f = queue_new();

                for (int i = 0; i < MAX_ALERTE; i++) {
                    float front_temp = queue_front(F);
                    if (niveau_temp(front_temp, seuil_min, seuil_max) == niv) {
                        queue_send(f, &front_temp);
                    } else {
                        test = 0;
                        // Reinitialisation de la file
                        while (queue_size(f) > 0) {
                                free(queue_receive(f));
                         }

                    }
                    free(queue_receive(F));
                }



                if (test == 1 && niv == 3) {
                    alarme("ATTENTION: ALERTE NIVEAU 3 - TEMPERATURE CRITIQUE!");
                } else if (test == 1 && niv == 2) {
                    alarme("ATTENTION: ALERTE NIVEAU 2 - TEMPERATURE ELEVEE");
                } else if (test == 1 && niv == 1) {
                    alarme("ALERTE NIVEAU 1 - TEMPERATURE ANORMALE");
                }
                F=f;
                queue_free(f);
            }
        }
    } else {
        // Reinitialisation la file si température normale
        while (queue_size(F) > 0) {
            free(queue_receive(F));
        }
    }
}





// FONCTION : LANCER SYSTEME


void demarrer_systeme(int seuil_min, int seuil_max, int intervalle, char mode, int horlogue_minutes) {
    printf("\n** DEBUT DU SYSTEME \n\n");
    printf("Seuils: %d-%d C, Intervalle: %ds, Mode: %c, Duree: %d min\n",
           seuil_min, seuil_max, intervalle, mode, horlogue_minutes);

    time_t debut = time(NULL);
    time_t fin = debut + (horlogue_minutes * 60);
    int continuer = 1;
    float temp;
    Queue *file_alertes = queue_new();

    // lire Température de test  une seul fois  pour le mode statique

    if (mode == '2') {
        printf("Entrez la temperature statique: ");
        scanf("%f", &temp);
        while (getchar() != '\n'); // Vider le buffer
    }

    while (time(NULL) < fin && continuer) {

         // Retourner les temperatures de test alea si  mode aleatoire
         if(mode == '1'){
            float min = -20.0;
            float max = 100.0;

            temp = min + ((float)rand() / RAND_MAX) * (max - min);}


           printf("Temp: %.1f C, Seuils: [%d-%d], Alerte: N%d\n",
               temp, seuil_min, seuil_max,
               niveau_temp(temp, seuil_min, seuil_max));
        // Journalisation
            journalisation(temp, seuil_min, seuil_max);

        // Gestion des alertes
            gerer_alert(file_alertes, temp, seuil_min, seuil_max);



        // Vérifier l'appui sur Entrée

            if (_kbhit()) {
                char c = _getch();
                if (c == '\r' || c == '\n') {
                    continuer = 0;
                }
            }


        if (continuer) {
                Sleep(intervalle* 1000);

        }
    }

    queue_free(file_alertes);
    printf("\n=== SYSTEME TERMINE ===\n");
}

