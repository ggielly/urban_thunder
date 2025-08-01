# SOLUTION FINALE - Problème d'artéfacts VRAM

## Problème identifié

Les artéfacts VRAM venaient de **deux causes principales** :

### 1. Index VRAM qui se chevauchaient

**INCORRECT :** Index trop proches causant chevauchement en mémoire VRAM
```c
// ❌ Ces index causaient des chevauchements
VDP_loadTileSet(road_tiles.tileset, TILE_USER_INDEX, DMA);
VDP_loadTileSet(grass_tiles.tileset, TILE_USER_INDEX + 8, DMA);   // Trop proche !
VDP_loadTileSet(sky_tiles.tileset, TILE_USER_INDEX + 16, DMA);    // Trop proche !
```

**CORRECT :** Index suffisamment espacés
```c
// ✅ Index correctement espacés
VDP_loadTileSet(road_tiles.tileset, TILE_USER_INDEX, CPU);
VDP_loadTileSet(grass_tiles.tileset, TILE_USER_INDEX + 32, CPU);  // Espace suffisant
VDP_loadTileSet(sky_tiles.tileset, TILE_USER_INDEX + 64, CPU);    // Espace suffisant
```

### 2. SPR_init() incompatible

**Problème :** `SPR_init()` cause un écran noir systématiquement dans cette configuration
**Solution :** Ne pas utiliser le système de sprites SGDK

## Méthode de chargement correcte

### Configuration finale validée :

```c
int main() {
    VDP_init();
    VDP_setScreenWidth320();
    
    // PAS de SPR_init() !
    
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    
    // Chargement des palettes
    PAL_setPalette(PAL0, main_palette.data, DMA);
    PAL_setPalette(PAL1, simple_palette.data, DMA);
    
    // Chargement des tilesets avec index espacés et CPU
    if (road_tiles.tileset) {
        VDP_loadTileSet(road_tiles.tileset, TILE_USER_INDEX, CPU);
    }
    if (grass_tiles.tileset) {
        VDP_loadTileSet(grass_tiles.tileset, TILE_USER_INDEX + 32, CPU);
    }
    if (sky_tiles.tileset) {
        VDP_loadTileSet(sky_tiles.tileset, TILE_USER_INDEX + 64, CPU);
    }
    
    while(TRUE) {
        SYS_doVBlankProcess();
    }
}
```

## Points clés de la solution

- ✅ **Index VRAM espacés** : +32, +64 au lieu de +8, +16
- ✅ **CPU au lieu de DMA** : Plus stable pour le chargement de tilesets
- ❌ **Pas de SPR_init()** : Cause écran noir dans cette configuration
- ✅ **VDP_loadTileSet()** : Fonctionne parfaitement avec les bons paramètres

## Alternative pour les sprites

Utiliser des tiles animées avec `VDP_setTileMapXY()` pour simuler des sprites :

```c
// Simuler un sprite avec une tile
VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, 0, 0, TILE_USER_INDEX + 96), playerX/8, playerY/8);
```

## Tests validés

1. ✅ Palettes multiples fonctionnent
2. ✅ Tilesets multiples fonctionnent  
3. ✅ Plans BG_A et BG_B utilisables
4. ❌ Système de sprites SGDK incompatible
5. ✅ Configuration stable pour développement de jeu
