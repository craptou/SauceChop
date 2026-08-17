# Conception du moteur audio

## Responsabilité

Le moteur transforme trois entrées en audio :

- un `SourceSample` immuable ;
- un snapshot de slices et de séquence ;
- les événements MIDI et informations de transport du bloc courant.

Il doit produire le même résultat logique en temps réel et lors d'un rendu hors ligne.

## Découpage égal

Pour `N` slices et `L` frames :

```text
start(i) = floor(i * L / N)
end(i)   = floor((i + 1) * L / N)
```

Cette formulation distribue les arrondis et garantit que la première slice commence à 0 et que la dernière termine à `L`.

Chaque frontière est exprimée dans la fréquence d'échantillonnage originale du fichier. Le lecteur convertit ensuite vers la fréquence du host.

## Position de lecture

Une voix contient au minimum :

- identifiant de slice ;
- position source fractionnaire ;
- incrément source par sample de sortie ;
- direction `+1` ou `-1` ;
- gain courant et cible ;
- phase de fade ;
- état One Shot/Gate ;
- âge pour le voice stealing.

Pour le pitch MVP :

```text
pitchRatio = 2^(semitones / 12)
sampleRateRatio = sourceSampleRate / hostSampleRate
sourceIncrement = pitchRatio * sampleRateRatio
```

En reverse, la voix démarre juste avant `sourceEndSample` et soustrait l'incrément.

## Interpolation

Progression proposée :

1. interpolation linéaire pour obtenir le prototype ;
2. interpolation cubique ou Lagrange pour le MVP si le coût reste acceptable ;
3. comparaison perceptuelle avec un resampler de meilleure qualité avant 1.0.

Les lectures proches des frontières utilisent des accès bornés ; aucune voix ne lit en dehors du buffer source.

## Fades

Les fades sont appliqués dans le domaine de sortie. Leur durée en samples est recalculée depuis les millisecondes et la fréquence du host.

Règles :

- valeurs négatives interdites ;
- somme fade-in + fade-out limitée à la durée de lecture prévue ;
- courbe linéaire dans le prototype ;
- courbe equal-power à évaluer par écoute avant le MVP ;
- un lissage court s'applique aussi aux changements de gain en cours de voix.

## Séquence interne

Le mode libre possède une horloge en samples. À la fin d'une slice, le séquenceur démarre la suivante selon `sequenceOrder`. Comme le pitch simple modifie la durée, la séquence libre suit la durée réellement jouée.

En mode synchronisé, les frontières musicales proviennent du PPQ et non de la fin effective d'une voix. Chaque pas de séquence démarre à la position musicale prévue ; une voix précédente peut être coupée ou chevaucher selon une politique à trancher avant la Bêta.

## Transport DAW

Au début de chaque bloc :

1. lire les informations de position du `AudioPlayHead` ;
2. détecter Play/Stop et les discontinuités PPQ ;
3. convertir la plage du bloc en pas de séquence ;
4. planifier les déclenchements avec leur offset précis dans le bloc ;
5. réinitialiser proprement les voix lors d'un saut non continu.

Le traitement ne suppose jamais que le BPM est constant entre deux blocs.

## MIDI

Les événements du `MidiBuffer` doivent être traités à leur offset dans le bloc, pas seulement au début du bloc.

Mapping initial :

```text
sliceIndex = midiNoteNumber - midiBaseNote
```

Un index hors plage est ignoré. Les Note Off sont associés à une note et à une voix ; plusieurs Note On de même hauteur doivent rester gérables.

## Polyphonie et voice stealing

Ordre proposé pour voler une voix :

1. voix déjà en relâchement ;
2. voix au gain instantané le plus faible ;
3. voix la plus ancienne.

Le vol applique un fade très court afin d'éviter une discontinuité.

## Randomisation

La randomisation utilise un générateur déterministe initialisé avec `randomSeed`. Pour les positions :

1. collecter les indices de destination non verrouillés ;
2. collecter les slices occupant ces destinations ;
3. appliquer Fisher-Yates uniquement à cette collection ;
4. replacer les valeurs dans les destinations libres ;
5. stocker la nouvelle graine ou l'état suffisant pour reproduire le résultat.

Les tests vérifient que les positions verrouillées ne changent jamais et que la sortie forme une permutation valide.

## Time-stretch et formants

Le moteur avancé sera caché derrière une interface, par exemple :

```cpp
class TimeStretchEngine
{
public:
    virtual void prepare(const StretchConfig&) = 0;
    virtual void reset() noexcept = 0;
    virtual void render(const StretchRequest&, AudioView output) = 0;
    virtual int latencySamples() const noexcept = 0;
};
```

Deux usages sont distincts :

- **pré-calcul par slice** : plus simple et stable, mais consomme de la mémoire et réagit moins vite aux changements ;
- **traitement temps réel** : plus flexible, mais complexe, coûteux et sensible à la latence.

La stratégie initiale envisagée pour la Bêta est un cache préparé en arrière-plan pour la synchronisation ; la version 1.0 pourra adopter un chemin temps réel si les tests le justifient.

## Rendu hors ligne

`OfflineRenderer` reçoit un snapshot gelé. Il ne lit jamais directement l'état UI pendant le rendu.

Étapes :

1. valider le snapshot et la destination temporaire ;
2. déterminer précisément la durée de sortie ;
3. préparer une instance indépendante du moteur ;
4. rendre par blocs ;
5. écrire un WAV temporaire puis le fermer ;
6. renommer atomiquement vers le nom exposé au glisser-déposer ;
7. notifier l'UI.

Un rendu annulé ou échoué supprime uniquement son fichier temporaire identifié, jamais le dossier complet.

## Budgets initiaux

- moins de 25 % d'un cœur CPU dans le scénario MVP de 16 voix à 48 kHz/512 samples sur la machine de référence ;
- aucun dropout lors d'une randomisation ou modification d'interface ;
- mémoire décodée bornée par l'exigence FR-071 ;
- temps de chargement visé inférieur à 500 ms pour un WAV stéréo de 30 secondes sur SSD, hors construction graphique différée ;
- déterminisme du rendu hors ligne à paramètres et architecture identiques.

Ces valeurs servent de garde-fous et seront ajustées après les premiers benchmarks.

