# GUIDE DE DEBUG - URBAN THUNDER

## 🔧 Version actuelle : STABLE BASE

La ROM compile et fonctionne avec affichage de texte basique seulement.

## 📋 Marqueurs de code commenté

Pour réactiver progressivement les fonctionnalités, cherchez ces marqueurs dans `main.c` :

### Étape 1 - Initialisation
- `DEBUG_DISABLE_SPR_INIT` - Initialisation système de sprites
- `DEBUG_DISABLE_PALETTES` - Configuration des palettes de couleurs
- `DEBUG_DISABLE_TILESETS` - Chargement des tilesets graphiques

### Étape 2 - Interface basique  
- `DEBUG_DISABLE_UI` - Interface utilisateur et textes
- `DEBUG_DISABLE_BACKGROUND` - Fond d'écran statique

### Étape 3 - Sprites et objets
- `DEBUG_DISABLE_SPRITES` - Sprite du joueur
- `DEBUG_DISABLE_SPRITES_UPDATE` - Mise à jour des sprites

### Étape 4 - Logique de jeu
- `DEBUG_DISABLE_INPUT` - Gestion des contrôles
- `DEBUG_DISABLE_GAMELOGIC` - Logique de jeu principale

### Étape 5 - Systèmes avancés
- `DEBUG_DISABLE_LOOKUPS` - Tables de lookup pour performance
- `DEBUG_DISABLE_AI` - Système d'intelligence artificielle

### Étape 6 - Rendu 3D
- `DEBUG_DISABLE_ROADRENDER` - Moteur de rendu pseudo-3D de route
- `DEBUG_DISABLE_UI_RENDER` - Interface utilisateur dynamique

## 🚀 Instructions de test

1. **Pour réactiver une fonctionnalité** :
   - Cherchez le marqueur `DEBUG_DISABLE_XXX`
   - Supprimez les `/*` et `*/` autour du code
   - Recompilez avec `make build`
   - Testez dans BlastEm

2. **Si erreur ADDRESS ERROR** :
   - Remettez immédiatement en commentaire la section problématique
   - Le problème vient de cette section spécifique

3. **Ordre recommandé de test** :
   ```
   SPR_INIT → PALETTES → UI → SPRITES → INPUT → GAMELOGIC
   ```

## 📝 Notes techniques

- **Base stable** : VDP_setScreenWidth320() + VDP_setPlaneSize() + VDP_clearPlane()
- **Problème identifié** : ADDRESS ERROR vient des systèmes complexes
- **Cause probable** : Accès mémoire mal aligné ou pointeurs invalides dans assembleur

## 🎯 Objectif

Identifier précisément quelle fonction cause l'ADDRESS ERROR en réactivant le code étape par étape.
