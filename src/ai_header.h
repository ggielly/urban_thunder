/* ai_riders.h - Header pour le système IA des concurrents Road Rash */

#ifndef AI_RIDERS_H
#define AI_RIDERS_H

#include <genesis.h>

// === CONSTANTES ===

#define MAX_AI_RIDERS 8
#define AI_DECISION_INTERVAL 30
#define AI_SIGHT_DISTANCE 200
#define AI_ATTACK_RANGE 32
#define RUBBER_BAND_DISTANCE 400

// === ENUMS ===

typedef enum {
    AI_AGGRESSIVE = 0,    // Fonce droit, peu d'évitement
    AI_DEFENSIVE,         // Évite les collisions, prudent
    AI_ERRATIC,          // Comportement imprévisible
    AI_RUBBER_BAND,      // Ajuste sa vitesse selon le joueur
    AI_BLOCKER          // Essaie de bloquer le joueur
} AIType;

typedef enum {
    AI_STATE_RACING = 0,     // Course normale
    AI_STATE_ATTACKING,      // Attaque le joueur
    AI_STATE_AVOIDING,       // Évite un obstacle
    AI_STATE_CRASHED,        // En train de tomber/récupérer
    AI_STATE_CATCHING_UP     // Rattrapage rubber band
} AIState;

// === STRUCTURES ===

typedef struct {
    // Position et physique
    s16 x, y;                // Position écran
    s32 worldX, worldZ;      // Position monde (précision fixe 16.16)
    s16 speed;               // Vitesse actuelle
    s16 maxSpeed;            // Vitesse max de ce rider
    s16 acceleration;        // Accélération
    s16 handling;            // Maniabilité (0-255)
    
    // IA et comportement
    AIType aiType;           // Type d'IA
    AIState state;           // État actuel
    u16 stateTimer;          // Timer pour l'état courant
    s16 targetX;             // Position X cible
    s16 decisionTimer;       // Timer pour prendre des décisions
    
    // Rubber band et difficulté adaptative
    s16 rubberBandStrength;  // Force du rubber band (0-255)
    s16 playerDistance;      // Distance au joueur
    u16 aggressionLevel;     // Niveau d'agressivité (0-255)
    
    // Animation et rendu
    u8 spriteIndex;          // Index du sprite assigné
    u8 animFrame;            // Frame d'animation courante
    u8 animTimer;            // Timer d'animation
    bool visible;            // Visible à l'écran
    bool active;             // Actif dans la simulation
    
    // Combat et interactions
    u16 attackTimer;         // Cooldown des attaques
    s16 health;              // Points de vie (crashes)
    bool canAttack;          // Peut attaquer ce frame
} AIRider;

// === VARIABLES GLOBALES ===

extern AIRider aiRiders[MAX_AI_RIDERS];
extern u8 activeRiders;

// Variables externes du moteur principal
extern s16 playerX, playerSpeed;
extern u32 trackPosition;

// === PROTOTYPES DE FONCTIONS ===

// Initialisation et gestion
void initAISystem(void);
void spawnAIRider(AIType type, s32 worldZ, s16 laneX);
void disableAIRider(AIRider* rider);
void resetAISystem(void);

// IA et décisions
void updateAIDecisions(AIRider* rider);
void updateRacingAI(AIRider* rider);
void updateAttackingAI(AIRider* rider);
void updateAvoidingAI(AIRider* rider);
void updateCrashedAI(AIRider* rider);
void updateCatchingUpAI(AIRider* rider);
void updateRubberBandAI(AIRider* rider);

// Combat et interactions
void performAIAttack(AIRider* rider);
void checkAICollisions(void);
void handlePlayerAICollision(AIRider* rider);
void handleAICollision(AIRider* rider1, AIRider* rider2);

// Physique et mouvement
void updateAIPhysics(AIRider* rider, s16 roadCurve);

// Rendu et culling
void updateAIVisibility(void);
void assignSpriteToAI(AIRider* rider);
void releaseSpriteFromAI(AIRider* rider);
void renderAIRiders(void);
void updateAIAnimation(AIRider* rider);

// Interface publique
void updateAISystem(s16 roadCurve);
u8 findFreeSprite(void);

// Fonctions utilitaires
u8 getActiveRiderCount(void);
AIRider* getNearestRider(s16 playerX);

// === PROTOTYPES ASSEMBLEUR ===

extern void updateAIPhysicsASM(AIRider* rider, s16 targetX, s16 roadCurve);
extern bool checkCollisionASM(s16 x1, s16 y1, s16 x2, s16 y2, s16 threshold);
extern void sortRidersByDistance(AIRider* riders, u8 count);
extern void calculateAISpacing(AIRider* riders, u8 count, s16 playerSpeed);
extern u16 fastRandomASM(void);
extern void updateAIBehaviorFlagsASM(AIRider* riders, u8 count, u16 frameCounter);
extern void calculateRelativePositions(AIRider* riders, u8 count, s16 playerX, s32 playerZ);

#endif /* AI_RIDERS_H */