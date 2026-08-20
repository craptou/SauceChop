# Changelog

Les changements notables de SauceChop sont consignés dans ce fichier. Le format suit l'esprit de *Keep a Changelog* et les versions produit suivent SemVer.

## Unreleased

### Added

- documentation initiale du produit ;
- spécification fonctionnelle et non fonctionnelle ;
- architecture du plug-in et du moteur audio ;
- roadmap, backlog, stratégie de tests et processus de release ;
- conventions de contribution et gabarits Git.
- sous-module JUCE fixé à la version 8.0.15 ;
- projet CMake avec cibles VST3, Standalone et tests ;
- processeur instrument MIDI stéréo silencieux et état de paramètres persistant ;
- interface redimensionnable du premier jalon ;
- calcul overflow-safe des slices égales et tests unitaires associés ;
- presets CMake Windows avec dossier de build ASCII.
- CI GitHub Actions pour compiler et tester sous Windows x64.
- chargement asynchrone des samples WAV et AIFF mono/stéréo ;
- limites de sécurité de 10 minutes et 1 GiB décodé ;
- cache de pics de waveform calculé hors du thread audio ;
- import par sélecteur de fichier ou glisser-déposer ;
- affichage de la waveform, des informations du sample et des divisions 4/8/16/32 ;
- restauration du chemin et de l'identité de base du sample avec l'état du plug-in ;
- tests de décodage WAV/AIFF/MP3 et de génération des pics.
- import et décodage MP3 via le codec multimédia natif de Windows.
- moteur de lecture mono/stéréo sans allocation dans le callback audio ;
- adaptation de fréquence par interpolation linéaire et accès source bornés ;
- commandes Play/Stop avec fades anti-clic et gain de sortie lissé ;
- publication du sample par pointeur protégé afin que sa destruction reste hors du thread audio ;
- repère de lecture animé sur la waveform et suivi de la slice active ;
- tests du moteur à fréquences identiques et différentes, des limites de slices et des arrêts.

### Changed

- rien pour le moment.

### Fixed

- rien pour le moment.

### Security

- rien pour le moment.

## Règles de mise à jour

- ajouter les changements utilisateur ou développeur significatifs sous `Unreleased` ;
- ne pas lister chaque commit ou refactor interne sans impact ;
- lors d'une release, déplacer les entrées sous `## [x.y.z] — AAAA-MM-JJ` ;
- créer une nouvelle section `Unreleased` vide ;
- relier ultérieurement les numéros de version à leurs tags Git lorsque le dépôt distant existe.
