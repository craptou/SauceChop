# Architecture logicielle

## Vue d'ensemble

SauceChop est un instrument VST3. Le plug-in ne traite pas obligatoirement l'entrée audio du DAW : il produit de l'audio depuis un fichier chargé et des événements MIDI. Une future variante « effet » est hors périmètre.

```text
FL Studio
  ├─ transport, tempo, état VST et événements MIDI
  ▼
PluginProcessor
  ├─ StateStore ───────────────► ValueTree versionné
  ├─ CommandQueue ─────────────► changements préparés hors audio
  ├─ Sequencer ────────────────► événements de slices
  └─ AudioEngine
       ├─ SourceSample immuable
       ├─ SliceVoicePool
       ├─ Resampler/PitchEngine
       └─ TimeStretchEngine (1.0)

PluginEditor
  ├─ WaveformView
  ├─ SequenceView
  ├─ SliceInspector
  └─ TransportControls

BackgroundServices
  ├─ SampleLoader
  ├─ WaveformCacheBuilder
  └─ OfflineRenderer
```

## Modules envisagés

L'arborescence sera créée pendant le bootstrap du code :

```text
Source/
  PluginProcessor.*
  PluginEditor.*
  audio/
    AudioEngine.*
    SliceVoice.*
    SliceVoicePool.*
    PlaybackInterpolator.*
    TimeStretchEngine.*
  model/
    ProjectState.*
    SliceState.*
    StateSerializer.*
    StateMigration.*
  sequencing/
    Sequencer.*
    TransportMapper.*
    Randomizer.*
  files/
    SampleLoader.*
    SampleIdentity.*
    OfflineRenderer.*
    TemporaryFileStore.*
  ui/
    WaveformView.*
    SequenceView.*
    SliceInspector.*
    TransportBar.*
    Theme.*
  util/
    AudioMath.*
    LockFreeCommandQueue.*
Tests/
```

L'arborescence décrit les responsabilités, pas une obligation de créer immédiatement toutes les classes.

## Modèle de données

### SourceSample

Le sample chargé est traité comme immuable après publication au moteur audio.

```cpp
struct SourceSample
{
    juce::AudioBuffer<float> audio;
    double originalSampleRateHz = 0.0;
    std::int64_t frameCount = 0;
    juce::String displayName;
    SampleIdentity identity;
};
```

Une nouvelle importation construit un nouvel objet sur un thread de travail, puis remplace atomiquement la référence utilisée par le moteur.

### SliceState

```cpp
struct SliceState
{
    int id = 0;
    std::int64_t sourceStartSample = 0;
    std::int64_t sourceEndSample = 0; // borne exclusive
    float gainDb = 0.0f;
    float pitchSemitones = 0.0f;
    float fadeInMs = 3.0f;
    float fadeOutMs = 3.0f;
    bool reversed = false;
    bool preserveFormants = false;
    bool positionLocked = false;
    bool effectsLocked = false;
};
```

`id` identifie la slice source. Sa position dans `sequenceOrder` identifie sa position musicale. Ces deux notions ne doivent jamais être confondues.

### ProjectState

```cpp
struct ProjectState
{
    int schemaVersion = 1;
    std::vector<SliceState> slices;
    std::vector<int> sequenceOrder;
    int sliceCount = 16;
    int midiBaseNote = 60;
    int polyphony = 16;
    std::uint64_t randomSeed = 0;
    SampleIdentity sampleIdentity;
    PlaybackSettings playback;
};
```

La représentation sérialisée sera un `juce::ValueTree`. Le moteur audio utilise toutefois un snapshot compact et immuable, adapté à la lecture temps réel.

## Frontière temps réel

### Thread audio

Autorisé :

- lire des snapshots immuables ;
- parcourir des vecteurs préalloués ;
- interpoler et mixer des samples ;
- consommer une file SPSC bornée ;
- mettre à jour des atomiques simples ;
- écrire des mesures dans une file de télémétrie locale bornée.

Interdit :

- ouvrir, lire ou écrire un fichier ;
- décoder un format audio ;
- appeler l'interface graphique ;
- allouer/libérer en régime stable ;
- attendre un mutex, un thread ou une condition ;
- construire de gros `ValueTree` ou `String` ;
- rendre un fichier complet.

### Thread message/UI

Il gère les interactions, l'`UndoManager`, les mutations du modèle éditable et la publication de snapshots. Une mutation lourde est déléguée à un thread de travail.

### Threads de travail

Ils assurent le décodage audio, le calcul de forme d'onde, le hash et le rendu hors ligne. Leur résultat n'est publié que lorsqu'il est complet et valide.

## Publication d'état

Approche initiale, mise en œuvre pour le sample source depuis M3 :

1. l'UI modifie le `ValueTree` éditable ;
2. un `EngineSnapshot` complet est construit hors du callback audio ;
3. le propriétaire partagé reste côté message et un pointeur immuable est publié atomiquement ;
4. `processBlock()` adopte le nouveau snapshot au début d'un bloc ;
5. un pointeur de protection annoncé par le callback empêche la destruction d'un ancien sample encore utilisé ; les références retirées sont libérées côté message.

Un simple `shared_ptr` atomique n'est volontairement pas utilisé : il pourrait provoquer la destruction du dernier propriétaire sur le thread audio.

## Paramètres VST

`AudioProcessorValueTreeState` est utilisé pour les paramètres globaux stables :

- output gain ;
- global transpose ;
- slice count ;
- play mode ;
- MIDI base note ;
- sync/link ;
- speed multiplier.

Les paramètres individuels des slices sont stockés dans le modèle interne. Changer de 4 à 32 slices ne doit pas modifier dynamiquement la liste des paramètres exposés au DAW.

## Persistance du sample

Le MVP stocke une identité composée de :

- chemin canonique ;
- taille du fichier ;
- date de modification ;
- empreinte de contenu ;
- nom affiché ;
- fréquence et nombre de frames attendus.

À la restauration :

1. essayer le chemin exact ;
2. vérifier l'identité ;
3. essayer le cache géré par SauceChop si cette option est activée ;
4. demander une relocalisation ;
5. rester silencieux avec un état d'erreur visible si rien n'est trouvé.

L'intégration du fichier complet dans l'état VST est reportée : elle peut rendre les projets très volumineux et bloquer la sérialisation demandée par le DAW.

## Erreurs

Les couches basses retournent des erreurs structurées. L'UI transforme celles-ci en messages compréhensibles. Une erreur de fichier ne doit jamais devenir une exception non interceptée à travers la frontière du host.

Catégories initiales :

- unsupported format ;
- decode failed ;
- file missing or changed ;
- memory limit exceeded ;
- render failed ;
- temporary directory unavailable ;
- incompatible state version.

## Dépendances

- JUCE : infrastructure plug-in, audio, MIDI, UI, fichiers et état ;
- VST3 fourni via la chaîne JUCE ;
- Signalsmith Stretch envisagé pour la version 1.0 derrière une abstraction interne ;
- Catch2 ou le framework de tests JUCE : décision ouverte.

Toute nouvelle dépendance exige une vérification de licence, de maintenance, de taille binaire et de compatibilité temps réel.
