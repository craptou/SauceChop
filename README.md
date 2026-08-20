# SauceChop

SauceChop est un instrument audio VST3 de découpage créatif destiné en priorité à FL Studio sous Windows. Le plug-in charge un échantillon, le découpe en segments, permet de réorganiser et transformer chaque segment, puis joue ou exporte le résultat.

Le projet s'inspire de la catégorie des *sample choppers*, sans reprendre le code, le nom, l'identité visuelle ou les ressources d'un produit existant.

## Objectif actuel

Construire d'abord un MVP stable capable de :

- charger un fichier WAV, AIFF ou MP3 ;
- afficher sa forme d'onde ;
- créer 4, 8, 16 ou 32 slices égales ;
- lire, sélectionner et réordonner les slices ;
- appliquer gain, pitch par vitesse de lecture, reverse et fades ;
- déclencher les slices par MIDI ;
- randomiser l'ordre en respectant les verrous ;
- restaurer correctement une session FL Studio.

Le time-stretch avec conservation de hauteur, les formants et l'export par glisser-déposer arrivent après validation de ce noyau.

## Stack prévue

- C++20
- JUCE 8
- CMake
- Visual Studio 2022
- VST3 64 bits
- Windows 11 comme plateforme de développement initiale
- FL Studio et pluginval pour la validation

## Build rapide

Après un clone incluant les sous-modules :

```powershell
git submodule update --init --recursive
cmake --preset windows-vs2022
cmake --build --preset debug --parallel
ctest --preset debug
```

Le preset Windows place les artefacts dans `C:\SauceChopBuild\windows-vs2022`. Ce chemin ASCII évite un problème de génération des ressources JUCE lorsque le chemin du dépôt contient des caractères accentués.

Artefacts Debug :

```text
C:\SauceChopBuild\windows-vs2022\SauceChop_artefacts\Debug\VST3\SauceChop.vst3
C:\SauceChopBuild\windows-vs2022\SauceChop_artefacts\Debug\Standalone\SauceChop.exe
```

Pour tester dans FL Studio, copier le bundle `SauceChop.vst3` dans `C:\Program Files\Common Files\VST3\`, puis lancer un scan depuis le gestionnaire de plug-ins.

## Documentation

| Document | Contenu |
| --- | --- |
| [Vision produit](docs/PRODUCT.md) | Public, proposition de valeur et limites |
| [Spécifications](docs/SPECIFICATION.md) | Comportements fonctionnels et critères d'acceptation |
| [Architecture](docs/ARCHITECTURE.md) | Modules, données, threads et persistance |
| [Moteur audio](docs/AUDIO_ENGINE.md) | Lecture, slicing, MIDI, synchronisation et rendu |
| [Interface](docs/UI_UX.md) | Écrans, interactions et états visuels |
| [Feuille de route](docs/ROADMAP.md) | Jalons dans l'ordre de réalisation |
| [Backlog](docs/BACKLOG.md) | Epics et tâches prêtes à convertir en issues Git |
| [Build Windows](docs/BUILD_WINDOWS.md) | Outils, compilation et installation locale |
| [Tests](docs/TESTING.md) | Stratégie, matrices et critères de livraison |
| [Conventions Git](CONTRIBUTING.md) | Branches, commits, revues et définition de terminé |
| [Décisions](docs/DECISIONS.md) | Choix techniques actés ou à trancher |
| [Risques](docs/RISKS.md) | Risques techniques, produit et licence |
| [Livraison](docs/RELEASE.md) | Versionnement, paquets et checklist de publication |
| [Glossaire](docs/GLOSSARY.md) | Termes audio et conventions du projet |
| [Changelog](CHANGELOG.md) | Historique des changements visibles |

## Démarrage recommandé

1. Lire [la vision produit](docs/PRODUCT.md) et [les spécifications](docs/SPECIFICATION.md).
2. Trancher les décisions ouvertes marquées `D-OPEN` dans [DECISIONS.md](docs/DECISIONS.md).
3. Initialiser Git et créer le premier jalon depuis [BACKLOG.md](docs/BACKLOG.md).
4. Installer les outils décrits dans [BUILD_WINDOWS.md](docs/BUILD_WINDOWS.md).
5. Créer le squelette JUCE/CMake, puis obtenir un VST3 vide visible dans FL Studio.

## Statut

Jalon M3 terminé : WAV, AIFF et MP3 mono ou stéréo peuvent être chargés puis lus avec adaptation à la fréquence du host. Les commandes Play/Stop utilisent des fades anti-clic, le gain est lissé et un repère suit la lecture. La bande `ORDER` permet de sélectionner et réordonner les slices par glisser-déposer ; cette permutation est jouée, affichée sur la grande waveform et restaurée avec la session.
