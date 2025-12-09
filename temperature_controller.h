#ifndef TEMPERATURE_CONTROLLER_H
#define TEMPERATURE_CONTROLLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Chemins
#define FICHIER_JOURNAL "journal.txt"
#define FICHIER_RAPPORT "rapport_journalier.txt"
#define MAX_ALERTE 2

// Structure pour une mesure
typedef struct Mesure {
    char date[11];
    char heure[9];
    float temperature;
    int alerte;
    struct Mesure *suivant;
} Mesure;

// Structure pour les statistiques par jour
typedef struct Stats {
    char date[11];
    float min, max, somme;
    int nb_mesures;
    int alertes1, alertes2, alertes3;
    int duree_alerte_sec;
    struct Stats *suivant;
} Stats;

typedef struct {
    int total_jours;
    int total_mesures;
    int total_alertes1;
    int total_alertes2;
    int total_alertes3;
    int total_alertes;
    int total_duree_alerte_sec;
    float temp_min_globale;
    float temp_max_globale;
    float moyenne_globale;
    float somme_temperatures;
} ResumeGlobal;

// Structure pour la file d'alertes
typedef struct Node {
    float temperature;
    struct Node *suivant;
} Node;

typedef struct {
    Node *tete;
    Node *queue;
    int taille;
} Queue;

// Prototypes des fonctions
Queue* queue_new(void);
void queue_free(Queue *q);
int queue_send(Queue *q, float *temp);
float* queue_receive(Queue *q);
float queue_front(Queue *q);
int queue_size(Queue *q);

Mesure* ajouter_mesure(Mesure *tete, char *date, char *heure, float temp, int alerte);
Mesure* lire_journal(void);
Stats* obtenir_stats(Stats **tete, char *date);
Stats* calculer_stats(Mesure *mesures, int intervalle_mesure);
void afficher_stats(Stats *stats, int intervalle_mesure);
void generer_rapport(Stats *stats, int intervalle_mesure);
ResumeGlobal calculer_resume_global(Stats *stats);
void afficher_recommandations(ResumeGlobal resume);
void liberer_mesures(Mesure *m);
void liberer_stats(Stats *s);
int compter_mesures(Mesure *m);

// Nouvelles fonctions pour le système
char* get_current_datetime_string(void);
int niveau_temp(float temp, int seuil_min, int seuil_max);
void journalisation(float temp, int seuil_min, int seuil_max);
void alarme(const char *msg);
void gerer_alert(Queue *F, float temp, int seuil_min, int seuil_max);
int checkKeyPress(void);
float capteur_temperature(char mode, float temp_static);
void demarrer_systeme(int seuil_min, int seuil_max, int intervalle, char mode, int horlogue_minutes);
void get_terminal_size(int *rows, int *cols);
void log_display_message(const char* message);


void afficher_courbe_ascii_complete(Stats *stats);
#endif
