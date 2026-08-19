# Vision produit

## Résumé

SauceChop transforme rapidement un fichier audio en une nouvelle phrase musicale. L'utilisateur importe un sample, choisit un nombre de slices, réorganise et modifie celles-ci, puis joue le résultat en synchronisation avec son projet ou l'exporte vers le DAW.

## Public principal

- producteurs utilisant FL Studio sous Windows ;
- beatmakers travaillant avec des mélodies, voix, boucles et breaks ;
- utilisateurs qui souhaitent obtenir rapidement des variations sans éditer manuellement chaque fragment dans la playlist.

## Problème résolu

Le découpage manuel d'une boucle implique plusieurs opérations dispersées : découper, réordonner, ajuster les clics, transposer, inverser, écouter les variantes et consolider le résultat. SauceChop réunit ce flux dans une interface unique, immédiate et reproductible.

## Promesse

Passer d'un sample brut à une variation exploitable en moins d'une minute, sans quitter le plug-in.

## Principes produit

1. **Résultat immédiat** : le premier son doit être obtenu après un simple glisser-déposer.
2. **Hasard contrôlable** : toute randomisation respecte les verrous et peut être annulée.
3. **Aucune surprise à la réouverture** : une session restaurée doit sonner comme au moment de sa sauvegarde.
4. **Manipulation directe** : les slices sont sélectionnées et déplacées directement dans la forme d'onde ou la séquence.
5. **Stabilité avant sophistication** : un pitch simple et fiable précède le time-stretch avancé.
6. **Identité originale** : nom final, interface, textes, sons et graphismes doivent être propres au projet.

## Différenciation envisagée

La première version ne cherche pas à battre tous les produits existants. Les pistes de différenciation seront évaluées après le MVP :

- randomisation reproductible avec historique et graine visible ;
- probabilités par slice ;
- plusieurs motifs A/B ;
- détection de transitoires assistée mais toujours éditable ;
- export simultané de l'audio et du MIDI ;
- règles de randomisation musicales plutôt qu'un simple mélange uniforme.

Ces pistes sont hors périmètre tant que les fonctions essentielles ne sont pas stables.

## Périmètre des versions

### Prototype

- VST3 chargeable dans FL Studio ;
- chargement et lecture d'un WAV ;
- forme d'onde minimale ;
- découpage égal en 4/8/16/32 ;
- lecture de la séquence.

### MVP

- WAV, AIFF et MP3 stéréo ou mono ;
- sélection et réorganisation ;
- gain, pitch par vitesse, reverse, fades ;
- lecture MIDI en mode One Shot et Gate ;
- randomisation, reset et verrous de position ;
- undo/redo pour les actions d'édition ;
- sauvegarde/restauration fiable de l'état.

### Bêta

- synchronisation au tempo et au transport ;
- vitesse moitié/double ;
- verrouillage des effets ;
- rendu WAV hors ligne et glisser-déposer vers le DAW ;
- préférences et gestion des erreurs utilisateur.

### Version 1.0

- pitch/time-stretch de qualité avec conservation optionnelle des formants ;
- presets ;
- installateur/désinstallateur ;
- documentation utilisateur ;
- télémétrie absente par défaut, sauf décision explicite ultérieure ;
- campagne de compatibilité et de performance terminée.

## Hors périmètre initial

- macOS, Audio Unit, AAX et Linux ;
- format natif FL Studio ;
- éditeur spectral ;
- enregistrement audio dans le plug-in ;
- marketplace de samples ;
- synchronisation cloud ;
- compte utilisateur, activation en ligne ou DRM ;
- application autonome.

## Indicateurs de réussite

- aucun crash pendant une session de test de deux heures ;
- restauration exacte sur 20 projets de référence ;
- aucune allocation mémoire répétée dans le callback audio en régime stable ;
- démarrage du son en moins d'un bloc audio après un événement MIDI ;
- utilisation intuitive du flux principal sans consulter le manuel ;
- rendu exporté cohérent avec ce qui est entendu dans le plug-in.
