# Registre des risques

Échelle : probabilité et impact de 1 (faible) à 5 (très fort). Le score est leur produit.

| ID | Risque | P | I | Score | Réduction |
| --- | --- | ---: | ---: | ---: | --- |
| R-01 | blocage du thread audio | 3 | 5 | 15 | snapshots immuables, files bornées, mesures d'allocations |
| R-02 | restauration impossible si le sample est déplacé | 4 | 5 | 20 | identité+hash, relocalisation, décision de cache avant M4 |
| R-03 | qualité insuffisante du pitch/time-stretch | 4 | 4 | 16 | MVP simple, abstraction, benchmark et tests d'écoute |
| R-04 | dérive ou sauts en synchronisation DAW | 3 | 5 | 15 | planification par PPQ, tests seek/tempo/boucle |
| R-05 | export différent de l'écoute | 3 | 5 | 15 | logique moteur partagée et tests de comparaison |
| R-06 | explosion du périmètre | 5 | 4 | 20 | jalons stricts, hors-périmètre explicite, validation du MVP |
| R-07 | conditions de licence incompatibles | 2 | 5 | 10 | choix de licence tôt, inventaire et audit avant release |
| R-08 | conflit de nom ou identité trop proche | 3 | 4 | 12 | nom provisoire, recherche de marque avant identifiants finaux |
| R-09 | état VST trop volumineux | 3 | 4 | 12 | ne pas intégrer l'audio par défaut, mesurer la sérialisation |
| R-10 | consommation mémoire des longs fichiers | 3 | 4 | 12 | limites strictes, estimation avant décodage, messages clairs |
| R-11 | destruction d'objet sur le thread audio | 3 | 5 | 15 | stratégie de publication testée, destruction différée |
| R-12 | drag-and-drop externe variable selon le host | 3 | 3 | 9 | prototype tôt dans FL Studio, fallback « Save As » |
| R-13 | UI lente avec longue waveform | 3 | 3 | 9 | cache min/max multirésolution et rendu partiel |
| R-14 | bugs dus aux fréquences/tailles de blocs | 3 | 4 | 12 | matrice automatisée et aucun ratio implicite |
| R-15 | dépendance à un seul DAW | 2 | 3 | 6 | respecter VST3/JUCE, tester pluginval et un second host avant 1.0 |

## Risques à traiter immédiatement

### R-02 — Portabilité du sample

Le chemin seul est insuffisant. Le MVP doit savoir distinguer fichier manquant et fichier remplacé. Avant les premiers testeurs externes, décider si le plug-in copie le sample dans un cache géré ou propose une option autonome.

### R-06 — Périmètre

Les fonctions suivantes restent hors MVP : time-stretch avancé, formants, détection de transitoires, patterns multiples et macOS. Une idée nouvelle entre au backlog mais ne change pas le jalon courant sans décision explicite.

### R-11 — Durée de vie des snapshots

Une publication atomique ne suffit pas si la dernière référence est libérée dans `processBlock()`. Le prototype doit instrumenter création/destruction et démontrer que la libération lourde se produit hors audio.

## Revue

Mettre ce registre à jour :

- à la fin de chaque jalon ;
- lors de l'ajout d'une dépendance ;
- après tout crash du host ou corruption d'état ;
- avant une diffusion à de nouveaux testeurs.

