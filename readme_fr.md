# 🏍️ Road Rash Mega Drive - Moteur Pseudo-3D

Un moteur de jeu complet type Road Rash pour SEGA Mega Drive, développé avec SGDK et assembleur 68000 optimisé.

## ✨ Fonctionnalités

- **Moteur pseudo-3D** inspiré d'Out Run/Road Rash avec perspective dynamique
- **Segments de piste paramétrés** : virages, pentes, décors multiples
- **Rendu assembleur optimisé** : routines 68000 pour performance maximale
- **Multi-niveaux** : système de chargement de pistes modulaire
- **Support VSCode complet** : IntelliSense, build tasks, debugging

## 🛠️ Installation et Setup

### Prérequis

1. **SGDK** (Sega Genesis Development Kit) installé
   - Windows: `C:\sgdk\`
   - Linux/macOS: `~/sgdk/`

2. **Émulateur Mega Drive**
   - Gens ou BlastEm recommandés
   - Exodus pour debugging avancé

3. **Visual Studio Code** avec extensions :
   - C/C++ (Microsoft)
   - Amiga Assembly (pour syntaxe 68000)
   - Genesis Code (optionnel, pour intégration SGDK)

### Structure du projet

```
road_rash_md/
├── src/
│   ├── main.c              # Moteur principal
│   └── road_engine.s       # Routines ASM optimisées
├── res/
│   ├── resources.res       # Définition ressources
│   ├── road_tiles.png      # Tuiles route 16x16
│   ├── grass_tiles.png     # Tuiles herbe 16x16  
│   ├── sky_tiles.png       # Tuiles ciel 16x16
│   ├── player_bike.png     # Sprite joueur 32x32
│   ├── road_palette.png    # Palette route/décor
│   └── sprites_palette.png # Palette sprites
├── .vscode/
│   ├── settings.json       # Config VSCode
│   └── tasks.json          # Tâches de build
├── makefile.gen            # Makefile SGDK
└── README.md
```

## 🚀 Compilation et Test

### Via VSCode (recommandé)

1. Ouvrir le dossier dans VSCode
2. **Ctrl+Shift+P** → "Tasks: Run Build Task" 
3. Ou **Ctrl+Shift+B** pour build rapide
4. Tester avec **Ctrl+Shift+P** → "Tasks: Run Task" → "Run in Gens"

### Via ligne de commande

```bash
# Build
make -f makefile.gen

# Test 
gens rom.bin
# ou
blastem rom.bin

# Clean
make clean
```

## 🎮 Contrôles

- **Flèches gauche/droite** : Direction
- **Flèche haut** : Accélération
- **Flèche bas** : Freinage

## 🏗️ Architecture Technique

### Moteur Pseudo-3D

Le rendu utilise une approche par **strips horizontaux** :

1. **Segments de piste** : `TrackSegment[]` définit virages, pentes, décors
2. **Génération temps réel** : calcul des `RoadStrip[]` à chaque frame
3. **Rendu assembleur** : blit optimisé tilemap + hscroll par ligne

```c
typedef struct {
    s16 curve;         // Courbure : -32 (gauche) à +32 (droite)  
    s16 hill;          // Pente : -16 (descente) à +16 (montée)
    u16 length;        // Durée du segment
    u8 decorType;      // Type de décor
    u8 roadType;       // Surface route
    u8 paletteIndex;   // Palette VDP
    u8 eventFlags;     // Événements (spawn, checkpoints...)
} TrackSegment;
```

### Optimisations Assembleur

- **renderRoadStripsASM()** : rendu tilemap direct en VRAM
- **clearPlanA()** : effacement rapide zone route
- **Lookup tables** : précalcul échelle/largeur par ligne

### Système Multi-Niveaux

Chaque niveau = tableau de `TrackSegment[]` :
- Stockage compact (~2KB par niveau)
- Chargement dynamique possible
- Séparation logique/visuel (même layout, décors différents)

## 🎨 Création de Contenu

### Génération des Graphics

Utilise le **générateur de tiles HTML** fourni :
1. Ouvre `tile_generator.html` dans un navigateur
2. Génère automatiquement les tiles de base
3. Télécharge les PNG dans le dossier `res/`

### Création de Niveaux

Édite le tableau `level1[]` dans `main.c` :

```c
TrackSegment level1[] = {
    { 0, 0,  60, 0, 0, 0, 0 },     // Ligne droite 60 strips
    { 15, 2, 80, 1, 1, 1, 0 },     // Virage droite + montée 
    { -20, -1, 100, 2, 0, 2, 0 },  // Virage gauche + descente
    { 0, 0, 0xFFFF, 0, 0, 0, 0 }   // Fin niveau
};
```

### Palettes Mega Drive

- **PAL0** : Route, herbe, ciel (16 couleurs)
- **PAL1** : Sprites joueur, objets (16 couleurs)
- **Format indexé** : PNG avec palette fixe

## 🔧 Personnalisation

### Ajout de Nouveaux Décors

1. Créer nouvelles tuiles 16x16
2. Ajouter dans `resources.res`
3. Modifier `decorType` dans les segments
4. Adapter le rendu ASM si nécessaire

### Optimisation Performance

- **VBlank timing** : respecter les 60 FPS
- **VRAM management** : limiter les écritures
- **Sprites** : max 64 total, 20 par ligne
- **DMA transfers** : pour chargements rapides

## 🐛 Debugging

### Outils recommandés

- **Exodus** : debugger cycle-accurate avec visualiseur VDP
- **BlastEm** : debugging gdb intégré  
- **Gens** : test rapide, save states

### Debug dans VSCode

1. Compiler en mode debug : `CFLAGS += -g`
2. Utiliser l'extension Genesis Code
3. Breakpoints dans le code C
4. Monitoring registres 68000

## 📚 Ressources et Références

- [SGDK Documentation](https://github.com/Stephane-D/SGDK)
- [Pseudo-3D Racing Games](http://www.extentofthejam.com/pseudo/)
- [Mega Drive Programming](https://plutiedev.com/)
- [68000 Assembly Reference](http://68k.hax.com/)

## 🚧 Améliorations Futures

- [ ] **Audio** : Musique XGM + effets sonores
- [ ] **IA Concurrents** : Motos adverses avec IA
- [ ] **Collisions avancées** : Système physique réaliste  
- [ ] **Effets visuels** : Particules, météo, jour/nuit
- [ ] **Éditeur de niveaux** : Outil graphique external
- [ ] **Support Rust** : Portage expérimental en Rust

## 📄 Licence

Code source libre pour apprentissage et développement homebrew Mega Drive.

---

**Bon développement ! 🏁**

*Créé avec passion pour la scène homebrew Mega Drive*