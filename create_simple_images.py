#!/usr/bin/env python3
"""
Créateur d'images de remplacement simples au format SGDK standard pour la compilation.
Version corrigée avec format de palette compatible ResComp.
"""

from PIL import Image
import os

def create_placeholder_image():
    """Crée une image noire de 128x32 pixels."""
    return Image.new('RGB', (128, 32), color=(0, 0, 0))

def create_palette_image():
    """Crée une image de palette compatible SGDK (128x32 pixels, indexée)."""
    img = Image.new('P', (128, 32))
    # Palette de 16 couleurs
    colors = [
        0, 0, 0, 
        68, 68, 68, 
        136, 136, 136, 
        204, 204, 204,
        255, 255, 255, 
        255, 255, 0, 
        255, 0, 0, 
        0, 255, 0,
        0, 0, 255, 
        255, 136, 0, 
        136, 68, 0, 
        255, 0, 255,
        0, 255, 255, 
        68, 255, 68, 
        0, 136, 255, 
        102, 102, 102
    ]
    # La palette doit être complétée à 768 octets pour Pillow
    palette_data = colors + [0] * (256 * 3 - len(colors))
    img.putpalette(palette_data)

    # Assigner les index de couleur aux pixels
    for y in range(32):
        for x in range(16):
            img.putpixel((x, y), x)
        for x in range(16, 128):
            img.putpixel((x, y), 0)
        
    return img

if __name__ == "__main__":
    print("Génération des images de base pour la compilation SGDK...")
    
    # S'assurer que le répertoire res existe
    res_dir = "res"
    if not os.path.exists(res_dir):
        os.makedirs(res_dir)

    # Créer les images en mémoire
    road_img = create_placeholder_image()
    grass_img = create_placeholder_image()
    sky_img = create_placeholder_image()
    palette_img = create_palette_image()
    
    # Sauvegarder les images dans le répertoire res/
    try:
        road_img.save(os.path.join(res_dir, "road_simple.png"))
        grass_img.save(os.path.join(res_dir, "grass_simple.png"))
        sky_img.save(os.path.join(res_dir, "sky_simple.png"))
        palette_img.save(os.path.join(res_dir, "simple_palette.png"))
        
        print(f"✅ Toutes les images ont été créées avec succès dans le dossier '{res_dir}' !")
        print("  - road_simple.png (128x32)")
        print("  - grass_simple.png (128x32)")
        print("  - sky_simple.png (128x32)") 
        print("  - simple_palette.png (16x1) - FORMAT CORRIGÉ")

    except IOError as e:
        print(f"❌ Erreur lors de la sauvegarde d'un fichier : {e}")
        print(f"Vérifiez les autorisations d'écriture dans le dossier '{res_dir}'.")