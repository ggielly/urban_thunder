# resources.res - Définition des ressources pour SGDK

# Images simples pour la route et l'environnement (format PNG classique)
#PALETTE main_palette "res/simple_palette.png"
PALETTE main_palette "res/simple_palette.png" 
IMAGE road_tiles "res/road_simple.png"
IMAGE grass_tiles "res/grass_simple.png"
IMAGE sky_tiles "res/sky_simple.png"

# Sprites pour le joueur et les objets
# SPRITE sprite_player "res/player_bike_sprite_(32x32).png" 2 2 FAST 5
# SPRITE sprite_ai_bike "res/road_simple.png" 2 2 FAST 0 // DEBUG - CONFLIT AVEC road_tiles !

# Sprite temporairement désactivé pour éviter conflit VRAM
# SPRITE sprite_ai_bike "res/player_bike_sprite_(32x32).png" 2 2 FAST 0

# Palettes
PALETTE simple_palette "res/simple_palette.png"

# Musique et sons (optionnel - à ajouter plus tard)
# XGM music_level1 "res/level1_music.vgm"
# WAV sound_engine "res/engine_sound.wav" 