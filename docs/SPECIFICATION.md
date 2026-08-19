# Spécification fonctionnelle

Les identifiants `FR-*` servent à relier exigences, issues et tests. La mention « MVP », « Bêta » ou « 1.0 » indique le premier jalon qui doit satisfaire l'exigence.

## Import et fichier source

### FR-001 — Importer un sample — MVP

L'utilisateur peut charger un fichier WAV, AIFF ou MP3 depuis un sélecteur ou par glisser-déposer sur le plug-in.

Critères d'acceptation :

- les fichiers mono et stéréo sont acceptés ;
- l'audio est converti au format de traitement interne sans changer sa vitesse ;
- un format invalide affiche une erreur non bloquante ;
- charger un nouveau fichier constitue une action annulable ;
- le thread audio n'effectue aucune lecture de fichier.

### FR-002 — Afficher le sample — Prototype

La forme d'onde représente l'intégralité du fichier et affiche les limites de slices. Le nom, la durée et le nombre de canaux sont visibles.

### FR-003 — Retrouver le fichier — MVP

L'état sauvegarde au minimum le chemin canonique, la taille, la date de modification et une empreinte du fichier. Si le fichier manque à la réouverture, le plug-in reste stable, signale le problème et propose de le relocaliser.

## Découpage

### FR-010 — Découpage égal — Prototype

L'utilisateur sélectionne 4, 8, 16 ou 32 slices. Chaque slice couvre une portion contiguë du sample ; la dernière absorbe les éventuels échantillons restants afin qu'aucun audio ne soit perdu.

### FR-011 — Sélection — MVP

Un clic sélectionne une slice. `Shift` applique l'édition à toutes les slices. La sélection actuelle est distincte de la tête de lecture.

### FR-012 — Réorganisation — MVP

Les slices peuvent être déplacées dans la séquence. La réorganisation change l'ordre de lecture, jamais les données du sample source.

### FR-013 — Reset de l'ordre — MVP

Une commande restaure l'ordre source `[0, 1, ..., N-1]` sans réinitialiser les effets individuels.

## Traitement par slice

### FR-020 — Gain — MVP

Chaque slice possède un gain de `-inf` à `+6 dB`. La valeur par défaut est `0 dB`. Les changements sont lissés pour éviter les clics.

### FR-021 — Pitch simple — MVP

Chaque slice peut être transposée de `-12` à `+12` demi-tons par changement de vitesse de lecture. Cette méthode modifie volontairement la durée dans le MVP.

### FR-022 — Reverse — MVP

Chaque slice peut être lue à l'envers sans modifier le buffer source.

### FR-023 — Fades anticlic — MVP

Chaque slice possède un fade-in et un fade-out, bornés afin de ne jamais dépasser ensemble la durée de la slice.

### FR-024 — Transposition globale — Bêta

Une transposition globale de `-12` à `+12` demi-tons s'ajoute au pitch individuel et respecte les bornes sûres du moteur.

### FR-025 — Time-stretch et formants — 1.0

Le moteur peut modifier durée et hauteur séparément. La préservation des formants est optionnelle par slice. La latence déclarée au DAW doit refléter le chemin de traitement utilisé.

## Lecture et MIDI

### FR-030 — Lecture interne — Prototype

Un bouton Play/Stop permet d'écouter la séquence sans lancer le transport du DAW.

### FR-031 — Déclenchement MIDI — MVP

Des notes consécutives déclenchent les slices. La note de base est configurable ; sa valeur initiale est le numéro MIDI 60. L'interface peut afficher « C5 » selon la convention de FL Studio, mais le stockage utilise toujours le numéro MIDI.

### FR-032 — Modes MIDI — MVP

- **One Shot** : la slice joue jusqu'à sa fin après Note On.
- **Gate** : la slice s'arrête au Note Off correspondant.

Les vélocités MIDI multiplient le gain de la voix.

### FR-033 — Polyphonie — MVP

Le moteur prend en charge au moins 16 voix. Quand la limite est atteinte, la voix la plus ancienne en phase de relâchement est volée en priorité.

### FR-034 — Synchronisation transport — Bêta

En mode Link, lecture, arrêt et position suivent le transport du DAW. Un déplacement de la tête de lecture recalcule immédiatement la position de séquence.

### FR-035 — Synchronisation tempo — Bêta

L'utilisateur indique la longueur musicale du sample source : 1/2, 1, 2 ou 4 mesures. Le moteur adapte la séquence au tempo du DAW. Les boutons `/2` et `x2` modifient ce ratio.

## Génération et historique

### FR-040 — Randomiser — MVP

Randomize réordonne seulement les positions non verrouillées. La permutation produite et la graine aléatoire sont stockées afin que le résultat soit reproductible.

### FR-041 — Verrouiller une position — MVP

Une slice verrouillée conserve son emplacement pendant Randomize. Le verrou ne l'empêche pas d'être sélectionnée ou jouée.

### FR-042 — Verrouiller les effets — Bêta

Une slice dont les effets sont verrouillés ne reçoit aucune modification d'effet lors des futures fonctions génératives.

### FR-043 — Rhythmize — Bêta

Rhythmize modifie uniquement l'ordre et les répétitions autorisées. Sa définition musicale exacte doit faire l'objet d'une décision avant implémentation.

### FR-044 — Undo/redo — MVP

L'utilisateur peut annuler et rétablir au minimum : chargement, découpage, ordre, randomisation, verrous et paramètres de slices. Le nombre minimal d'états conservés est 50.

## Export

### FR-050 — Rendu audio — Bêta

Une commande produit un WAV 32-bit float contenant exactement une répétition de la séquence. Le rendu utilise les mêmes règles de lecture que le moteur temps réel.

### FR-051 — Glisser vers le DAW — Bêta

Après rendu sur un thread de travail, l'utilisateur peut déposer le WAV dans FL Studio. Le fichier temporaire reste valide au moins jusqu'à la fermeture de l'instance du plug-in.

## État et automatisation

### FR-060 — Sauvegarde de session — MVP

Les paramètres globaux, slices, ordre, verrous, graine, note MIDI et référence du fichier sont sérialisés dans un `ValueTree` versionné.

### FR-061 — Compatibilité ascendante — Bêta

Chaque état contient un numéro de schéma. Une migration permet d'ouvrir les états des versions antérieures prises en charge.

### FR-062 — Paramètres automatisables — MVP

Les paramètres globaux utiles sont exposés au DAW. Les paramètres de chaque slice restent internes dans le MVP pour éviter une liste instable de paramètres VST lors d'un changement de nombre de slices.

## Erreurs et sécurité

### FR-070 — Audio manquant — MVP

Un fichier manquant n'entraîne ni crash ni silence ambigu : un bandeau indique le problème et propose « Relocaliser ».

### FR-071 — Fichier trop long — MVP

Une limite configurable est appliquée à la durée ou à la mémoire décodée. Le défaut initial proposé est 10 minutes ou 1 GiB décodé, la première limite atteinte étant utilisée.

### FR-072 — Sortie sûre — MVP

Le plug-in empêche NaN, infini et dépassements non bornés. Un limiteur n'est pas imposé au son, mais un garde-fou final protège le moteur pendant le développement.

## Exigences non fonctionnelles

- **NFR-001** : aucune allocation, ouverture de fichier ou attente de thread dans `processBlock()` en régime stable.
- **NFR-002** : le projet compile en 64 bits avec avertissements élevés et avertissements traités progressivement.
- **NFR-003** : le plug-in reste fonctionnel avec blocs de 32 à 2048 samples et fréquences de 44,1 à 192 kHz.
- **NFR-004** : l'interface reste utilisable de 100 % à 200 % de mise à l'échelle Windows.
- **NFR-005** : les actions longues montrent une progression et peuvent échouer sans bloquer le DAW.
- **NFR-006** : les données du sample source restent immuables après import.
