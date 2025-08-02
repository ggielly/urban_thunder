/* main.c - Moteur Road Rash avec système IA complet */

#include <genesis.h>
#include "resources.h"
#include "ai_riders.h"
#include "ai_integration.h"

// Prototypes de fonctions
void performPlayerAttack(void);
void checkLevelCompletion(void);
void updatePlayerHealth(void);
void handleSpecialEvents(void);
void completeLevel(void);
void handleGameOver(void);
void renderDebugInfo(void);

// === STRUCTURES DE BASE (inchangées) ===

typedef struct {
    s16 curve;         
    s16 hill;          
    u16 length;        
    u8 decorType;      
    u8 roadType;       
    u8 paletteIndex;   
    u8 eventFlags;     
} TrackSegment;

typedef struct {
    u16 screenY;       
    u16 roadWidth;     
    s16 roadXOffset;   
    u16 scale;         
} RoadStrip;

// === VARIABLES GLOBALES ===

#define MAX_STRIPS 120
#define SCREEN_HEIGHT 224
#define ROAD_BASE_WIDTH 160
#define HORIZON_Y 80

RoadStrip roadStrips[MAX_STRIPS];
TrackSegment level1[] = {
    { 0, 0,  60, 0, 0, 0, 0 },     // ligne droite
    { 15, 2, 80, 1, 1, 1, 0 },     // virage droite + montée
    { -20, -1, 100, 2, 0, 2, 0 },  // virage gauche + descente
    { 0, 0, 40, 0, 0, 0, 0 },      // ligne droite
    { 10, 0, 60, 1, 1, 1, 0 },     // léger virage droite
    { 0, 0, 0xFFFF, 0, 0, 0, 0 }   // fin du niveau
};

// Variables joueur et jeu
s32 trackPosition = 0;
u16 currentSegmentIndex = 0;
s16 playerX = 160;
s16 playerSpeed = 2;
s16 cameraX = 0;
s16 cameraY = 0;
Sprite* player;
u16 playerHealth = 100;
u16 gameScore = 0;
u8 currentLevel = 0;

// Variables système
u16 gameFrameCounter = 0;
bool gamePaused = false;
bool debugMode = false;

// Lookup tables pour optimiser les calculs
u16 scaleTable[224];
u16 widthTable[224];

// Prototypes des routines assembleur (inchangés)
extern void renderRoadStripsASM(RoadStrip* strips, u16 numStrips);
extern void clearPlanA(void);

void enable128kMode() {
    VDP_setReg(1, VDP_getReg(1) | 0x80);
}

void disable128kMode() {
    VDP_setReg(1, VDP_getReg(1) & ~0x80);
}

// === FONCTIONS DE BASE (inchangées mais optimisées) ===

void initLookupTables() {
    u16 i;
    for (i = 0; i < 224; i++) {
        if (i < HORIZON_Y) {
            scaleTable[i] = 0;
            widthTable[i] = 0;
        } else {
            u16 distance = 224 - i;
            scaleTable[i] = (distance * 256) / (224 - HORIZON_Y);
            widthTable[i] = (ROAD_BASE_WIDTH * scaleTable[i]) >> 8;
        }
    }
}

TrackSegment getCurrentSegment() {
    u32 accumLength = 0;
    u16 i = 0;
    
    while (level1[i].length != 0xFFFF) {
        if (trackPosition < accumLength + level1[i].length) {
            return level1[i];
        }
        accumLength += level1[i].length;
        i++;
    }
    
    trackPosition = 0;
    return level1[0];
}

void generateRoadStrips() {
    u16 i;
    TrackSegment currentSeg = getCurrentSegment();
    
    for (i = 0; i < MAX_STRIPS; i++) {
        u16 y = HORIZON_Y + i;
        if (y >= SCREEN_HEIGHT) break;
        
        roadStrips[i].screenY = y;
        roadStrips[i].scale = scaleTable[y];
        roadStrips[i].roadWidth = widthTable[y];
        
        s32 curveEffect = (currentSeg.curve * (224 - y)) >> 4;
        roadStrips[i].roadXOffset = cameraX + curveEffect;
        
        if (roadStrips[i].roadXOffset < -320) roadStrips[i].roadXOffset = -320;
        if (roadStrips[i].roadXOffset > 320) roadStrips[i].roadXOffset = 320;
    }
}

// === GESTION DES ENTRÉES AMÉLIORÉE ===

void handleInput() {
    u16 joy = JOY_readJoypad(JOY_1);
    
    // Pause du jeu
    if (joy & BUTTON_START) {
        static u16 startButtonDelay = 0;
        if (startButtonDelay == 0) {
            gamePaused = !gamePaused;
            startButtonDelay = 30; // Évite le spam
            
            if (gamePaused) {
                VDP_drawText("** PAUSED **", 14, 12);
            } else {
                VDP_drawText("            ", 14, 12);
            }
        }
        startButtonDelay--;
    }
    
    // Mode debug
    if (joy & BUTTON_C) {
        static u16 debugButtonDelay = 0;
        if (debugButtonDelay == 0) {
            debugMode = !debugMode;
            debugButtonDelay = 30;
        }
        debugButtonDelay--;
    }
    
    if (gamePaused) return; // Pas de mouvement en pause
    
    // Contrôles de base
    if (joy & BUTTON_LEFT) {
        playerX -= 3;
        cameraX -= 2;
        
        // Animation de penchée (futur)
        // playerLeanAngle = -15;
    }
    if (joy & BUTTON_RIGHT) {
        playerX += 3;
        cameraX += 2;
        
        // playerLeanAngle = 15;
    }
    
    // Gestion vitesse avec inertie
    if (joy & BUTTON_UP) {
        playerSpeed = min(playerSpeed + 1, 8);
    } else if (joy & BUTTON_DOWN) {
        playerSpeed = max(playerSpeed - 2, 0);
    } else {
        // Décélération naturelle
        if (playerSpeed > 2) {
            playerSpeed = max(playerSpeed - 1, 2);
        }
    }
    
    // Attaque du joueur (bouton A)
    if (joy & BUTTON_A) {
        performPlayerAttack();
    }
    
    // Contraintes joueur
    if (playerX < 80) {
        playerX = 80;
        playerSpeed = max(playerSpeed - 2, 0); // Pénalité sortie route
        playerHealth = max(playerHealth - 1, 0);
    }
    if (playerX > 240) {
        playerX = 240;
        playerSpeed = max(playerSpeed - 2, 0);
        playerHealth = max(playerHealth - 1, 0);
    }
    
    // Avancement sur la piste
    trackPosition += playerSpeed;
}

void performPlayerAttack() {
    static u16 attackCooldown = 0;
    
    if (attackCooldown > 0) {
        attackCooldown--;
        return;
    }
    
    // Recherche d'un rider proche à attaquer
    AIRider* target = getNearestRider(playerX);
    
    if (target != NULL && abs(target->x - playerX) < 32) {
        // Impact sur le rider IA
        target->speed = max(target->speed - 20, 0);
        target->health -= 25;
        target->state = AI_STATE_CRASHED;
        target->stateTimer = 90;
        
        // Gain de score
        gameScore += 100;
        
        // Effet visuel
        VDP_drawText("PUNCH!", 18, 8);
        
        // Cooldown de l'attaque
        attackCooldown = 60; // 1 seconde
        
        // Vérification KO
        if (target->health <= 0) {
            gameScore += 500;
            VDP_drawText("KNOCKOUT!", 16, 9);
        }
    }
    
    attackCooldown = max(attackCooldown, 15); // Cooldown minimal
}

// === MISE À JOUR DU JEU ===

void updateGame() {
    if (gamePaused) return;
    
    gameFrameCounter++;
    
    // Génération de la route
    generateRoadStrips();
    
    // Obtention de la courbure actuelle pour l'IA
    TrackSegment currentSeg = getCurrentSegment();
    s16 roadCurve = currentSeg.curve;
    
    // Mise à jour complète du système IA
    updateFullAISystem(roadCurve);
    
    // Vérification de fin de niveau
    checkLevelCompletion();
    
    // Mise à jour de la santé du joueur
    updatePlayerHealth();
    
    // Gestion des événements spéciaux
    handleSpecialEvents();
}

void checkLevelCompletion() {
    static u32 levelLength = 0;
    
    // Calcul de la longueur totale du niveau (fait une seule fois)
    if (levelLength == 0) {
        u16 i = 0;
        while (level1[i].length != 0xFFFF) {
            levelLength += level1[i].length;
            i++;
        }
    }
    
    // Vérification si le joueur a terminé le niveau
    if (trackPosition >= (s32)levelLength) {
        completeLevel();
    }
}

void completeLevel() {
    // Bonus de fin de niveau
    gameScore += 1000 + (playerHealth * 10);
    
    VDP_drawText("LEVEL COMPLETE!", 12, 12);
    VDP_drawText("BONUS: +1000", 14, 13);
    
    // Passage au niveau suivant (à implémenter)
    currentLevel++;
    
    // Reset pour le prochain niveau
    trackPosition = 0;
    resetAISystem();
    
    // Délai avant le prochain niveau
    VDP_waitVSync(); // Placeholder pour transition
}

void updatePlayerHealth() {
    // Régénération lente de la santé
    if (gameFrameCounter % 300 == 0 && playerHealth < 100) { // Toutes les 5 secondes
        playerHealth = min(playerHealth + 1, 100);
    }
    
    // Vérification game over
    if (playerHealth == 0) {
        handleGameOver();
    }
}

void handleGameOver() {
    VDP_drawText("GAME OVER", 15, 12);
    
    char scoreText[20];
    sprintf(scoreText, "SCORE: %d", gameScore);
    VDP_drawText(scoreText, 13, 14);
    
    // Reset du jeu (simplifié)
    playerHealth = 100;
    gameScore = 0;
    trackPosition = 0;
    currentLevel = 0;
    resetAISystem();
}

void handleSpecialEvents() {
    TrackSegment currentSeg = getCurrentSegment();
    
    // Gestion des événements de segment
    if (currentSeg.eventFlags != 0) {
        // Exemple : spawn de riders spéciaux
        if (currentSeg.eventFlags & 1) { // Bit 0 = spawn agressif
            if ((gameFrameCounter % 120) == 0) { // Toutes les 2 secondes
                spawnAIRider(AI_AGGRESSIVE, 
                           (trackPosition + 200) << 16, 
                           140 + (random() % 40));
            }
        }
        
        // Autres événements...
    }
}

// === RENDU ET INTERFACE ===

void updateSprites() {
    // Sprite du joueur
    SPR_setPosition(player, playerX - 8, 190);
    
    // Animation du joueur selon la vitesse
    u8 playerFrame = (gameFrameCounter >> 2) % 4; // Change toutes les 4 frames
    if (playerSpeed == 0) playerFrame = 0; // Statique si arrêté
    
    SPR_setFrame(player, playerFrame);
}

void renderUI() {
    char uiText[32];
    
    // Score
    sprintf(uiText, "SCORE:%06d", gameScore);
    VDP_drawText(uiText, 1, 1);
    
    // Santé du joueur (barre simple)
    sprintf(uiText, "HEALTH:");
    VDP_drawText(uiText, 25, 1);
    
    // Barre de santé visuelle
    u8 healthBars = playerHealth / 10;
    u8 i;
    for (i = 0; i < 10; i++) {
        VDP_drawText((i < healthBars) ? "|" : ".", 32 + i, 1);
    }
    
    // Vitesse
    sprintf(uiText, "SPEED:%d", playerSpeed);
    VDP_drawText(uiText, 1, 2);
    
    // Niveau actuel
    sprintf(uiText, "LEVEL:%d", currentLevel + 1);
    VDP_drawText(uiText, 25, 2);
    
    // Mode debug
    if (debugMode) {
        renderDebugInfo();
    }
}

void renderDebugInfo() {
    char debugText[32];
    
    // Informations techniques
    sprintf(debugText, "POS:%ld", trackPosition);
    VDP_drawText(debugText, 1, 25);
    
    sprintf(debugText, "CURVE:%d", getCurrentSegment().curve);
    VDP_drawText(debugText, 12, 25);
    
    sprintf(debugText, "FPS:%d", 60); // Placeholder
    VDP_drawText(debugText, 25, 25);
    
    // Info IA (depuis ai_integration.c)
    u8 activeAI = getActiveRiderCount();
    sprintf(debugText, "AI:%d", activeAI);
    VDP_drawText(debugText, 32, 25);
}

// === FONCTION PRINCIPALE ===

int main() {
    // Initialisation SGDK selon documentation officielle
    VDP_init();
    VDP_setScreenWidth320();

    /* DEBUG - SPR_init() CAUSE TOUJOURS ARTÉFACTS !
    // Initialisation SPR AVANT tout chargement de tileset
    SPR_init();
    // Note: SPR_setVRAMTileIndex est pour sprites individuels, pas global
    // SGDK gère automatiquement la VRAM des sprites
    DEBUG */

    // Nettoyage des plans AVANT tout chargement (évite artéfacts)
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    // Test simple d'affichage - SEULEMENT du texte
    VDP_drawText("THUNDER OK", 5, 5);
    VDP_drawText("STABLE", 5, 7);

    // DEBUG - Retour à la méthode qui FONCTIONNAIT avant
    PAL_setPalette(PAL0, main_palette.data, DMA);
    PAL_setPalette(PAL1, simple_palette.data, DMA);
    VDP_drawText("PALETTES OK", 5, 9);

    // Méthode qui FONCTIONNAIT : VDP_loadTileSet avec bons index
    VDP_drawText("LOADING TILES...", 5, 11);

    if (road_tiles.tileset) {
        VDP_loadTileSet(road_tiles.tileset, TILE_USER_INDEX, CPU);
        VDP_drawText("ROAD TILES OK", 5, 13);
    }
    if (grass_tiles.tileset) {
        VDP_loadTileSet(grass_tiles.tileset, TILE_USER_INDEX + 32, CPU);
        VDP_drawText("GRASS TILES OK", 5, 15);
    }
    if (sky_tiles.tileset) {
        VDP_loadTileSet(sky_tiles.tileset, TILE_USER_INDEX + 64, CPU);
        VDP_drawText("SKY TILES OK", 5, 17);
    }
    
    VDP_drawText("NO SPR_INIT", 5, 19);    /* DEBUG_STEP_4 - sprite_ai_bike désactivé dans resources.res
    player = SPR_addSprite(&sprite_ai_bike, playerX - 8, 190, 
                  TILE_ATTR(PAL1, 0, FALSE, FALSE));
    VDP_drawText("SPRITE OK", 5, 17);
    DEBUG_STEP_4 */
    
    /* DEBUG_DISABLE_SPRITES - Initialisation du sprite joueur COMMENTÉE
    player = SPR_addSprite(&sprite_ai_bike, playerX - 8, 190, 
                  TILE_ATTR(PAL1, 0, FALSE, FALSE));
    DEBUG_DISABLE_SPRITES */
    
    /* DEBUG_DISABLE_LOOKUPS - Initialisation des lookup tables COMMENTÉE
    initLookupTables();
    DEBUG_DISABLE_LOOKUPS */
    
    /* DEBUG_DISABLE_AI - Initialisation du système IA COMMENTÉE
    initAIForLevel(currentLevel);
    DEBUG_DISABLE_AI */
    
    /* DEBUG_DISABLE_BACKGROUND - Fond de ciel statique COMMENTÉ
    // Éviter VDP_setTileMapXY sans avoir chargé les tilesets
    u16 x, y;
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 40; x++) {
            VDP_setTileMapXY(BG_B,
                TILE_ATTR_FULL(PAL0, 0, FALSE, FALSE, 
                    TILE_USER_INDEX + 16), x, y);
        }
    }
    DEBUG_DISABLE_BACKGROUND */
    
    /* DEBUG_DISABLE_UI - Interface initiale COMMENTÉE
    // Éviter trop de texte pour l'instant
    VDP_drawText("ROAD RASH MD", 14, 1);
    VDP_drawText("ARROWS: STEER", 13, 26);
    VDP_drawText("A: ATTACK  C: DEBUG", 10, 27);
    DEBUG_DISABLE_UI */
    
    /* DEBUG_DISABLE_ALL_RESOURCE_LOADING - Tout chargement désactivé
    // Configuration des palettes
    PAL_setPalette(PAL0, main_palette.data, DMA);
    PAL_setPalette(PAL1, simple_palette.data, DMA);
    
    // Chargement des ressources avec vérification
    if (road_tiles.tileset) {
        VDP_loadTileSet(road_tiles.tileset, TILE_USER_INDEX, DMA);
    }
    if (grass_tiles.tileset) {
        VDP_loadTileSet(grass_tiles.tileset, TILE_USER_INDEX + 8, DMA);
    }
    if (sky_tiles.tileset) {
        VDP_loadTileSet(sky_tiles.tileset, TILE_USER_INDEX + 16, DMA);
    }
    
    // Initialisation du sprite joueur
    player = SPR_addSprite(&sprite_ai_bike, playerX - 8, 190, 
                  TILE_ATTR(PAL1, 0, FALSE, FALSE));
    
    // Initialisation des lookup tables
    // initLookupTables(); // Temporairement commenté pour debug
    
    // Initialisation du système IA pour le niveau courant  
    // initAIForLevel(currentLevel); // Temporairement commenté pour debug
    
    // Fond de ciel statique
    u16 x, y;
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 40; x++) {
            VDP_setTileMapXY(BG_B, 
                TILE_ATTR_FULL(PAL0, 0, 0, 0, TILE_USER_INDEX + 16), x, y);
        }
    }
    
    // Interface initiale
    VDP_drawText("ROAD RASH MD", 14, 1);
    VDP_drawText("ARROWS: STEER", 13, 26);
    VDP_drawText("A: ATTACK  C: DEBUG", 10, 27);
    DEBUG_DISABLE_ALL_RESOURCE_LOADING */
    
    // Boucle principale Genesis-friendly (SGDK >= 1.6)
    while(TRUE)
    {
        /* DEBUG_DISABLE_ALL_GAME_LOGIC - Toute la logique de jeu commentée
        ... code désactivé ...
        DEBUG_DISABLE_ALL_GAME_LOGIC */

        // Synchronisation VDP et traitement SGDK (évite artefacts)
        SYS_doVBlankProcess();

        enable128kMode();
        renderRoadStripsASM(roadStrips, MAX_STRIPS);
        disable128kMode();
    }

    // Ne jamais retourner de main sur Mega Drive !
    // return 0; // DEBUG_DISABLE_RETURN
}