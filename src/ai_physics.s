/* ai_physics.s - Routines assembleur optimisées pour l'IA des concurrents */

.text
.global updateAIPhysicsASM
.global checkCollisionASM
.global sortRidersByDistance

/* Constantes */
MAX_SPEED = 255
MIN_SPEED = 0
HANDLING_SCALE = 4
ACCELERATION_DAMPING = 8

/*
 * Fonction: updateAIPhysicsASM
 * Met à jour la physique d'un rider IA de manière optimisée
 * Paramètres:
 *   a0 = pointeur vers AIRider
 *   d0 = targetX (position X cible)
 *   d1 = roadCurve (courbure actuelle de la route)
 */
updateAIPhysicsASM:
    movem.l d2-d7/a1-a2, -(sp)
    
    /* Chargement des données du rider */
    move.w 0(a0), d2        /* d2 = rider->x */
    move.w 2(a0), d3        /* d3 = rider->y */
    move.w 14(a0), d4       /* d4 = rider->speed */
    move.w 16(a0), d5       /* d5 = rider->maxSpeed */
    move.w 18(a0), d6       /* d6 = rider->acceleration */
    move.w 20(a0), d7       /* d7 = rider->handling */
    
    /* === CALCUL DU MOUVEMENT LATERAL === */
    
    /* Différence entre position actuelle et cible */
    sub.w d2, d0            /* d0 = targetX - currentX */
    
    /* Application du handling (maniabilité) */
    muls.w d7, d0           /* d0 = diff * handling */
    asr.l #8, d0            /* d0 /= 256 (normalisation) */
    
    /* Limitation du mouvement latéral par frame */
    cmp.w #8, d0
    ble.s check_negative_move
    move.w #8, d0
    bra.s apply_lateral_move
    
check_negative_move:
    cmp.w #-8, d0
    bge.s apply_lateral_move
    move.w #-8, d0
    
apply_lateral_move:
    add.w d0, d2            /* Nouvelle position X */
    
    /* === GESTION DE LA VITESSE === */
    
    /* Calcul de l'accélération basée sur la différence de vitesse cible */
    move.w d5, d1           /* d1 = maxSpeed comme vitesse cible */
    sub.w d4, d1            /* d1 = maxSpeed - currentSpeed */
    
    /* Application de l'accélération */
    muls.w d6, d1           /* d1 = speedDiff * acceleration */
    asr.l #ACCELERATION_DAMPING, d1  /* Damping de l'accélération */
    
    add.w d1, d4            /* Nouvelle vitesse */
    
    /* === EFFETS DE LA COURBURE === */
    
    /* La courbure affecte la vitesse (ralentissement dans les virages) */
    move.w d1, d2           /* d2 = roadCurve */
    
    /* Valeur absolue de la courbure */
    tst.w d2
    bpl.s curve_positive
    neg.w d2
curve_positive:
    
    /* Réduction de vitesse proportionnelle à la courbure */
    lsr.w #2, d2            /* d2 = abs(curve) / 4 */
    sub.w d2, d4            /* speed -= curveEffect */
    
    /* === CONTRAINTES DE VITESSE === */
    
    /* Vitesse minimale */
    cmp.w #MIN_SPEED, d4
    bge.s check_max_speed
    move.w #MIN_SPEED, d4
    
check_max_speed:
    /* Vitesse maximale + tolérance pour dépassements temporaires */
    move.w d5, d1
    add.w #20, d1           /* maxSpeed + 20 */
    cmp.w d1, d4
    ble.s speed_ok
    move.w d1, d4
    
speed_ok:
    
    /* === CONTRAINTES POSITION === */
    
    /* Limites de la route (avec marges) */
    cmp.w #70, d2
    bge.s check_right_limit
    move.w #70, d2
    /* Pénalité de vitesse pour sortie de route */
    sub.w #3, d4
    
check_right_limit:
    cmp.w #250, d2
    ble.s position_ok
    move.w #250, d2
    /* Pénalité de vitesse */
    sub.w #3, d4
    
position_ok:
    
    /* === SAUVEGARDE DES RESULTATS === */
    
    move.w d2, 0(a0)        /* rider->x = nouvelle position */
    move.w d4, 14(a0)       /* rider->speed = nouvelle vitesse */
    
    movem.l (sp)+, d2-d7/a1-a2
    rts

/*
 * Fonction: checkCollisionASM
 * Vérifie la collision entre deux points avec un seuil
 * Paramètres:
 *   d0 = x1, d1 = y1 (premier point)
 *   d2 = x2, d3 = y2 (second point)  
 *   d4 = threshold (seuil de collision)
 * Retour:
 *   d0 = 1 si collision, 0 sinon
 */
checkCollisionASM:
    movem.l d5-d6, -(sp)
    
    /* Calcul de la distance en X */
    sub.w d2, d0            /* d0 = x1 - x2 */
    
    /* Valeur absolue */
    tst.w d0
    bpl.s no_collision
    neg.w d0
    
    /* Calcul de la distance en Y */
    sub.w d3, d1            /* d1 = y1 - y2 */
    
    /* Valeur absolue */
    tst.w d1
    bpl.s no_collision
    neg.w d1
    
    /* Vérification de la collision par rapport au seuil */
    cmp.w d4, d0
    ble.s no_collision
    cmp.w d4, d1
    ble.s no_collision
    
    /* Collision détectée */
    move.w #1, d0
    bra.s end_check_collision
    
no_collision:
    /* Pas de collision */
    move.w #0, d0
    
end_check_collision:
    movem.l (sp)+, d5-d6
    rts

/*
 * Fonction: sortRidersByDistance
 * Trie les riders par distance à la cible (X) - Méthode de tri simple
 * Paramètres:
 *   a0 = pointeur vers le début de la liste des riders
 *   d0 = nombre de riders à trier
 */
sortRidersByDistance:
    movem.l d1-d7/a1-a2, -(sp)
    
    /* Vérification du nombre de riders */
    cmp.w #2, d0
    blt.s end_sort
    
    /* Initialisation */
    move.w (a0), d1         /* d1 = première distance */
    move.w #1, d2           /* d2 = index du rider courant */
    move.w d0, d3           /* d3 = nombre de riders restants */
    
tri_riders:
    /* Chargement des distances */
    move.w (a0, d2.w), d4   /* d4 = distance du rider courant */
    
    /* Comparaison avec la distance de référence */
    cmp.w d1, d4
    blt.s rider_plus_loin
    
    /* Échange des riders */
    move.w (a0), d0         /* d0 = première distance */
    move.w d4, (a0)         /* première distance = distance du rider courant */
    move.w d0, (a0, d2.w)   /* distance du rider courant = première distance */
    move.w d4, d1           /* d1 = nouvelle distance de référence */
    
rider_plus_loin:
    /* Rider suivant */
    add.w #2, d2            /* d2++ (prochain rider) */
    sub.w #1, d3            /* d3-- (un rider de moins à trier) */
    cmp.w #1, d3
    bge.s tri_riders
    
end_sort:
    movem.l (sp)+, d1-d7/a1-a2
    rts