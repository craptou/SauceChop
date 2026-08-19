# Journal des décisions

Les décisions acceptées ne sont pas immuables, mais toute modification doit expliquer pourquoi et quelles migrations elle exige.

## Décisions acceptées

### D-001 — VST3 Windows x64 en premier

- **Statut** : accepté
- **Motif** : cible prioritaire FL Studio, périmètre réduit et chaîne de compilation unique.
- **Conséquence** : macOS/AU et autres formats ne doivent pas ralentir le MVP.

### D-002 — C++20, JUCE 8 et CMake

- **Statut** : accepté sous réserve de la décision de licence JUCE
- **Motif** : écosystème éprouvé pour plug-ins, UI, audio, MIDI, état et VST3.
- **Conséquence** : Visual Studio 2022 et une version CMake minimale seront fixés dans le dépôt.

### D-003 — Sample source immuable

- **Statut** : accepté
- **Motif** : simplifier reverse, undo, réorganisation et partage entre UI/moteur.
- **Conséquence** : les slices stockent des plages et paramètres, jamais des copies modifiées du sample.

### D-004 — Pitch par vitesse avant time-stretch

- **Statut** : accepté
- **Motif** : valider le produit et la stabilité temps réel avant le DSP le plus risqué.
- **Conséquence** : le pitch du MVP modifie la durée et cette limite doit être visible dans le produit.

### D-005 — État versionné et paramètres de slices internes

- **Statut** : accepté
- **Motif** : préserver des identifiants VST stables quand le nombre de slices change.
- **Conséquence** : l'automatisation individuelle par slice n'est pas promise dans le MVP.

### D-006 — Rendu audio sur thread de travail

- **Statut** : accepté
- **Motif** : le rendu et l'écriture disque sont incompatibles avec les contraintes du callback audio.
- **Conséquence** : l'UI doit exposer progression, succès, erreur et annulation.

### D-007 — Randomisation déterministe

- **Statut** : accepté
- **Motif** : restauration exacte, tests reproductibles et expérience d'undo fiable.
- **Conséquence** : graine et résultat doivent faire partie de l'état.

### D-008 — Décodage MP3 natif sur Windows

- **Statut** : accepté
- **Motif** : JUCE enregistre déjà le codec Windows Media capable de lire le MP3 sur notre plateforme cible.
- **Conséquence** : `JUCE_USE_MP3AUDIOFORMAT` reste désactivé afin de ne pas intégrer le décodeur logiciel accompagné d'un avertissement de propriété intellectuelle. Une édition Windows dépourvue des composants multimédias requis peut nécessiter l'installation des fonctionnalités média du système.

## Décisions ouvertes

### D-OPEN-001 — Disponibilité du nom SauceChop

- **Question** : le nom SauceChop peut-il être utilisé et protégé sur les marchés visés ?
- **À vérifier** : marques, noms de domaine, magasins de plug-ins et identifiants sociaux avant publication.
- **Échéance** : avant la création des identifiants définitifs VST/fabricant.

### D-OPEN-002 — Licence du code

- **Options** : dépôt privé/propriétaire, licence open source compatible, ou source-available.
- **Impact** : conditions JUCE, contributions, distribution et choix des dépendances.
- **Échéance** : avant d'ajouter `LICENSE` ou d'accepter une contribution externe.

### D-OPEN-003 — Formule JUCE

- **Question** : utilisation sous licence commerciale JUCE ou sous AGPL avec obligations correspondantes ?
- **Impact** : distribution fermée, coûts et mentions.
- **Échéance** : avant toute diffusion d'un binaire à l'extérieur de l'équipe autorisée.

### D-OPEN-004 — Gestion portable des samples

- **Options** : chemin+hash seulement ; copie dans un cache géré ; intégration optionnelle dans le projet/preset.
- **Impact** : taille des projets, portabilité, vie privée et nettoyage disque.
- **Échéance** : avant la fin de M4.

### D-OPEN-005 — Framework de tests

- **Options** : JUCE UnitTest, Catch2 ou doctest.
- **Impact** : dépendance, intégration CMake et ergonomie.
- **Échéance** : pendant M0.

### D-OPEN-006 — Politique des voix en mode Sync

- **Options** : couper à chaque pas, autoriser chevauchement, ou mode configurable.
- **Impact** : groove, CPU, durée des pitches et cohérence d'export.
- **Échéance** : avant M5.

### D-OPEN-007 — Moteur de time-stretch

- **Option principale** : Signalsmith Stretch, à benchmarker.
- **Alternatives** : autre bibliothèque compatible ou solution interne.
- **Impact** : qualité, latence, CPU et licence.
- **Échéance** : avant M7.

### D-OPEN-008 — Cache ou temps réel pour le stretch

- **Question** : pré-calculer les variantes ou traiter chaque voix en direct ?
- **Impact** : réactivité, mémoire, polyphonie et complexité.
- **Échéance** : après un prototype DSP mesuré.

### D-OPEN-009 — Définition de Rhythmize

- **Question** : permutation, répétitions, silences, probabilités ou règles de grille ?
- **Impact** : modèle de données et compréhension utilisateur.
- **Échéance** : avant la Bêta.

## Modèle pour une nouvelle décision

```text
### D-XXX — Titre

- Statut : proposé | accepté | remplacé | rejeté
- Contexte :
- Options considérées :
- Décision :
- Conséquences :
- Date :
```
