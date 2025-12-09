#ifndef CONFIG_CONTROLLER_H
#define CONFIG_CONTROLLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define CONFIG_PATH "config.txt"
#define INPUT_BUFFER 64

typedef struct
{
    int seuil_min;
    int seuil_max;
    int intervalle_mesure;
} Configuration;

// Fonctions publiques

void configuration_module( Configuration *configuration);
char choisir_mode_test(void);
int lire_configuration(Configuration *configuration);
int ecrire_configuration(const Configuration *configuration);




#endif // CONFIG_CONTROLLER_H
