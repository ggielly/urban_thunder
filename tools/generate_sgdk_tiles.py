#!/usr/bin/env python3
"""
Générateur de tiles SGDK pour Urban Thunder
Crée des fichiers PNG compatibles avec ResComp de SGDK
Format: 128x32 minimum avec tile [0,0] uni et palette intégrée
"""

from PIL import Image
import os

def create_sgdk_tileset(width, height, palette, tile_pattern, filename):
    """Crée un tileset au format SGDK RGB"""
    
    # Créer l'image
    img = Image.new('RGB', (width, height), palette[0])
    pixels = img.load()
    
    # ÉTAPE 1: Palette dans les 16 premiers pixels de la première ligne
    for i in range(16):
        if i < len(palette):
            color = palette[i]
            # Convertir hex en RGB
            r = int(color[1:3], 16)
            g = int(color[3:5], 16) 
            b = int(color[5:7], 16)
            pixels[i, 0] = (r, g, b)
        else:
            pixels[i, 0] = (0, 0, 0)  # Noir par défaut
    
    # ÉTAPE 2: Reste de la première ligne = couleur 0 (plain tile requirement)
    color0_rgb = (0, 0, 0)  # Couleur 0 = noir
    for x in range(16, width):
        pixels[x, 0] = color0_rgb
    
    # ÉTAPE 3: Tile [0,0] entièrement couleur 0 (16x16 en haut à gauche)
    for y in range(1, 16):
        for x in range(16):
            pixels[x, y] = color0_rgb
    
    # ÉTAPE 4: Générer les motifs à partir de x=16
    tile_size = 16
    
    for tile_y in range(0, height, tile_size):
        for tile_x in range(tile_size, width, tile_size):  # Commence à x=16
            
            # Créer le motif du tile
            for py in range(tile_size):
                for px in range(tile_size):
                    world_x = tile_x + px
                    world_y = tile_y + py
                    
                    # Éviter la ligne de palette
                    if world_y == 0:
                        continue
                        
                    if world_x < width and world_y < height:
                        color_idx = tile_pattern(px, py)
                        if color_idx < len(palette):
                            color = palette[color_idx]
                            r = int(color[1:3], 16)
                            g = int(color[3:5], 16)
                            b = int(color[5:7], 16)
                            pixels[world_x, world_y] = (r, g, b)
    
    # Sauvegarder
    output_path = f"../res/{filename}"
    img.save(output_path, 'PNG')
    print(f"✓ Créé: {output_path}")

def road_tile_pattern(x, y):
    """Motif de route avec ligne centrale jaune"""
    if 7 <= x <= 8:  # Ligne centrale
        return 7  # Jaune
    elif (x + y) % 8 == 0:  # Texture asphalte
        return 2  # Gris sombre
    else:
        return 3  # Gris moyen (fond)

def grass_tile_pattern(x, y):
    """Motif d'herbe"""
    if (x % 4 == 2) and (y % 3 == 1):  # Brins d'herbe
        return 6  # Vert vif
    elif (x + y) % 7 == 0:  # Pixels clairs
        return 7  # Vert clair
    else:
        return 4  # Vert moyen (fond)

def sky_tile_pattern(x, y):
    """Motif de ciel avec dégradé"""
    if y < 4 and (x + y) % 6 == 0:  # Nuages
        return 11  # Blanc
    else:
        # Dégradé bleu selon Y
        intensity = min(y // 4 + 4, 9)
        return intensity

def main():
    print("🏍️ Générateur de tiles SGDK pour Urban Thunder")
    print("=" * 50)
    
    # Palettes
    road_palette = [
        '#000000', '#222222', '#444444', '#666666',  # 0-3: Gris
        '#888888', '#AAAAAA', '#CCCCCC', '#FFFF00',  # 4-7: Gris + Jaune
        '#00AA00', '#00FF00', '#88FF88', '#0088FF',  # 8-11: Verts + Bleu
        '#FF0000', '#FF8800', '#FFFFFF', '#DDDDDD'   # 12-15: Rouge/Orange/Blanc
    ]
    
    grass_palette = [
        '#000000', '#004400', '#006600', '#008800',  # 0-3: Noirs/Verts sombres
        '#00AA00', '#00CC00', '#00FF00', '#44FF44',  # 4-7: Verts moyens
        '#88FF88', '#AAFFAA', '#CCFFCC', '#FFFFFF',  # 8-11: Verts clairs
        '#FFFF00', '#DDDD00', '#BBBB00', '#999900'   # 12-15: Jaunes
    ]
    
    sky_palette = [
        '#000000', '#001144', '#002266', '#003388',  # 0-3: Noirs/Bleus sombres
        '#0044AA', '#0055CC', '#0066FF', '#3388FF',  # 4-7: Bleus moyens
        '#66AAFF', '#99CCFF', '#CCDDFF', '#FFFFFF',  # 8-11: Bleus clairs/Blanc
        '#EEEEEE', '#DDDDDD', '#CCCCCC', '#BBBBBB'   # 12-15: Gris
    ]
    
    # Générer les tilesets
    create_sgdk_tileset(128, 32, road_palette, road_tile_pattern, "road_tiles.png")
    create_sgdk_tileset(128, 32, grass_palette, grass_tile_pattern, "grass_tiles.png") 
    create_sgdk_tileset(128, 32, sky_palette, sky_tile_pattern, "sky_tiles.png")
    
    print("\n✅ Tous les tiles SGDK ont été générés!")
    print("Relancez 'make' pour compiler.")

if __name__ == "__main__":
    main()
