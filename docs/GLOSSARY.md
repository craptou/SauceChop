# Glossaire et conventions

## Audio

- **Sample** : fichier audio source complet. Dans le code, préférer `sourceSample` pour éviter la confusion avec un échantillon numérique individuel.
- **Audio sample** : valeur numérique d'un canal à un instant précis.
- **Slice** : vue logique sur une plage du sample source.
- **Sequence** : ordre dans lequel les slices sont jouées.
- **Voice** : instance de lecture active d'une slice, notamment après un événement MIDI.
- **Pitch shift** : modification de hauteur.
- **Time-stretch** : modification de durée sans modification nécessaire de hauteur.
- **Formant** : zone de résonance caractéristique, importante notamment pour les voix.
- **Declick** : fade court appliqué aux frontières afin de réduire les discontinuités.
- **Transport** : état Play/Stop et position temporelle fournis par le DAW.
- **PPQ** : position musicale exprimée en noires.
- **Offline render** : génération d'un fichier sans dépendre de la lecture temps réel.

## MIDI

Les numéros MIDI sont la référence interne. La note 60 est stockée comme `60`, même si différentes applications l'affichent comme C3, C4 ou C5.

## Nommage

- code, noms de fichiers sources et identifiants : anglais ;
- documentation de développement : français ;
- textes de l'interface : anglais dans un premier temps, centralisés pour permettre une traduction ;
- unités explicites dans les noms : `sampleRateHz`, `fadeMs`, `positionPpq`, `startSample` ;
- demi-tons : `semitones` ; facteurs multiplicatifs : `ratio`.

## Versionnement

- version produit : SemVer `MAJOR.MINOR.PATCH` ;
- version d'état sérialisé : entier indépendant, par exemple `stateSchemaVersion = 2` ;
- version de format de preset : entier indépendant si les presets divergent ultérieurement de l'état de session.
