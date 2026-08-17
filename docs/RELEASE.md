# Processus de livraison

## Versionnement

Utiliser SemVer :

- `0.x.y` : développement, format et comportement encore évolutifs ;
- `1.0.0` : première version publique stable ;
- `PATCH` : correctif compatible ;
- `MINOR` : fonction compatible ;
- `MAJOR` : rupture nécessitant une communication/migration importante.

La version du schéma d'état est indépendante et n'est jamais déduite automatiquement de SemVer.

## Canaux

- **dev** : builds locales non distribuées ;
- **alpha** : équipe et testeurs très proches, projets non critiques ;
- **beta** : groupe externe limité, télémétrie uniquement si décidée et consentie ;
- **stable** : distribution générale.

## Contenu du paquet Windows

```text
SauceChop-1.0.0-Windows-x64/
  SauceChop.vst3
  README.txt
  LICENSE.txt ou EULA.txt
  THIRD_PARTY_NOTICES.txt
  CHANGELOG.txt
```

Un installateur pourra placer le VST3 dans le chemin système standard. Il doit désinstaller uniquement les fichiers qu'il a installés.

## Checklist technique

- [ ] build propre depuis un clone neuf ;
- [ ] dépendances fixées à des versions auditées ;
- [ ] configuration Release x64 ;
- [ ] pluginval réussi au niveau retenu ;
- [ ] matrice `TESTING.md` exécutée ;
- [ ] aucun bug bloquant ou critique ouvert ;
- [ ] restauration vérifiée depuis toutes les versions publiques supportées ;
- [ ] projets FL Studio avec plusieurs instances validés ;
- [ ] rendu DAW et rendu SauceChop comparés ;
- [ ] CPU, mémoire et allocations mesurés ;
- [ ] crash dumps et symboles privés archivés ;
- [ ] checksum SHA-256 calculé sur l'installateur/archives ;
- [ ] signature de code appliquée si disponible.

## Checklist produit et légale

- [ ] nom final et identifiants confirmés ;
- [ ] droits sur logo, icônes, polices et presets documentés ;
- [ ] formule/licence JUCE compatible avec la distribution ;
- [ ] licences tierces et notices incluses ;
- [ ] aucun sample tiers non autorisé ;
- [ ] guide d'installation et configuration minimale publiés ;
- [ ] limites connues documentées ;
- [ ] politique de confidentialité publiée si une collecte de données existe ;
- [ ] prix, remboursement et support définis si le produit est vendu.

## Procédure

1. geler les nouvelles fonctions ;
2. mettre à jour version, schéma, changelog et documentation ;
3. compiler depuis un clone propre ;
4. exécuter tests automatiques et manuels ;
5. produire le paquet dans un environnement propre ;
6. vérifier installation, scan, lancement et désinstallation ;
7. signer et calculer les checksums ;
8. publier d'abord dans le canal prévu ;
9. conserver artefacts, symboles, rapports et commit exact ;
10. surveiller les rapports sans modifier rétroactivement le binaire publié.

## Rollback

Si une release corrompt l'état, fait planter le host ou produit un export faux :

1. retirer ou marquer immédiatement le téléchargement concerné ;
2. publier un avis factuel avec versions touchées ;
3. conserver les fichiers nécessaires à la reproduction ;
4. corriger sur une branche dédiée ;
5. ajouter un test de régression ;
6. publier un patch sans réutiliser le même numéro de version.

## Support minimal

Chaque rapport doit pouvoir être relié à : version exacte, OS, version FL Studio, fréquence/ buffer, type de fichier, étapes et projet minimal. Les limites de support des anciennes versions seront annoncées avant leur abandon.
