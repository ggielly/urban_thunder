#!/usr/bin/env python3
"""Créer des images entièrement noires pour tester SGDK"""

from PIL import Image

def create_uniform_black_image():
    """Créer une image entièrement noire (tous les tiles uniformes)"""
    img = Image.new('RGB', (128, 32), color=(0, 0, 0))  # Entièrement noir
    return img

if __name__ == "__main__":
    print("Création d'images entièrement noires pour test SGDK...")
    
    # Créer l'image
    img = create_uniform_black_image()
    
    # Sauvegarder
    img.save("res/road_simple.png")
    img.save("res/grass_simple.png") 
    img.save("res/sky_simple.png")
    
    print("Images entièrement noires créées !")
    print("- road_simple.png (128x32) - entièrement noir")
    print("- grass_simple.png (128x32) - entièrement noir")
    print("- sky_simple.png (128x32) - entièrement noir")
