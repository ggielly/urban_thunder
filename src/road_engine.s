/* road_engine.s - Routines assembleur optimisées pour le rendu pseudo-3D */

.text

/* Constantes VDP */
VDP_CTRL = 0xC00004
VDP_DATA = 0xC00000
PLAN_A_BASE = 0xC000
TILE_USER_INDEX = 0x0001

/* Attributs des tuiles */
TILE_ROAD = TILE_USER_INDEX
TILE_GRASS = TILE_USER_INDEX + 8
PAL0_ATTR = 0x0000
PAL1_ATTR = 0x2000

.global renderRoadStripsASM
.global clearPlanA

/*
 * Fonction: renderRoadStripsASM
 * Paramètres: 
 *   a0 = pointeur vers tableau RoadStrip
 *   d0 = nombre de strips
 */
renderRoadStripsASM:
    movem.l d1-d7/a1-a6, -(sp)  /* Sauvegarde des registres */
    
    move.w d0, d7               /* d7 = compteur de strips */
    subq.w #1, d7               /* Ajustement pour dbra */
    
render_loop:
    /* Récupération des données du strip actuel */
    move.w 0(a0), d1            /* d1 = screenY */
    move.w 2(a0), d2            /* d2 = roadWidth */
    move.w 4(a0), d3            /* d3 = roadXOffset */
    move.w 6(a0), d4            /* d4 = scale */
    
    /* Vérification si on est dans la zone visible */
    cmp.w #224, d1
    bge next_strip
    cmp.w #80, d1               /* Horizon Y */
    blt next_strip
    
    /* Calcul de la position centrale de la route */
    move.w #160, d5             /* Centre écran */
    add.w d3, d5                /* + offset de courbure */
    
    /* Calcul des bords de la route */
    move.w d5, d6
    sub.w d2, d6                /* Bord gauche */
    asr.w #1, d6                /* /2 car roadWidth est le diamètre */
    
    move.w d5, d7
    add.w d2, d7                /* Bord droit */
    asr.w #1, d7                /* /2 */
    
    /* Conversion en coordonnées tuiles */
    asr.w #4, d6                /* /16 pour tuiles */
    move.w d7, d0
    asr.w #4, d0                /* /16 pour tuiles */
    
    /* Limitation aux bords de l'écran */
    tst.w d6
    bpl.s check_right
    moveq #0, d6
check_right:
    cmp.w #39, d0               /* 320/8 = 40 tuiles max */
    ble.s draw_scanline
    move.w #39, d0
    
draw_scanline:
    /* Dessiner la ligne courante */
    move.w d1, d5               /* Y en coordonnées tuiles */
    asr.w #3, d5                /* /8 pour tuiles */
    
    /* Dessiner l'herbe à gauche */
    moveq #0, d2                /* X = 0 */
grass_left_loop:
    cmp.w d6, d2
    bge.s draw_road_section
    
    /* Calcul adresse VRAM pour PLAN_A */
    move.w d5, d4               /* Y */
    lsl.w #6, d4                /* * 64 (largeur plan) */
    add.w a2, d4                /* + X */
    lsl.w #1, d4                /* * 2 (mots) */
    add.w #PLAN_A_BASE, d4
    
    /* Configuration DMA pour l'herbe */
    move.l #((2 << 30) | (0x8F01) | (0x9300) | (0x9400)), VDP_CTRL  /* Commande DMA + incrément 1 */
    move.l #((0x40000000) | (PLAN_A_BASE + (d5 * 128) + (d2 * 2))), VDP_CTRL /* Adresse VRAM */
    move.w #(TILE_GRASS | PAL0_ATTR), VDP_DATA
    
    addq.w #1, a2
    bra.s grass_left_loop
    
draw_road_section:
    /* Dessiner la section de route */
    move.w d6, a2               /* X début route */
road_loop:
    cmp.w d0, a2                /* X fin route */
    bgt.s draw_grass_right
    
    /* Calcul adresse VRAM */
    move.w d5, d4               /* Y */
    lsl.w #6, d4                /* * 64 */
    add.w a2, d4                /* + X */
    lsl.w #1, d4                /* * 2 */
    add.w #PLAN_A_BASE, d4
    
    /* Configuration DMA pour la route */
    move.l #((2 << 30) | (0x8F01) | (0x9300) | (0x9400)), VDP_CTRL  /* Commande DMA + incrément 1 */
    move.l #((0x40000000) | (PLAN_A_BASE + (d5 * 128) + (a2 * 2))), VDP_CTRL /* Adresse VRAM */
    move.w #(TILE_ROAD | PAL0_ATTR), VDP_DATA
    
    addq.w #1, a2
    bra.s road_loop
    
draw_grass_right:
    /* Dessiner l'herbe à droite */
    move.w d0, a2               /* X fin route */
    addq.w #1, a2               /* X début herbe droite */
grass_right_loop:
    cmp.w #40, a2               /* Largeur écran */
    bge.s next_strip
    
    /* Calcul adresse VRAM */
    move.w d5, d4               /* Y */
    lsl.w #6, d4                /* * 64 */
    add.w a2, d4                /* + X */
    lsl.w #1, d4                /* * 2 */
    add.w #PLAN_A_BASE, d4
    
    /* Configuration DMA pour l'herbe */
    move.l #((2 << 30) | (0x8F01) | (0x9300) | (0x9400)), VDP_CTRL  /* Commande DMA + incrément 1 */
    move.l #((0x40000000) | (PLAN_A_BASE + (d5 * 128) + (a2 * 2))), VDP_CTRL /* Adresse VRAM */
    move.w #(TILE_GRASS | PAL0_ATTR), VDP_DATA
    
    addq.w #1, a2
    bra.s grass_right_loop
    
next_strip:
    lea 8(a0), a0               /* Strip suivant (8 octets) */
    dbra d7, render_loop        /* Décrémenter et boucler */
    
    movem.l (sp)+, d1-d7/a1-a6  /* Restauration des registres */
    rts

/*
 * Fonction: clearPlanA
 * Efface rapidement le plan A (zone route)
 */
clearPlanA:
    movem.l d0-d2/a0, -(sp)
    
    /* Configuration pour effacer la zone route (lignes 10-28) */
    move.w #18, d0              /* 18 lignes à effacer */
    subq.w #1, d0               /* Pour dbra */
    
    move.w #10, d1              /* Y début */
    
clear_line_loop:
    move.w #39, d2              /* 40 tuiles par ligne */
    subq.w #1, d2               /* Pour dbra */
    moveq #0, d0                /* X début */
    
clear_tile_loop:
    /* Calcul adresse VRAM */
    move.w d1, d3               /* Y */
    lsl.w #6, d3                /* * 64 */
    add.w d0, d3                /* + X */
    lsl.w #1, d3                /* * 2 */
    add.w #PLAN_A_BASE, d3
    
    /* Écriture tuile vide */
    move.l d3, d4
    ori.l #0x40000000, d4
    move.l d4, VDP_CTRL
    move.w #0, VDP_DATA         /* Tuile 0 = vide */
    
    addq.w #1, a0
    dbra d2, clear_tile_loop
    
    addq.w #1, d1
    dbra d0, clear_line_loop
    
    movem.l (sp)+, d0-d2/a0
    rts

/*
 * Fonction: setHorizontalScroll
 * Paramètres: d0 = line, d1 = offset
 */
.global setHorizontalScroll
setHorizontalScroll:
    movem.l d2-d3, -(sp)
    
    /* Calcul adresse scroll horizontal */
    move.w d0, d2               /* Line */
    lsl.w #2, d2                /* * 4 (PLAN_A + PLAN_B) */
    add.w #0xFC00, d2           /* Base adresse H-scroll */
    
    /* Écriture offset */
    move.l d2, d3
    ori.l #0x40000000, d3       /* Commande écriture VRAM */
    move.l d3, VDP_CTRL
    move.w d1, VDP_DATA         /* Offset PLAN_A */
    move.w #0, VDP_DATA         /* Offset PLAN_B (fixe) */
    
    movem.l (sp)+, d2-d3
    rts