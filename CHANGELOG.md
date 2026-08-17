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
