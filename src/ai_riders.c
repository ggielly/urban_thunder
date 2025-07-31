#include <genesis.h>
#include "resources.h"
#include "ai_riders.h"

// Configuration globale IA
#define AI_DECISION_INTERVAL 30    // Frames entre décisions
#define AI_SIGHT_DISTANCE 200      // Distance de "vue" de l'IA
#define AI_ATTACK_RANGE 32         // Portée d'attaque
#define RUBBER_BAND_DISTANCE 400   // Distance max rubber band

// Variables globales
AIRider aiRiders[MAX_AI_RIDERS];
u8 activeRiders = 0;

// Tables de personnalité pré-calculées
const s16 aiPersonalities[5][6] = {
    // maxSpeed, accel, handling, rubber, aggression, attackFreq
    { 240, 4, 180, 50,  200, 120 },  // AGGRESSIVE
    { 200, 3, 220, 80,  80,  180 },  // DEFENSIVE  
    { 220, 5, 140, 30,  150, 90 },   // ERRATIC
    { 210, 3, 200, 180, 100, 150 },  // RUBBER_BAND
    { 180, 2, 160, 60,  250, 60 }    // BLOCKER
};

// === INITIALISATION ET GESTION ===

void initAISystem(void) {
    u8 i;
    
    // Reset de tous les riders
    for (i = 0; i < MAX_AI_RIDERS; i++) {
        aiRiders[i].active = FALSE;
        aiRiders[i].visible = FALSE;
        aiRiders[i].spriteIndex = 0xFF; // Non assigné
    }
    
    activeRiders = 0;
    
    VDP_drawText("AI SYSTEM READY", 1, 1);
}

void spawnAIRider(AIType type, s32 worldZ, s16 laneX) {
    if (activeRiders >= MAX_AI_RIDERS) return;
    
    u8 index = activeRiders;
    AIRider* rider = &aiRiders[index];
    
    // Configuration de base
    rider->worldX = (s32)laneX << 16;
    rider->worldZ = worldZ;
    rider->x = laneX;
    rider->y = 120; // Position Y de base
    
    // Personnalité basée sur le type
    rider->aiType = type;
    rider->maxSpeed = aiPersonalities[type][0];
    rider->acceleration = aiPersonalities[type][1];
    rider->handling = aiPersonalities[type][2];
    rider->rubberBandStrength = aiPersonalities[type][3];
    rider->aggressionLevel = aiPersonalities[type][4];
    
    // État initial
    rider->state = AI_STATE_RACING;
    rider->speed = rider->maxSpeed >> 1; // Démarre à mi-vitesse
    rider->stateTimer = 0;
    rider->decisionTimer = random() % AI_DECISION_INTERVAL;
    rider->targetX = laneX;
    
    // Combat
    rider->health = 100;
    rider->attackTimer = 0;
    rider->canAttack = TRUE;
    
    // Animation
    rider->animFrame = 0;
    rider->animTimer = 0;
    
    // Activation
    rider->active = TRUE;
    rider->visible = FALSE; // Sera activé par le culling
    
    activeRiders++;
}

// === IA DECISION MAKING ===

void updateAIDecisions(AIRider* rider) {
    if (rider->decisionTimer > 0) {
        rider->decisionTimer--;
        return;
    }
    
    // Reset du timer de décision
    rider->decisionTimer = AI_DECISION_INTERVAL + (random() % 20) - 10;
    
    // Calcul de la distance au joueur
    s16 playerDist = abs(rider->x - playerX);
    rider->playerDistance = playerDist;
    
    // Machine à états simplifiée
    switch (rider->state) {
        case AI_STATE_RACING:
            updateRacingAI(rider);
            break;
            
        case AI_STATE_ATTACKING:
            updateAttackingAI(rider);
            break;
            
        case AI_STATE_AVOIDING:
            updateAvoidingAI(rider);
            break;
            
        case AI_STATE_CRASHED:
            updateCrashedAI(rider);
            break;
            
        case AI_STATE_CATCHING_UP:
            updateCatchingUpAI(rider);
            break;
    }
}

void updateRacingAI(AIRider* rider) {
    s16 laneCenter;
    s16 avoidanceX;
    
    switch (rider->aiType) {
        case AI_AGGRESSIVE:
            // Fonce vers le joueur ou la ligne droite
            if (rider->playerDistance < AI_SIGHT_DISTANCE && rider->aggressionLevel > 150) {
                rider->targetX = playerX + ((random() % 40) - 20);
                if (rider->playerDistance < AI_ATTACK_RANGE) {
                    rider->state = AI_STATE_ATTACKING;
                    rider->stateTimer = 60; // 1 seconde d'attaque
                }
            } else {
                rider->targetX = 160 + ((random() % 80) - 40); // Centre ±40
            }
            break;
            
        case AI_DEFENSIVE:
            // Évite le joueur et les autres riders
            laneCenter = 160;
            avoidanceX = 0;
            
            // Évitement du joueur
            if (rider->playerDistance < 80) {
                avoidanceX = (rider->x < playerX) ? -60 : 60;
            }
            
            rider->targetX = laneCenter + avoidanceX;
            
            // Changement d'état si trop proche
            if (rider->playerDistance < 40) {
                rider->state = AI_STATE_AVOIDING;
                rider->stateTimer = 90;
            }
            break;
            
        case AI_ERRATIC:
            // Comportement chaotique
            if ((random() % 100) < 30) { // 30% de chance de changer
                rider->targetX = 100 + random() % 120; // Entre 100-220
            }
            
            // Attaque occasionnelle
            if (rider->playerDistance < 60 && (random() % 100) < 25) {
                rider->state = AI_STATE_ATTACKING;
                rider->stateTimer = 30;
            }
            break;
            
        case AI_RUBBER_BAND:
            updateRubberBandAI(rider);
            break;
            
        case AI_BLOCKER:
            // Essaie de bloquer le joueur
            rider->targetX = playerX + ((random() % 20) - 10);
            
            // Ralentit si devant le joueur
            if (rider->worldZ > (s32)(trackPosition << 16)) {
                rider->speed = min(rider->speed, playerSpeed + 1);
            }
            break;
    }
    
    // Contraintes de la route
    if (rider->targetX < 80) rider->targetX = 80;
    if (rider->targetX > 240) rider->targetX = 240;
}

void updateRubberBandAI(AIRider* rider) {
    s32 playerWorldZ = trackPosition << 16;
    s32 distanceToPlayer = rider->worldZ - playerWorldZ;
    
    // Rubber band - ajuste la vitesse selon la distance
    if (distanceToPlayer > (RUBBER_BAND_DISTANCE << 16)) {
        // Trop loin derrière - accélère
        rider->state = AI_STATE_CATCHING_UP;
        rider->stateTimer = 120;
        rider->speed = min(rider->maxSpeed + 20, 255);
    } else if (distanceToPlayer < -(RUBBER_BAND_DISTANCE << 16)) {
        // Trop loin devant - ralentit
        rider->speed = max(rider->speed - 2, rider->maxSpeed >> 2);
    } else {
        // Distance correcte - vitesse normale
        rider->speed = rider->maxSpeed;
    }
    
    // Position cible normale
    rider->targetX = 160 + ((random() % 60) - 30);
}

void updateAttackingAI(AIRider* rider) {
    // Fonce vers le joueur
    rider->targetX = playerX;
    rider->speed = min(rider->speed + 2, rider->maxSpeed + 10);
    
    // Vérification de collision pour attaque
    if (rider->playerDistance < AI_ATTACK_RANGE && rider->canAttack) {
        performAIAttack(rider);
    }
    
    // Timeout de l'attaque
    if (rider->stateTimer > 0) {
        rider->stateTimer--;
    } else {
        rider->state = AI_STATE_RACING;
        rider->attackTimer = 180; // Cooldown 3 secondes
    }
}

void updateAvoidingAI(AIRider* rider) {
    // Mouvement d'évitement
    s16 avoidDir = (rider->x < playerX) ? -1 : 1;
    rider->targetX = rider->x + (avoidDir * 80);
    
    // Ralentit légèrement
    rider->speed = max(rider->speed - 1, rider->maxSpeed >> 2);
    
    if (rider->stateTimer > 0) {
        rider->stateTimer--;
    } else {
        rider->state = AI_STATE_RACING;
    }
}

void updateCrashedAI(AIRider* rider) {
    // Récupération après crash
    rider->speed = 0;
    rider->targetX = rider->x; // Reste sur place
    
    if (rider->stateTimer > 0) {
        rider->stateTimer--;
    } else {
        rider->state = AI_STATE_RACING;
        rider->speed = rider->maxSpeed >> 2; // Redémarre lentement
        rider->health = min(rider->health + 20, 100);
    }
}

void updateCatchingUpAI(AIRider* rider) {
    // Mode rattrapage accéléré
    rider->speed = min(rider->maxSpeed + 30, 255);
    rider->targetX = 160; // Ligne droite pour rattraper
    
    if (rider->stateTimer > 0) {
        rider->stateTimer--;
    } else {
        rider->state = AI_STATE_RACING;
    }
}

// === COMBAT ET INTERACTIONS ===

void performAIAttack(AIRider* rider) {
    if (rider->attackTimer > 0) return;
    
    // Logique d'attaque selon le type
    switch (rider->aiType) {
        case AI_AGGRESSIVE:
        case AI_BLOCKER:
            // Coup de poing/kick
            if (checkCollisionASM(rider->x, rider->y, playerX, 190, 24)) {
                // Impact sur le joueur
                playerSpeed = max(playerSpeed - 15, 0);
                // Effect visuel/sonore ici
                VDP_drawText("HIT!", 15, 10);
            }
            break;
            
        case AI_ERRATIC:
            // Attaque chaotique - peut rater
            if ((random() % 100) < 70) { // 70% de réussite
                if (checkCollisionASM(rider->x, rider->y, playerX, 190, 28)) {
                    playerSpeed = max(playerSpeed - 10, 0);
                }
            }
            break;
            
        default:
            // Autres types attaquent moins fort
            if (checkCollisionASM(rider->x, rider->y, playerX, 190, 20)) {
                playerSpeed = max(playerSpeed - 5, 0);
            }
            break;
    }
    
    rider->canAttack = FALSE;
    rider->attackTimer = 120 + (random() % 60); // Cooldown variable
}

void checkAICollisions(void) {
    u8 i, j;
    
    // Collisions entre IA
    for (i = 0; i < activeRiders; i++) {
        if (!aiRiders[i].active || !aiRiders[i].visible) continue;
        
        // Collision avec le joueur
        if (checkCollisionASM(aiRiders[i].x, aiRiders[i].y, playerX, 190, 20)) {
            handlePlayerAICollision(&aiRiders[i]);
        }
        
        // Collisions entre IA
        for (j = i + 1; j < activeRiders; j++) {
            if (!aiRiders[j].active || !aiRiders[j].visible) continue;
            
            if (checkCollisionASM(aiRiders[i].x, aiRiders[i].y, 
                                aiRiders[j].x, aiRiders[j].y, 18)) {
                handleAICollision(&aiRiders[i], &aiRiders[j]);
            }
        }
    }
}

void handlePlayerAICollision(AIRider* rider) {
    // Effets de la collision
    s16 impactForce = (rider->speed + playerSpeed) >> 3;
    
    // Impact sur le joueur
    if (playerSpeed > rider->speed) {
        // Joueur plus rapide - pousse l'IA
        rider->speed = max(rider->speed - impactForce, 0);
        rider->state = AI_STATE_CRASHED;
        rider->stateTimer = 60;
        rider->health -= 15;
    } else {
        // IA plus rapide - impacte le joueur  
        playerSpeed = max(playerSpeed - (impactForce >> 1), 0);
        // Décalage latéral du joueur
        playerX += (rider->x > playerX) ? -10 : 10;
    }
    
    // Vérification KO de l'IA
    if (rider->health <= 0) {
        disableAIRider(rider);
    }
}

void handleAICollision(AIRider* rider1, AIRider* rider2) {
    // Collision entre deux IA - échange de vitesse partiel
    s16 speed1 = rider1->speed;
    s16 speed2 = rider2->speed;
    
    rider1->speed = (speed1 + speed2) >> 1;
    rider2->speed = (speed1 + speed2) >> 1;
    
    // Les deux passent en évitement
    rider1->state = AI_STATE_AVOIDING;
    rider1->stateTimer = 45;
    
    rider2->state = AI_STATE_AVOIDING;  
    rider2->stateTimer = 45;
}

// === PHYSIQUE ET MOUVEMENT ===

void updateAIPhysics(AIRider* rider, s16 roadCurve) {
    // Appel de la routine assembleur optimisée
    updateAIPhysicsASM(rider, rider->targetX, roadCurve);
    
    // Mise à jour position monde
    rider->worldZ += (s32)rider->speed << 14; // Conversion vitesse -> mouvement
    
    // Contraintes physiques
    if (rider->x < 60) {
        rider->x = 60;
        rider->speed = max(rider->speed - 5, 0); // Pénalité bord route
    }
    if (rider->x > 260) {
        rider->x = 260;
        rider->speed = max(rider->speed - 5, 0);
    }
    
    // Dégradation naturelle de la vitesse
    if (rider->speed > rider->maxSpeed) {
        rider->speed = max(rider->speed - 1, rider->maxSpeed);
    }
    
    // Mise à jour des timers
    if (rider->attackTimer > 0) rider->attackTimer--;
    if (!rider->canAttack && rider->attackTimer == 0) {
        rider->canAttack = TRUE;
    }
}

// === RENDU ET CULLING ===

void updateAIVisibility(void) {
    u8 i;
    s32 playerWorldZ = trackPosition << 16;
    
    for (i = 0; i < activeRiders; i++) {
        if (!aiRiders[i].active) continue;
        
        // Culling par distance
        s32 distance = abs(aiRiders[i].worldZ - playerWorldZ);
        bool wasVisible = aiRiders[i].visible;
        
        aiRiders[i].visible = (distance < (500 << 16)); // 500 unités max
        
        // Activation/désactivation du sprite
        if (aiRiders[i].visible && !wasVisible) {
            assignSpriteToAI(&aiRiders[i]);
        } else if (!aiRiders[i].visible && wasVisible) {
            releaseSpriteFromAI(&aiRiders[i]);
        }
    }
}

void assignSpriteToAI(AIRider* rider) {
    if (rider->spriteIndex != 0xFF) return; // Déjà assigné
    
    // Configuration du sprite selon le type d'IA
    u16 tileAttr = TILE_ATTR(PAL1, 0, FALSE, FALSE);
    Sprite* sprite = SPR_addSprite(&sprite_ai_bike, rider->x - 8, rider->y - 8, tileAttr);
    
    if (sprite != NULL) {
        // Stockage du pointeur sprite converti en index pour simplification
        rider->spriteIndex = (u8)((u32)sprite & 0xFF);
    } else {
        rider->spriteIndex = 0xFF; // Pas de sprite disponible
    }
}

void releaseSpriteFromAI(AIRider* rider) {
    if (rider->spriteIndex != 0xFF) {
        // Pour SGDK, nous devons passer NULL pour libérer tous les sprites
        // ou gérer manuellement les sprites individuels
        rider->spriteIndex = 0xFF;
    }
}

void renderAIRiders(void) {
    u8 i;
    
    for (i = 0; i < activeRiders; i++) {
        if (!aiRiders[i].active || !aiRiders[i].visible) continue;
        
        AIRider* rider = &aiRiders[i];
        
        // Calcul de la position d'affichage basée sur la profondeur
        s32 playerWorldZ = trackPosition << 16;
        s32 relativeZ = rider->worldZ - playerWorldZ;
        
        // Projection pseudo-3D
        if (relativeZ > 0) {
            // Derrière le joueur - plus petit
            rider->y = 200;
        } else {
            // Devant le joueur - taille normale
            rider->y = 120 + ((-relativeZ) >> 18); // Plus haut si plus loin
        }
        
        // Mise à jour position sprite
        if (rider->spriteIndex != 0xFF) {
            // Pour SGDK, nous ne pouvons pas facilement mettre à jour la position individuellement
            // Il faudrait un système de gestion plus sophistiqué des sprites
            // Pour l'instant, on laisse comme placeholder
            
            // Animation
            updateAIAnimation(rider);
        }
    }
}

void updateAIAnimation(AIRider* rider) {
    rider->animTimer++;
    
    if (rider->animTimer >= 8) { // Change frame toutes les 8 frames
        rider->animTimer = 0;
        rider->animFrame = (rider->animFrame + 1) % 4; // 4 frames d'animation
        
        // Mise à jour de la frame du sprite
        if (rider->spriteIndex != 0xFF) {
            // Pour SGDK, changement de frame nécessite une approche différente
            // Placeholder pour l'instant
        }
    }
}

// === INTERFACE PUBLIQUE ===

void updateAISystem(s16 roadCurve) {
    u8 i;
    
    // Mise à jour de chaque rider actif
    for (i = 0; i < activeRiders; i++) {
        if (!aiRiders[i].active) continue;
        
        updateAIDecisions(&aiRiders[i]);
        updateAIPhysics(&aiRiders[i], roadCurve);
    }
    
    // Gestion des collisions
    checkAICollisions();
    
    // Mise à jour visibilité et culling
    updateAIVisibility();
    
    // Rendu final
    renderAIRiders();
}

void disableAIRider(AIRider* rider) {
    releaseSpriteFromAI(rider);
    rider->active = FALSE;
    rider->visible = FALSE;
    
    // Compactage du tableau (optionnel)
    // Plus simple : marquer comme inactif et réutiliser plus tard
}

u8 findFreeSprite(void) {
    // Version simplifiée pour la compilation
    static u8 nextSprite = 1; // Sprite 0 réservé au joueur
    
    if (nextSprite < 16) { // Max 16 sprites IA simultanés
        return nextSprite++;
    }
    return 0xFF;
}