# Interface et expérience utilisateur

## Structure principale

```text
┌────────────────────────────────────────────────────────────┐
│ SauceChop  fichier.wav       120 BPM?     ⚙              │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  Forme d'onde + limites + sélection + tête de lecture      │
│                                                            │
├────────────────────────────────────────────────────────────┤
│ [01][02][03][04][05][06][07][08] ...  ordre des slices    │
├────────────────────────────────────────────────────────────┤
│ Slice 04   Gain  Pitch  Reverse  Fade In/Out  Locks        │
├────────────────────────────────────────────────────────────┤
│ 4  8  16  32 | Reset | Randomize | Undo | MIDI | Sync     │
│                         ◀  ■  ▶              Drag audio ↗   │
└────────────────────────────────────────────────────────────┘
```

Le wireframe exprime la hiérarchie, pas le style graphique final.

## Flux principal

1. À l'ouverture, une grande zone invite à déposer un fichier.
2. Après import, le sample est automatiquement découpé en 16 slices.
3. Cliquer une slice la sélectionne et la préécoute.
4. La faire glisser dans la bande de séquence change son ordre.
5. L'inspecteur modifie la ou les slices sélectionnées.
6. Randomize crée une variante ; Undo revient immédiatement en arrière.
7. En Bêta, « Drag audio » rend et expose la variante au DAW.

## États visuels des slices

Les états ne reposent jamais uniquement sur la couleur :

| État | Traitement visuel proposé |
| --- | --- |
| normale | fond neutre + numéro |
| sélectionnée | contour accentué + poignée/repère |
| en lecture | indicateur animé ou tête de lecture |
| position verrouillée | icône cadenas de position |
| effets verrouillés | icône cadenas d'effet |
| deux verrous | deux icônes, jamais une couleur ambiguë |
| silencieuse | symbole mute ou gain `-inf` |
| erreur | hachures + message associé |

## Comportements

### Dépôt d'un fichier

- toute la zone waveform accepte le dépôt ;
- un survol valide affiche clairement « Drop sample » ;
- remplacer un sample demande confirmation seulement si cette opération ne peut pas être annulée ; normalement, l'historique suffit ;
- pendant le décodage, la dernière version valide continue de fonctionner jusqu'à publication du nouveau sample.

### Sélection

- clic : une slice ;
- `Shift` + clic : plage ou sélection multiple, décision à confirmer lors du prototype ;
- `Ctrl+A` ou `Shift` appliqué depuis l'inspecteur : toutes les slices ;
- `Escape` : annuler l'interaction ou fermer un message non critique.

### Réorganisation

Le déplacement montre l'emplacement cible avant le relâchement. Une position verrouillée ne peut être remplacée par une randomisation, mais un déplacement manuel explicite peut soit être interdit, soit demander le déverrouillage ; cette règle reste à trancher.

### Paramètres

- double-clic : valeur par défaut ;
- clic droit : saisie numérique et éventuel menu d'automatisation fourni par le host ;
- molette avec modificateur fin ;
- affichage permanent de l'unité ;
- toutes les actions sont accessibles sans pointer un élément de quelques pixels seulement.

## Redimensionnement

Taille initiale proposée : `1000 × 650 px` logiques.

- minimum : `760 × 500` ;
- ratio non imposé ;
- waveform et séquence prennent l'espace supplémentaire ;
- inspecteur peut devenir une colonne latérale au-dessus d'une largeur à définir ;
- prise en charge des échelles Windows 100 %, 125 %, 150 % et 200 %.

## Accessibilité

- contraste texte/fond suffisant ;
- verrous distingués par icône et libellé ;
- focus clavier visible ;
- infobulles courtes ;
- zones cliquables d'au moins 28 px logiques, avec 36 px visés ;
- animations désactivables si elles sont ajoutées ;
- aucune animation indispensable à la compréhension de l'état.

## Messages importants

Exemples de formulations :

- `Unsupported audio format. Use WAV or AIFF.`
- `The source sample cannot be found. Relocate it to restore playback.`
- `This file is too large to load safely.`
- `Audio render failed. Your current pattern was not changed.`

Une erreur n'efface jamais la forme d'onde ou l'état précédent avant que le remplacement ait réussi.

## Identité visuelle

Le design final doit être original. À éviter :

- reprise du nom ou du logo d'un concurrent ;
- copie exacte de la disposition, des couleurs, icônes ou illustrations d'un produit de référence ;
- ressources graphiques dont la licence n'est pas documentée.

Avant la phase de polish, créer un mini design system avec palette, typographie, grille, rayons, ombres, icônes et états de composants.
