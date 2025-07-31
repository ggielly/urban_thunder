#ifndef _AI_INTEGRATION_H_
#define _AI_INTEGRATION_H_

#include "genesis.h"
#include "ai_riders.h"

// Types de niveaux pour le spawning IA
typedef enum {
    LEVEL_CITY = 0,
    LEVEL_HIGHWAY,
    LEVEL_MOUNTAIN
} LevelType;

// Structure pour la sauvegarde de l'état IA
typedef struct {
    u8 activeRiders;
    u8 difficultyLevel;
    s32 trackPosition;
    u16 frameCounter;
} AISaveState;

// Configuration du spawning des IA
typedef struct {
    u16 spawnDistance;      // Distance de spawn
    AIType aiType;          // Type d'IA à spawner
    s16 laneX;             // Position X de spawn
    u8 probability;         // Probabilité de spawn (0-255)
} AISpawnPoint;

// Variables globales (extern pour utilisation dans d'autres fichiers)
extern u8 activeRiders;
extern AIRider aiRiders[MAX_AI_RIDERS];
extern s32 trackPosition;

// Fonctions d'initialisation et configuration
void initAIForLevel(u8 levelType);
void spawnInitialRiders(void);
void resetAISystem(void);

// Fonctions de mise à jour principales
void updateAISpawning(void);
void updateAIDifficulty(void);
void updateFullAISystem(s16 roadCurve);

// Gestion des interactions avec le joueur
void handlePlayerAIInteraction(void);
void handleCloseInteraction(AIRider* rider);
void triggerCollisionEffects(s16 x, s16 y);

// Optimisation et performance
void optimizeAIPerformance(void);
void cullDistantRiders(void);
void adjustUpdateFrequency(void);
void boostActiveRiders(void);

// Statistiques et debug
void updateAIStatistics(void);
void displayAIDebugInfo(void);
u8 getActiveRiderCount(void);
AIRider* getNearestRider(s16 playerX);

// Sauvegarde/chargement
void saveAIState(AISaveState* state);
void loadAIState(const AISaveState* state);

#endif // _AI_INTEGRATION_H_
