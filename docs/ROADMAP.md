# Feuille de route

La roadmap est organisée par résultats vérifiables. Une phase n'est terminée que lorsque son critère de sortie est satisfait.

## M0 — Cadrage et dépôt

Résultat : le projet est traçable et reproductible avant l'ajout de code audio.

- vérifier la disponibilité du nom SauceChop avant toute publication ;
- choisir la licence du dépôt et la formule JUCE adaptée ;
- initialiser Git ;
- ajouter `.gitignore`, `LICENSE` et règles de formatage ;
- créer le projet CMake/JUCE minimal ;
- activer une CI de compilation Windows lorsque le dépôt distant existe.

Critère de sortie : un nouveau clone peut configurer et compiler un exécutable ou plug-in vide en suivant `BUILD_WINDOWS.md`.

## M1 — Coquille VST3

Résultat : FL Studio découvre et ouvre une instance stable.

- implémenter `PluginProcessor` et `PluginEditor` ;
- déclarer le plug-in comme instrument MIDI avec sortie stéréo ;
- ajouter paramètres globaux temporaires et sauvegarde minimale ;
- construire VST3 et Standalone en Debug ;
- valider les cycles ajout/suppression et ouverture/fermeture d'éditeur.

Critère de sortie : 100 ouvertures/fermetures et 50 ajouts/suppressions sans crash ni fuite évidente.

## M2 — Chargement et visualisation

Résultat : un WAV, AIFF ou MP3 peut être chargé et affiché sans toucher au thread audio.

Statut : en cours. Le chargement, les limites, la publication transactionnelle et la waveform sont implémentés ; le hash complet, la relocalisation et le corpus étendu restent à faire.

- chargeur asynchrone ;
- contrôles de taille et formats ;
- `SourceSample` immuable ;
- cache de forme d'onde ;
- remplacement transactionnel et erreurs utilisateur ;
- identité du fichier et première sérialisation versionnée.

Critère de sortie : corpus d'au moins 30 fichiers valides/invalides chargé sans blocage ou crash.

## M3 — Slicing et lecture

Résultat : la séquence de slices produit du son.

Statut : en cours. La lecture dans l'ordre source, la conversion de fréquence, Play/Stop, les fades et le repère visuel sont implémentés. La bande d'ordre et le réarrangement restent à faire.

- découpage 4/8/16/32 ;
- moteur de voix ;
- interpolation et conversion de fréquence ;
- lecture interne Play/Stop ;
- ordre source et réorganisation ;
- repère de lecture dans l'UI.

Critère de sortie : aucune lecture hors limites sous sanitizers/tests, et sortie correcte à 44,1/48/96 kHz.

## M4 — MVP créatif

Résultat : le plug-in est utile pour créer une variation et la rejouer.

- gain, pitch par vitesse, reverse et fades ;
- MIDI One Shot/Gate et polyphonie ;
- randomisation déterministe ;
- verrou de position, reset, undo/redo ;
- état complet et relocalisation de sample ;
- optimisation du callback audio.

Critère de sortie : tous les critères MVP de `SPECIFICATION.md` sont couverts par tests ou scénarios manuels documentés.

## M5 — Synchronisation DAW

Résultat : une boucle reste calée sur FL Studio.

- position PPQ et transport ;
- longueur source en mesures ;
- changements de BPM ;
- déplacements de tête de lecture ;
- `/2` et `x2` ;
- définir la politique de coupure/chevauchement.

Critère de sortie : 20 minutes de lecture avec changements de tempo et boucles de transport sans dérive audible ou événement perdu.

## M6 — Export et Bêta

Résultat : le son entendu peut être déposé comme audio dans FL Studio.

- moteur de rendu partagé ;
- WAV temporaire 32-bit float ;
- progression/annulation ;
- glisser-déposer externe ;
- nettoyage sûr des fichiers de cette instance ;
- verrou des effets et Rhythmize défini ;
- première série de testeurs.

Critère de sortie : 50 exports consécutifs conformes, y compris après undo, changement de sample et modification du nombre de slices.

## M7 — Qualité DSP et 1.0

Résultat : version distribuable avec pitch/time indépendants.

- intégrer le moteur de time-stretch derrière l'abstraction ;
- conservation optionnelle des formants ;
- mesurer/déclarer la latence ;
- presets et migrations d'état ;
- design system et polish UI ;
- installateur signé si possible ;
- matrice de compatibilité et guide utilisateur ;
- audit licences et attributions.

Critère de sortie : checklist de `RELEASE.md` complète, aucun bug bloquant ou critique ouvert.

## Après 1.0

Les thèmes suivants ne sont pas promis avant validation par des utilisateurs : détection de transitoires, patterns A/B, probabilités, export MIDI, macOS/AU, application autonome et traitement d'une entrée audio en direct.
