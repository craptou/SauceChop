# Stratégie de tests

## Niveaux de validation

### Tests unitaires

À exécuter sans DAW :

- frontières du découpage ;
- conversion demi-tons/ratio ;
- bornage des fades ;
- mapping MIDI ;
- permutation et verrous ;
- sérialisation/migrations ;
- identité des fichiers ;
- règles de voice stealing ;
- conversion PPQ/pas de séquence.

### Tests de propriétés

Particulièrement utiles pour :

- aucune frontière hors de `[0, frameCount]` ;
- couverture complète et sans trou du découpage égal ;
- randomisation toujours permutation valide ;
- positions verrouillées invariantes ;
- sérialiser puis désérialiser conserve l'état ;
- aucune sortie NaN/infini pour des paramètres valides.

### Tests d'intégration

- chargement de corpus audio ;
- publication d'un sample pendant la lecture ;
- MIDI avec offsets au milieu d'un bloc ;
- rendu offline comparé au chemin temps réel ;
- sample manquant puis relocalisé ;
- états d'anciennes versions migrés ;
- annulation pendant un rendu ou chargement.

### Validation de plug-in

Exécuter pluginval avec sévérité croissante pendant le développement. Archiver dans la CI ou la release le rapport de la configuration distribuée.

### Tests dans FL Studio

Scénarios manuels obligatoires :

1. découverte après installation ;
2. ajout et suppression répétés ;
3. ouverture/fermeture et redimensionnement de l'éditeur ;
4. import par dialogue et glisser-déposer ;
5. sauvegarde, fermeture de FL Studio et restauration ;
6. clonage de channel et duplication d'instance ;
7. plusieurs instances avec samples différents ;
8. automation de paramètres globaux ;
9. lecture, pause, boucle, seek et changement de BPM ;
10. export vers playlist ;
11. rendu du projet FL Studio hors ligne ;
12. modification ou disparition du fichier source.

## Matrice audio

### Fréquences du host

- 44,1 kHz ;
- 48 kHz ;
- 88,2/96 kHz ;
- 192 kHz au moins comme test de robustesse.

### Tailles de bloc

- 32 ;
- 64 ;
- 128 ;
- 256 ;
- 512 ;
- 1024 ;
- 2048 ;
- tailles non puissance de deux si l'hôte de test les permet.

### Fichiers

- mono/stéréo ;
- 16/24/32-bit integer ;
- 32-bit float ;
- fichiers très courts ;
- fichiers silencieux ;
- impulsions aux frontières ;
- fréquences source différentes du host ;
- nom Unicode et chemin long ;
- fichier corrompu/tronqué ;
- taille proche de la limite ;
- nombre de canaux non pris en charge.

Les fichiers de test doivent être créés par le projet ou provenir de sources dont la redistribution est autorisée.

## Tests de restauration

Conserver un dossier de fixtures d'états :

```text
Tests/fixtures/state/v1/
Tests/fixtures/state/v2/
```

Pour chaque version publiée, garder au moins :

- état par défaut ;
- état 4 et 32 slices ;
- état avec ordre aléatoire et verrous ;
- état utilisant toutes les bornes de paramètres ;
- état avec sample manquant simulé.

## Qualité sonore

### Tests numériques

- silence reste silence ;
- gain unitaire ne change pas une lecture non transposée hors fades ;
- reverse appliqué deux fois retrouve la séquence attendue ;
- absence de discontinuité excessive aux fades ;
- rendu déterministe avec même graine et paramètres.

### Tests d'écoute

Utiliser un petit corpus : voix, boucle de batterie, pad harmonique, basse, piano et signal percussif. Comparer à niveau égal :

- interpolation ;
- longueurs de fades ;
- algorithmes de time-stretch ;
- formants activés/désactivés ;
- tempos extrêmes documentés.

Les résultats subjectifs sont notés avec la version, les réglages et le casque/moniteurs utilisés.

## Performance

Mesurer en `RelWithDebInfo` :

- CPU moyen et pic ;
- temps maximal de `processBlock()` ;
- allocations dans le callback ;
- mémoire par instance ;
- chargement du sample ;
- construction waveform ;
- rendu offline ;
- 1, 4, 8 et 16 instances simultanées.

Un test de stress change les paramètres, redimensionne l'UI et randomise pendant la lecture.

## Gravité des bugs

- **Bloquant** : crash du host, corruption de projet, perte de données, installation dangereuse.
- **Critique** : silence inattendu, restauration incorrecte, désynchronisation majeure, export faux.
- **Majeur** : fonction centrale inutilisable avec contournement difficile.
- **Mineur** : défaut local avec contournement simple.
- **Cosmétique** : apparence ou texte sans impact fonctionnel.

Aucune release publique ne contient de bug bloquant ou critique connu.

## Rapport de bug minimal

- version SauceChop et configuration Debug/Release ;
- version Windows et FL Studio ;
- fréquence, taille de buffer et pilote audio ;
- étapes exactes ;
- résultat observé et attendu ;
- fichier/projet minimal si redistribuable ;
- logs et dump si disponibles ;
- reproductibilité sur Standalone ou pluginval.
