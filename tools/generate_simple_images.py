#!/usr/bin/env python3
"""
Générateur d'images PNG classiques pour SGDK
Format IMAGE au lieu de TILESET - plus simple
"""

from PIL import Image

def create_simple_image(filename, base_color, width=64, height=64):
    """Crée une image PNG simple"""
    
    img = Image.new('RGB', (width, height), base_color)
    pixels = img.load()
    
    # Ajouter un motif simple
    for y in range(height):
        for x in range(width):
            if filename == "road_tiles.png":
                # Route : ligne centrale jaune + asphalte gris
                if width//2 - 2 <= x <= width//2 + 1:
                    pixels[x, y] = (255, 255, 0)  # Jaune
                elif (x + y) % 4 == 0:
                    pixels[x, y] = (64, 64, 64)   # Gris sombre
                else:
                    pixels[x, y] = (96, 96, 96)   # Gris moyen
                    
            elif filename == "grass_tiles.png":
                # Herbe : vert avec variations
                if (x % 3 == 0) and (y % 4 == 1):
                    pixels[x, y] = (0, 255, 0)    # Vert vif
                else:
                    pixels[x, y] = (0, 128, 0)    # Vert foncé
                    
            elif filename == "sky_tiles.png":
                # Ciel : dégradé bleu
                blue_intensity = min(128 + y * 2, 255)
                pixels[x, y] = (100, 150, blue_intensity)
    
    # Sauvegarder
    output_path = f"../res/{filename}"
    img.save(output_path, 'PNG')
    print(f"✓ Créé: {output_path} ({width}x{height})")

def main():
    print("🏍️ Générateur d'images simples pour SGDK")
    print("=" * 45)
    
    # Créer des images PNG classiques
    create_simple_image("road_tiles.png", (96, 96, 96))
    create_simple_image("grass_tiles.png", (0, 128, 0))
    create_simple_image("sky_tiles.png", (100, 150, 200))
    
    print("\n✅ Images PNG simples générées!")
    print("Format: 64x64 pixels, format classique")

if __name__ == "__main__":
    main()
