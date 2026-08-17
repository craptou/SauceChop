# Développement et build sous Windows

Ce document décrit la cible initiale Windows x64. Les numéros exacts de versions seront figés dans les fichiers de build lors du bootstrap.

## Prérequis

- Windows 11 64 bits ;
- Visual Studio 2022 avec le workload **Desktop development with C++** ;
- MSVC x64 et Windows SDK ;
- CMake récent ;
- Git ;
- JUCE 8 ;
- FL Studio 64 bits ;
- pluginval pour les validations automatisables.

Avant de distribuer un binaire fermé, vérifier que la formule JUCE choisie correspond aux revenus/financements et conditions en vigueur. Conserver dans le dépôt les notices requises par toutes les dépendances.

## Dépendances

Choix initial recommandé : JUCE comme sous-module Git fixé à un tag précis.

Arborescence envisagée :

```text
external/
  JUCE/
  signalsmith-stretch/  # ajouté seulement au jalon M7
```

Ne pas suivre directement une branche mouvante dans une release. Toute mise à jour de dépendance doit être un commit isolé et testé.

## Configuration CMake envisagée

Depuis PowerShell :

```powershell
git submodule update --init --recursive
cmake --preset windows-vs2022
cmake --build --preset debug --parallel
ctest --preset debug
```

Pour une release locale :

```powershell
cmake --build --preset release --parallel
ctest --preset release
```

Les cibles individuelles sont `SauceChop_VST3`, `SauceChop_Standalone` et `SauceChopTests`.

### Chemin de build ASCII

Le preset écrit dans :

```text
C:\SauceChopBuild\windows-vs2022
```

JUCE 8.0.15 peut échouer pendant la génération des ressources Windows lorsque le chemin de build contient des caractères accentués. Le dépôt actuel se trouve sous un nom utilisateur contenant `ë` ; le dossier externe ASCII évite ce problème sans déplacer les sources. Il peut être supprimé et entièrement régénéré depuis Git.

## Formats construits

Pendant le développement :

- `VST3` pour FL Studio ;
- `Standalone` pour accélérer certains tests UI/audio.

Le Standalone n'est pas un livrable 1.0 tant qu'il n'est pas explicitement ajouté au périmètre produit.

## Installation locale

Le VST3 Debug est produit ici :

```text
C:\SauceChopBuild\windows-vs2022\SauceChop_artefacts\Debug\VST3\SauceChop.vst3
```

Le chemin VST3 système habituel est :

```text
C:\Program Files\Common Files\VST3\
```

Préférer une étape CMake contrôlée ou une copie explicite du bundle `SauceChop.vst3`. Ne jamais automatiser la suppression récursive du dossier VST3 global.

Dans FL Studio :

1. ouvrir le gestionnaire de plug-ins ;
2. lancer une recherche de nouveaux plug-ins ;
3. vérifier que SauceChop apparaît comme générateur/instrument ;
4. ouvrir l'interface et enregistrer un projet de test.

## Configurations

- **Debug** : assertions, symboles, protections et logs locaux ; peut être trop lente pour juger le DSP avancé.
- **RelWithDebInfo** : configuration préférée pour profiler.
- **Release** : utilisée pour la validation finale et la distribution.

## Dossiers générés

Les éléments suivants ne doivent pas être versionnés :

```text
build/
cmake-build-*/
.vs/
*.user
*.suo
*.VC.db
```

La future `.gitignore` couvrira aussi les binaires, symboles, caches JUCE et fichiers temporaires de rendu.

## Diagnostic minimal

Si FL Studio ne trouve pas le plug-in :

- vérifier que la cible est x64 ;
- vérifier le chemin réel du bundle `.vst3` ;
- relancer un scan avec vérification des plug-ins ;
- confirmer qu'aucune ancienne copie portant le même identifiant VST3 n'est chargée ;
- tester le même binaire avec pluginval ;
- consulter le débogueur Visual Studio attaché au processus FL Studio seulement après avoir reproduit le problème dans Standalone/pluginval.

## Reproductibilité

Le dépôt doit fixer :

- tag/commit JUCE ;
- version CMake minimale ;
- standard C++ ;
- identifiants fabricant et plug-in ;
- options de build importantes ;
- version des dépendances DSP.

Un build de release ne doit pas dépendre d'un chemin absolu propre à la machine d'un développeur.
