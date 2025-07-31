#!/usr/bin/env python3
"""
Générateur simple pour SGDK - Format ultra-basique
Crée un tileset minimal qui respecte strictement les contraintes SGDK
"""

from PIL import Image
import os

def create_minimal_sgdk_tileset(filename, tile_color_idx=1):
    """Crée un tileset minimal SGDK avec format ultra-strict"""
    
    width, height = 128, 32
    
    # Palette SGDK standard (16 couleurs exactes)
    palette_colors = [
        (0, 0, 0),        # 0 - Noir (transparent)
        (34, 34, 34),     # 1 - Gris très sombre  
        (68, 68, 68),     # 2 - Gris sombre
        (102, 102, 102),  # 3 - Gris moyen
        (136, 136, 136),  # 4 - Gris clair
        (170, 170, 170),  # 5 - Gris très clair
        (255, 255, 255),  # 6 - Blanc
        (255, 255, 0),    # 7 - Jaune
        (0, 170, 0),      # 8 - Vert foncé
        (0, 255, 0),      # 9 - Vert
        (136, 255, 136),  # 10 - Vert clair
        (0, 136, 255),    # 11 - Bleu
        (255, 0, 0),      # 12 - Rouge
        (255, 136, 0),    # 13 - Orange
        (204, 204, 204),  # 14 - Gris clair 2
        (221, 221, 221)   # 15 - Gris blanc
    ]
    
    # Créer image entièrement noire
    img = Image.new('RGB', (width, height), (0, 0, 0))
    pixels = img.load()
    
    # 1. PALETTE obligatoire dans la première ligne
    for i in range(16):
        pixels[i, 0] = palette_colors[i]
    
    # 2. Reste de la première ligne = couleur 0 (NOIR)
    for x in range(16, width):
        pixels[x, 0] = (0, 0, 0)
    
    # 3. TILE [0,0] entièrement couleur 0 (16x16 en haut à gauche)
    for y in range(1, 16):
        for x in range(16):
            pixels[x, y] = (0, 0, 0)
    
    # 4. Pattern simple à partir du tile [1,0] (x=16)
    tile_color = palette_colors[tile_color_idx]
    
    for y in range(height):
        for x in range(16, width):  # Commence à x=16
            if y == 0:
                continue  # Garder la ligne de palette
                
            # Motif simple : alternance selon le type
            if filename == "road_tiles.png":
                # Route : ligne centrale + asphalte
                if 23 <= x <= 24:  # Ligne centrale (relative au tile)
                    pixels[x, y] = palette_colors[7]  # Jaune
                elif (x + y) % 4 == 0:
                    pixels[x, y] = palette_colors[2]  # Gris sombre
                else:
                    pixels[x, y] = palette_colors[3]  # Gris moyen
                    
            elif filename == "grass_tiles.png":
                # Herbe : vert avec variations
                if (x % 3 == 0) and (y % 4 == 1):
                    pixels[x, y] = palette_colors[9]  # Vert vif
                else:
                    pixels[x, y] = palette_colors[8]  # Vert foncé
                    
            elif filename == "sky_tiles.png":
                # Ciel : bleu uniforme
                pixels[x, y] = palette_colors[11]  # Bleu
    
    # Sauvegarder
    output_path = f"../res/{filename}"
    img.save(output_path, 'PNG')
    print(f"✓ Créé: {output_path}")

def main():
    print("🏍️ Générateur SGDK ultra-simple")
    print("=" * 40)
    
    # Créer les 3 tilesets
    create_minimal_sgdk_tileset("road_tiles.png")
    create_minimal_sgdk_tileset("grass_tiles.png") 
    create_minimal_sgdk_tileset("sky_tiles.png")
    
    print("\n✅ Tiles ultra-simples générés!")
    print("Format: 128x32, palette ligne 0, tile [0,0] noir uni")

if __name__ == "__main__":
    main()
