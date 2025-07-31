#ifndef _AI_RIDERS_H_
#define _AI_RIDERS_H_

#include "genesis.h"

// Constantes
#define MAX_AI_RIDERS 8

// Types d'IA
typedef enum {
    AI_AGGRESSIVE = 0,    // Direct attacks, minimal avoidance
    AI_DEFENSIVE,         // Collision avoidance, cautious driving
    AI_ERRATIC,          // Unpredictable behavior patterns
    AI_RUBBER_BAND,      // Speed adjustment based on player position
    AI_BLOCKER          // Actively blocks player progress
} AIType;

// États de l'IA
typedef enum {
    AI_STATE_RACING = 0,     // Normal racing behavior
    AI_STATE_ATTACKING,      // Active player attack
    AI_STATE_AVOIDING,       // Obstacle/collision avoidance
    AI_STATE_CRASHED,        // Post-crash recovery
    AI_STATE_CATCHING_UP     // Rubber-band catch-up mode
} AIState;

// Structure du rider IA
typedef struct {
    // Physics and position (fixed-point 16.16 for precision)
    s16 x, y;                // Screen coordinates
    s32 worldX, worldZ;      // World position (16.16 fixed-point)
    s16 speed;               // Current speed
    s16 maxSpeed;            // Maximum speed capability
    s16 acceleration;        // Acceleration rate
    s16 handling;            // Steering responsiveness (0-255)
    
    // AI behavior parameters
    AIType aiType;           // Personality type
    AIState state;           // Current state
    u16 stateTimer;          // State duration counter
    s16 targetX;             // Target X position
    s16 decisionTimer;       // Decision-making interval
    
    // Adaptive difficulty
    s16 rubberBandStrength;  // Rubber-band effect strength
    s16 playerDistance;      // Distance to player
    u16 aggressionLevel;     // Aggression intensity (0-255)
    
    // Rendering and animation
    u8 spriteIndex;          // Assigned sprite slot
    u8 animFrame;            // Current animation frame
    u8 animTimer;            // Animation timing
    bool visible;            // Visibility culling flag
    bool active;             // Simulation active flag
    
    // Combat system
    u16 attackTimer;         // Attack cooldown
    s16 health;              // Damage points
    bool canAttack;          // Attack capability flag
} AIRider;

// Variables globales
extern AIRider aiRiders[MAX_AI_RIDERS];
extern u8 activeRiders;
extern s32 trackPosition;
extern s16 playerX;
extern s16 playerSpeed;

// Fonctions publiques
void updateAIPhysicsASM(AIRider* rider, s16 targetX, s16 roadCurve);
bool checkCollisionASM(s16 x1, s16 y1, s16 x2, s16 y2, s16 threshold);
void sortRidersByDistance(AIRider* riders, u16 count);

// Fonctions d'intégration IA
void initAISystem(void);
void spawnAIRider(AIType aiType, s32 worldZ, s16 laneX);
void disableAIRider(AIRider* rider);
void updateAISystem(s16 roadCurve);
void handlePlayerAICollision(AIRider* rider);
void spawnInitialRiders(void);
void boostActiveRiders(void);
void handleCloseInteraction(AIRider* rider);
void triggerCollisionEffects(s16 x, s16 y);
void cullDistantRiders(void);
void adjustUpdateFrequency(void);
void updateAIStatistics(void);
u8 getActiveRiderCount(void);
AIRider* getNearestRider(s16 x);

// Fonctions internes AI
void updateRacingAI(AIRider* rider);
void updateAttackingAI(AIRider* rider);
void updateAvoidingAI(AIRider* rider);
void updateCrashedAI(AIRider* rider);
void updateCatchingUpAI(AIRider* rider);
void updateRubberBandAI(AIRider* rider);
void performAIAttack(AIRider* rider);
void handleAICollision(AIRider* rider1, AIRider* rider2);
void assignSpriteToAI(AIRider* rider);
void releaseSpriteFromAI(AIRider* rider);
void updateAIAnimation(AIRider* rider);
u8 findFreeSprite(void);

#endif // _AI_RIDERS_H_
