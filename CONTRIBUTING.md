# Contribuer à SauceChop

## Avant de coder

1. Relier la modification à une issue ou à un identifiant de `docs/BACKLOG.md`.
2. Lire les exigences `FR-*` concernées.
3. Ajouter une décision dans `docs/DECISIONS.md` si le changement modifie l'architecture, le format d'état, une dépendance ou le périmètre.
4. Pour le DSP temps réel, décrire explicitement allocations, synchronisation et durée de vie des objets.

## Branches

Convention proposée :

```text
main
feature/DSP-002-slice-voice
fix/HOST-006-state-restore
docs/architecture-snapshot-lifetime
chore/update-juce
```

`main` doit rester compilable. Les branches sont courtes et centrées sur un seul résultat.

## Commits

Format recommandé :

```text
type(scope): description impérative
```

Exemples :

```text
feat(dsp): add bounded slice playback
fix(state): preserve locked positions on reload
test(midi): cover sample-accurate note offsets
docs(build): document pluginval workflow
```

Types usuels : `feat`, `fix`, `test`, `docs`, `refactor`, `perf`, `build`, `chore`.

Éviter de mélanger mise à jour d'une dépendance, reformatage massif et changement fonctionnel dans le même commit.

## Pull requests

Même en travaillant seul, une pull request sur les changements importants aide à conserver une trace de décision.

Elle indique :

- problème et résultat ;
- issue/exigence liée ;
- approche choisie et compromis ;
- tests exécutés ;
- impact temps réel ;
- captures avant/après pour l'UI ;
- changement de schéma et migration, le cas échéant ;
- dépendances ou licences ajoutées.

## Style C++

- C++20 ;
- RAII et types propriétaires explicites ;
- `noexcept` lorsque la garantie est réelle et utile au chemin audio ;
- pas de pointeur brut propriétaire ;
- unités présentes dans les noms ;
- pas de constantes magiques DSP ;
- bornes et invariants vérifiés en Debug ;
- commentaires expliquant le « pourquoi », particulièrement autour du temps réel.

Le format exact sera automatisé avec `.clang-format` pendant M0.

## Règles temps réel

Tout changement dans le callback audio doit répondre à ces questions :

- peut-il allouer ou libérer ?
- peut-il prendre un verrou ?
- peut-il appeler le système de fichiers ou logger de façon bloquante ?
- sa complexité dépend-elle sans borne d'une donnée utilisateur ?
- le host peut-il appeler cette méthode dans un ordre inattendu ?
- le résultat reste-t-il valide à toutes les fréquences et tailles de blocs supportées ?

Si une réponse est incertaine, la modification n'est pas terminée.

## Définition de terminé

Une tâche est terminée lorsque :

- critères d'acceptation satisfaits ;
- tests pertinents ajoutés et exécutés ;
- aucun nouveau warning non expliqué ;
- état sauvegardé/migré si nécessaire ;
- documentation mise à jour ;
- licences vérifiées pour toute ressource ou dépendance ;
- aucun fichier généré ou secret ajouté au dépôt ;
- test rapide dans Standalone et/ou FL Studio selon le changement.

## Ressources et propriété intellectuelle

N'ajouter que du code, des samples, images, polices et icônes créés pour le projet ou accompagnés d'une licence compatible et documentée. Ne pas copier les ressources, presets, textes ou éléments distinctifs d'un plug-in de référence.

## Sécurité

Ne jamais committer de certificat de signature, clé privée, jeton, mot de passe ou identifiant de service. Les fichiers audio importés par les utilisateurs sont locaux et ne doivent pas être envoyés sur un réseau sans fonctionnalité explicite et consentement clair.
