# Backlog initial

Chaque ligne peut devenir une issue. Les identifiants restent stables même si l'ordre change.

## EPIC-01 — Fondation

- `DEV-001` ✅ Initialiser Git et ajouter un `.gitignore` C++/JUCE/Visual Studio.
- `DEV-002` Choisir la licence du dépôt et documenter la licence JUCE.
- `DEV-003` ✅ Créer `CMakeLists.txt` avec JUCE et cibles VST3/Standalone.
- `DEV-004` ✅ Ajouter les scripts/presets CMake de développement.
- `DEV-005` ✅ Activer formatage C++ et avertissements du compilateur.
- `DEV-006` ✅ Ajouter une CI Windows x64.
- `DEV-007` Documenter la machine de benchmark.

## EPIC-02 — Host et état

- `HOST-001` ✅ Créer le processeur instrument stéréo MIDI.
- `HOST-002` ✅ Créer l'éditeur redimensionnable minimal.
- `HOST-003` ✅ Définir les identifiants de paramètres VST stables.
- `HOST-004` ✅ Implémenter le premier `ValueTree` de paramètres.
- `HOST-005` Ajouter le squelette de migration de schéma.
- `HOST-006` Tester sauvegarde/rechargement dans FL Studio.

## EPIC-03 — Import audio

- `FILE-001` ✅ Charger WAV mono/stéréo.
- `FILE-002` ✅ Charger AIFF mono/stéréo.
- `FILE-002B` ✅ Charger MP3 mono/stéréo via le codec natif de la plateforme.
- `FILE-003` ✅ Appliquer les limites de taille/mémoire.
- `FILE-004` Construire `SampleIdentity` et son hash.
- `FILE-005` ✅ Publier un nouveau `SourceSample` sans bloquer l'audio.
- `FILE-006` Gérer fichier absent, modifié et relocalisé.
- `FILE-007` Créer un corpus de fichiers de test.

## EPIC-04 — Forme d'onde et slices

- `UI-001` ✅ Construire un cache min/max de waveform.
- `UI-002` ✅ Dessiner la forme d'onde depuis un cache borné.
- `SLICE-001` ✅ Calculer 4/8/16/32 frontières égales.
- `SLICE-002` ✅ Afficher limites, numéros et sélection.
- `SLICE-003` ✅ Implémenter la bande d'ordre.
- `SLICE-004` ✅ Implémenter le drag-reorder avec prévisualisation.
- `SLICE-005` ✅ Implémenter reset de l'ordre.

## EPIC-05 — Moteur temps réel

- `DSP-001` Définir l'`EngineSnapshot` complet (la publication sûre du sample source est prête).
- `DSP-002` ✅ Implémenter une voix mono/stéréo bornée.
- `DSP-003` ✅ Implémenter conversion de fréquence et interpolation.
- `DSP-004` ✅ Ajouter gain lissé et fades.
- `DSP-005` Ajouter reverse.
- `DSP-006` Ajouter pitch par ratio de vitesse.
- `DSP-007` Implémenter pool de 16 voix et voice stealing.
- `DSP-008` Ajouter protections NaN/infini en Debug.
- `DSP-009` Mesurer allocations et CPU dans `processBlock()`.

## EPIC-06 — MIDI et séquence

- `MIDI-001` Mapper les notes consécutives depuis la note 60.
- `MIDI-002` Respecter les offsets d'événements dans le bloc.
- `MIDI-003` Implémenter One Shot.
- `MIDI-004` Implémenter Gate et Note Off.
- `SEQ-001` ✅ Implémenter la lecture interne de l'ordre réorganisé.
- `SEQ-002` ✅ Implémenter Play/Stop et reprise déterministe.

## EPIC-07 — Création et historique

- `GEN-001` Implémenter Fisher-Yates avec graine stockée.
- `GEN-002` Respecter les verrous de position.
- `GEN-003` Ajouter verrouillage/déverrouillage multiple.
- `GEN-004` Ajouter UndoManager et commandes réversibles.
- `GEN-005` Couvrir randomisation et verrous par tests de propriétés.

## EPIC-08 — Synchronisation

- `SYNC-001` Lire tempo, PPQ, signature et transport.
- `SYNC-002` Détecter les discontinuités du transport.
- `SYNC-003` Mapper longueur source et mesures.
- `SYNC-004` Planifier les pas aux offsets précis du bloc.
- `SYNC-005` Implémenter moitié/double vitesse.
- `SYNC-006` Tester automation de tempo et boucles FL Studio.

## EPIC-09 — Export

- `EXPORT-001` Créer un moteur offline depuis un snapshot.
- `EXPORT-002` Écrire un WAV 32-bit float transactionnel.
- `EXPORT-003` Ajouter progression et annulation.
- `EXPORT-004` Déposer le fichier dans FL Studio.
- `EXPORT-005` Concevoir un nettoyage sûr des temporaires.
- `EXPORT-006` Comparer sortie temps réel et offline.

## EPIC-10 — Time-stretch

- `STRETCH-001` Valider licence et intégration de Signalsmith Stretch.
- `STRETCH-002` Implémenter l'abstraction `TimeStretchEngine`.
- `STRETCH-003` Construire le cache de traitements en arrière-plan.
- `STRETCH-004` Ajouter compensation des formants.
- `STRETCH-005` Gérer latence et transitions de paramètres.
- `STRETCH-006` Créer une campagne d'écoute et de performance.

## EPIC-11 — Livraison

- `REL-001` Finaliser nom, identifiants VST et visuels.
- `REL-002` Ajouter presets d'usine originaux.
- `REL-003` Créer installateur et désinstallateur.
- `REL-004` Générer avis de licences tierces.
- `REL-005` Rédiger manuel utilisateur et changelog.
- `REL-006` Exécuter toute la matrice de tests.
- `REL-007` Signer les binaires/installateur si un certificat est disponible.

## Priorité immédiate

Ordre recommandé pour les premières issues : `DEV-001`, `DEV-002`, `DEV-003`, `HOST-001`, `HOST-002`, `HOST-006`, puis `FILE-001`.
