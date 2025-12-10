# Controleur Climatique (C)

Projet C de simulation d'un controleur de temperature avec journalisation et generation de rapports. L'application est interactive en console et s'appuie sur un fichier de configuration `config.txt` pour les seuils et l'intervalle de mesure.

## Arborescence
- `main.c` : menu principal.
- `config_controller.c/.h` : lecture/edition de la configuration, choix du mode capteur, utilitaires.
- `temperature_controller.c/.h` : boucle de mesure, journalisation, alertes, stats et rapports.
- `config.txt` : configuration (exemple par defaut: `seuil_min=10`, `seuil_max=30`, `intervalle_mesure=3`).
- `journal.txt` : journal des mesures.
- `rapport_journalier.txt` : rapport genere a la demande.
- `output/` : binaire genere (`climate.exe`).

## Prerequis
- GCC (testé avec MSYS2 `ucrt64` sur Windows).
- Lancer depuis le dossier racine du projet pour que les chemins relatifs (config, journal, rapport) fonctionnent.

## Compilation (Windows / PowerShell)
```pwsh
# depuis la racine du projet
cd .\FinalCProject
gcc -Wall -Wextra -std=c11 -g main.c config_controller.c temperature_controller.c -o output\climate.exe
# si gcc n'est pas dans le PATH, remplacez `gcc` par le chemin complet vers votre binaire, par ex. C:\msys64\ucrt64\bin\gcc.exe
```

## Execution
```pwsh
# rester dans le dossier racine, sinon copier aussi config.txt
./output/climate.exe
```

## Utilisation rapide
1. Au demarrage, le programme lit `config.txt`. Si absent/invalid, des valeurs par defaut sont appliquees.
2. Menu principal :
   - `1` Gerer configuration : editer seuils min/max et intervalle, puis sauvegarder dans `config.txt`.
   - `2` Choisir mode capteur : `1` aleatoire (0-60 C), `2` statique (valeur saisie).
   - `3` Lancer le systeme : boucle pendant N minutes, mesure toutes `intervalle_mesure` secondes, journalise dans `journal.txt`, leve des alertes N1/N2/N3.
   - `4` Rapports : lit `journal.txt`, calcule stats par jour, ecrit `rapport_journalier.txt`, affiche stats et recommandations.
   - `0` Quitter.

## Fichiers generes
- `journal.txt` : `YYYY-MM-DD HH:MM:SS - Temperature: XX.XX C - Niveau alerte: N`
- `rapport_journalier.txt` : resume par jour (min/max/moyenne, alertes, duree alerte) + resume global.

## Conseils
- Toujours executer depuis le dossier racine pour que les fichiers soient trouves/crees au bon endroit.
- En mode statique, saisir une temperature initiale quand le systeme le demande.
- Ajuster `seuil_min`, `seuil_max` et `intervalle_mesure` dans `config.txt` ou via le menu avant de lancer la simulation.
