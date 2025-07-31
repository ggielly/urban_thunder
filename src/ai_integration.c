/* ai_integration.c - Intégration du système IA dans le moteur Road Rash */

#include <genesis.h>
#include "resources.h"
#include "ai_riders.h"
#include "ai_integration.h"

// Points de spawn prédéfinis pour différents types de niveaux
const AISpawnPoint citySpawns[] = {
    { 200, AI_AGGRESSIVE, 120, 180 },
    { 350, AI_DEFENSIVE, 200, 150 },
    { 450, AI_BLOCKER, 160, 100 },
    { 600, AI_ERRATIC, 140, 120 },
    { 750, AI_RUBBER_BAND, 180, 200 },
    { 0, 0, 0, 0 }  // Terminateur
};

const AISpawnPoint highwaySpawns[] = {
    { 150, AI_RUBBER_BAND, 140, 200 },
    { 280, AI_AGGRESSIVE, 180, 160 },
    { 400, AI_DEFENSIVE, 120, 140 },
    { 520, AI_ERRATIC, 200, 100 },
    { 650, AI_BLOCKER, 160, 120 },
    { 800, AI_AGGRESSIVE, 220, 180 },
    { 0, 0, 0, 0 }
};

const AISpawnPoint mountainSpawns[] = {
    { 300, AI_DEFENSIVE, 160, 180 },
    { 450, AI_RUBBER_BAND, 140, 160 },
    { 600, AI_ERRATIC, 180, 120 },
    { 750, AI_AGGRESSIVE, 200, 140 },
    { 0, 0, 0, 0 }
};

// Variables globales
static const AISpawnPoint* currentSpawns = citySpawns;
static u16 lastSpawnCheck = 0;
static u16 frameCounter = 0;
static u8 difficultyLevel = 1;  // 1-5, influence le comportement IA

// Statistiques de performance (debug)
typedef struct {
    u16 aiUpdatesPerFrame;
    u16 collisionChecks;
    u16 visibleRiders;
    u16 totalSpawned;
} AIStats;

static AIStats aiStats = {0, 0, 0, 0};

// === INTEGRATION AVEC LE MOTEUR PRINCIPAL ===

void initAIForLevel(u8 levelType) {
    // Sélection des spawns selon le type de niveau
    switch (levelType) {
        case 0: // City
            currentSpawns = citySpawns;
            difficultyLevel = 1;
            break;
        case 1: // Highway  
            currentSpawns = highwaySpawns;
            difficultyLevel = 2;
            break;
        case 2: // Mountain
            currentSpawns = mountainSpawns;
            difficultyLevel = 3;
            break;
        default:
            currentSpawns = citySpawns;
            difficultyLevel = 1;
            break;
    }
    
    // Initialisation du système IA
    initAISystem();
    
    // Spawn initial de quelques riders
    spawnInitialRiders();
    
    VDP_drawText("AI LEVEL READY", 1, 2);
}

void spawnInitialRiders(void) {
    u8 i;
    const AISpawnPoint* spawn = currentSpawns;
    
    // Spawn 2-3 riders au début selon la difficulté
    u8 initialCount = 2 + (difficultyLevel >> 1);
    
    for (i = 0; i < initialCount && spawn->spawnDistance != 0; i++, spawn++) {
        s32 spawnZ = (trackPosition + spawn->spawnDistance) << 16;
        spawnAIRider(spawn->aiType, spawnZ, spawn->laneX);
        aiStats.totalSpawned++;
    }
}

void updateAISpawning(void) {
    const AISpawnPoint* spawn = currentSpawns;
    u16 currentDistance = trackPosition >> 4; // Distance parcourue /16
    
    // Vérification de spawn seulement toutes les 15 frames
    if ((frameCounter % 15) != 0) return;
    
    // Parcours des points de spawn
    while (spawn->spawnDistance != 0) {
        u16 spawnDist = lastSpawnCheck + spawn->spawnDistance;
        
        if (currentDistance >= spawnDist) {
            // Vérification probabiliste
            u16 rand = random() & 0xFF;
            u8 adjustedProb = spawn->probability;
            
            // Ajustement de probabilité selon la difficulté
            adjustedProb = (adjustedProb * difficultyLevel) / 3;
            if (adjustedProb > 255) adjustedProb = 255;
            
            if (rand < adjustedProb) {
                // Calcul de la position de spawn
                s32 spawnZ = (trackPosition + (spawn->spawnDistance << 4)) << 16;
                s16 laneVariation = (random() % 40) - 20; // ±20 pixels
                s16 finalX = spawn->laneX + laneVariation;
                
                // Vérification qu'on n'a pas trop de riders actifs
                if (getActiveRiderCount() < (4 + difficultyLevel)) {
                    spawnAIRider(spawn->aiType, spawnZ, finalX);
                    aiStats.totalSpawned++;
                }
            }
            
            lastSpawnCheck = currentDistance;
        }
        spawn++;
    }
}

void updateAIDifficulty(void) {
    // Augmentation progressive de la difficulté
    static u16 difficultyTimer = 0;
    
    difficultyTimer++;
    
    // Augmente la difficulté toutes les 30 secondes (1800 frames)
    if (difficultyTimer >= 1800) {
        difficultyTimer = 0;
        
        if (difficultyLevel < 5) {
            difficultyLevel++;
            
            // Notification visuelle
            char diffText[20];
            sprintf(diffText, "DIFFICULTY: %d", difficultyLevel);
            VDP_drawText(diffText, 1, 3);
            
            // Boost de tous les riders actifs
            boostActiveRiders();
        }
    }
}

void boostActiveRiders(void) {
    u8 i;
    
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        if (aiRiders[i].active) {
            // Augmentation des stats selon la difficulté
            aiRiders[i].maxSpeed += (difficultyLevel * 5);
            aiRiders[i].aggressionLevel = min(aiRiders[i].aggressionLevel + 20, 255);
            aiRiders[i].acceleration += 1;
            
            // Limitation pour éviter les valeurs extrêmes
            if (aiRiders[i].maxSpeed > 280) {
                aiRiders[i].maxSpeed = 280;
            }
        }
    }
}

// === INTEGRATION AVEC LE SYSTEME DE COLLISION ===

void handlePlayerAIInteraction(void) {
    u8 i;
    
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        if (!aiRiders[i].active || !aiRiders[i].visible) continue;
        
        AIRider* rider = &aiRiders[i];
        s16 distance = abs(rider->x - playerX);
        
        // Interaction proche (pas forcément collision)
        if (distance < 40) {
            handleCloseInteraction(rider);
        }
        
        // Vérification collision précise
        if (checkCollisionASM(rider->x, rider->y, playerX, 190, 18)) {
            handlePlayerAICollision(rider);
            aiStats.collisionChecks++;
            
            // Effets visuels/sonores de collision
            triggerCollisionEffects(rider->x, rider->y);
        }
    }
}

void handleCloseInteraction(AIRider* rider) {
    // Interactions non-collision (intimidation, blocage, etc.)
    switch (rider->aiType) {
        case AI_AGGRESSIVE:
            // Essaie d'intimider le joueur
            if (rider->x < playerX + 20 && rider->x > playerX - 20) {
                // Réduction légère de la vitesse joueur
                if (playerSpeed > 0) {
                    playerSpeed = max(playerSpeed - 1, 0);
                }
            }
            break;
            
        case AI_BLOCKER:
            // Ajuste sa position pour bloquer
            if (abs(rider->x - playerX) < 30) {
                rider->targetX = playerX + ((rider->x < playerX) ? -15 : 15);
                rider->speed = min(rider->speed, playerSpeed + 1);
            }
            break;
            
        case AI_DEFENSIVE:
            // S'écarte activement
            if (rider->x < playerX + 25 && rider->x > playerX - 25) {
                rider->state = AI_STATE_AVOIDING;
                rider->stateTimer = 60;
            }
            break;
            
        default:
            break;
    }
}

void triggerCollisionEffects(s16 x, s16 y) {
    // Effet visuel simple - flash de l'écran
    PAL_setColor(0, 0x0EEE); // Flash blanc
    
    // Note: Ici tu peux ajouter:
    // - Particules de crash
    // - Effets sonores
    // - Shake de caméra
    // - Score/dégâts
    
    // Reset de la couleur après quelques frames
    // (à implémenter avec un timer)
}

// === GESTION DE LA PERFORMANCE ===

void optimizeAIPerformance(void) {
    static u8 optimizeCounter = 0;
    
    optimizeCounter++;
    
    // Optimisation toutes les 60 frames (1 seconde)
    if (optimizeCounter >= 60) {
        optimizeCounter = 0;
        
        // Tri des riders par distance pour optimiser le culling
        sortRidersByDistance(aiRiders, activeRiders);
        
        // Désactivation des riders trop éloignés
        cullDistantRiders();
        
        // Réduction de la fréquence de mise à jour pour les riders éloignés
        adjustUpdateFrequency();
    }
}

void cullDistantRiders(void) {
    u8 i;
    s32 playerWorldZ = trackPosition << 16;
    const s32 maxDistance = 800 << 16; // 800 unités max
    
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        if (!aiRiders[i].active) continue;
        
        s32 distance = abs(aiRiders[i].worldZ - playerWorldZ);
        
        if (distance > maxDistance) {
            // Désactivation du rider éloigné
            disableAIRider(&aiRiders[i]);
        }
    }
}

void adjustUpdateFrequency(void) {
    u8 i;
    s32 playerWorldZ = trackPosition << 16;
    
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        if (!aiRiders[i].active) continue;
        
        s32 distance = abs(aiRiders[i].worldZ - playerWorldZ);
        
        // Réduction de fréquence pour les riders éloignés
        if (distance > (300 << 16)) {
            // Mise à jour une frame sur deux
            if ((frameCounter & 1) == (i & 1)) {
                continue; // Skip cette frame
            }
        }
    }
}

// === INTERFACE PUBLIQUE POUR LE MOTEUR PRINCIPAL ===

void updateFullAISystem(s16 roadCurve) {
    frameCounter++;
    
    // Reset des stats de frame
    aiStats.aiUpdatesPerFrame = 0;
    aiStats.collisionChecks = 0;
    aiStats.visibleRiders = 0;
    
    // Mise à jour du spawning
    updateAISpawning();
    
    // Mise à jour de la difficulté
    updateAIDifficulty();
    
    // Mise à jour du système IA principal
    updateAISystem(roadCurve);
    
    // Interactions avec le joueur
    handlePlayerAIInteraction();
    
    // Optimisations de performance
    optimizeAIPerformance();
    
    // Mise à jour des statistiques
    updateAIStatistics();
}

void updateAIStatistics(void) {
    u8 i, visible = 0;
    
    // Comptage des riders visibles
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        if (aiRiders[i].active && aiRiders[i].visible) {
            visible++;
        }
    }
    
    aiStats.visibleRiders = visible;
    aiStats.aiUpdatesPerFrame = activeRiders;
    
    // Affichage debug (optionnel)
    #ifdef DEBUG_AI
    displayAIDebugInfo();
    #endif
}

#ifdef DEBUG_AI
void displayAIDebugInfo(void) {
    char debugText[32];
    
    sprintf(debugText, "RIDERS:%d VIS:%d", activeRiders, aiStats.visibleRiders);
    VDP_drawText(debugText, 1, 26);
    
    sprintf(debugText, "DIFF:%d SPAWN:%d", difficultyLevel, aiStats.totalSpawned);
    VDP_drawText(debugText, 1, 27);
}
#endif

// === FONCTIONS UTILITAIRES ===

u8 getActiveRiderCount(void) {
    u8 count = 0, i;
    
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        if (aiRiders[i].active) count++;
    }
    
    return count;
}

AIRider* getNearestRider(s16 playerX) {
    u8 i;
    AIRider* nearest = NULL;
    s16 minDistance = 1000;
    
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        if (!aiRiders[i].active || !aiRiders[i].visible) continue;
        
        s16 distance = abs(aiRiders[i].x - playerX);
        if (distance < minDistance) {
            minDistance = distance;
            nearest = &aiRiders[i];
        }
    }
    
    return nearest;
}

void resetAISystem(void) {
    u8 i;
    
    // Désactivation de tous les riders
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        if (aiRiders[i].active) {
            disableAIRider(&aiRiders[i]);
        }
    }
    
    // Reset des statistiques
    aiStats.totalSpawned = 0;
    aiStats.aiUpdatesPerFrame = 0;
    aiStats.collisionChecks = 0;
    aiStats.visibleRiders = 0;
    
    activeRiders = 0;
    frameCounter = 0;
    lastSpawnCheck = 0;
    difficultyLevel = 1;
    
    VDP_drawText("AI SYSTEM RESET", 1, 1);
}

// === SAUVEGARDE/CHARGEMENT ÉTAT IA (optionnel) ===

void saveAIState(AISaveState* state) {
    state->activeRiders = activeRiders;
    state->difficultyLevel = difficultyLevel;
    state->trackPosition = trackPosition;
    state->frameCounter = frameCounter;
}

void loadAIState(const AISaveState* state) {
    difficultyLevel = state->difficultyLevel;
    trackPosition = state->trackPosition;
    frameCounter = state->frameCounter;
    
    // Note: La restauration complète des riders nécessiterait
    // plus de données (positions, états, etc.)
}