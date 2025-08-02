/*
 * advanced_renderer.c - Moteur de rendu haute performance pour Road Rash Mega Drive
 * 
 * Ce fichier contient un système de rendu optimisé basé sur la technique scanline
 * avec support pour transparence, dithering et effets visuels avancés.
 * 
 * UTILISATION FUTURE:
 * - Intégrer ce système pour remplacer le rendu tilemap basique
 * - Permet des effets 3D plus avancés (polygones, gradients, transparence)
 * - Compatible avec le système de RoadStrip existant
 * 
 * DÉPENDANCES:
 * - SGDK standard
 * - Accès direct VRAM (nécessite désactivation protection VDP)
 * - Fonction doit être compilée avec RAM_CODE pour performance
 */

#include <genesis.h>

// === CONFIGURATION DU MOTEUR ===

// Activer/désactiver les fonctionnalités pour optimiser la taille/performance
#define ENABLE_SEMITRANSPARENCY 1      // Support alpha blending
#define ENABLE_DITHERED_ALPHA 1        // Support transparency par dithering
#define ENABLE_FOG 0                   // Support brouillard (futur)
#define ENABLE_TEXTURE_MAPPING 0       // Support textures (futur)
#define ENABLE_BOUNDS_CHECKING 1       // Vérifications sécurité (debug)
#define ENABLE_VBLANK_PROTECTION 1     // Protection interruptions VBlank

// === STRUCTURES DE DONNÉES ===

// Flags pour les propriétés des polygones
typedef struct {
    u8 has_transparency : 1;    // Utilise alpha blending
    u8 dithered_alpha : 1;      // Utilise dithering pour transparence
    u8 is_textured : 1;         // Utilise texture mapping (futur)
    u8 fog_enabled : 1;         // Affecté par le brouillard (futur)
    u8 z_buffer_test : 1;       // Test de profondeur (futur)
    u8 reserved : 3;            // Bits réservés
} poly_flags_t;

// Structure d'un polygone à rendre
typedef struct {
    u16 color;                  // Couleur 16-bit (format RGB555)
    poly_flags_t flags;         // Propriétés du polygone
    s16 z_depth;               // Profondeur pour z-buffer (futur)
    u8 alpha_level;            // Niveau de transparence 0-255 (futur)
    u16 texture_id;            // ID de texture (futur)
} g_poly_t;

// Structure d'une scanline à rendre
typedef struct {
    u16 *vid_pnt;              // Pointeur début dans VRAM
    u16 *end_pnt;              // Pointeur fin dans VRAM  
    u16 y_line;                // Numéro de ligne Y
    s16 *z_buffer;             // Buffer de profondeur (futur)
    u16 *texture_u;            // Coordonnées texture U (futur)
    u16 *texture_v;            // Coordonnées texture V (futur)
} scanline_tx_t;

// === VARIABLES GLOBALES ===

// Cache pour optimiser les accès VRAM
static u32 vram_cache_start = 0;
static u32 vram_cache_size = 0;

// Statistiques de performance (debug)
static struct {
    u32 pixels_drawn;
    u32 scanlines_processed;
    u32 transparency_operations;
    u16 max_pixels_per_frame;
} render_stats;

// === FONCTIONS UTILITAIRES ===

/*
 * Initialise le système de rendu avancé
 * ATTENTION: Désactive certaines protections VDP pour performance maximale
 */
void init_advanced_renderer(void) {
    // Reset des statistiques
    render_stats.pixels_drawn = 0;
    render_stats.scanlines_processed = 0;
    render_stats.transparency_operations = 0;
    render_stats.max_pixels_per_frame = 0;
    
    // TODO FUTUR: Initialiser z-buffer si activé
    #if ENABLE_Z_BUFFER
    // Allouer buffer de profondeur (320 * 224 * 2 bytes = 143KB)
    // ATTENTION: Dépasse la RAM disponible, nécessite streaming
    #endif
    
    // TODO FUTUR: Initialiser cache texture
    #if ENABLE_TEXTURE_MAPPING
    // init_texture_cache();
    #endif
    
    VDP_drawText("ADVANCED RENDERER INIT", 1, 1);
}

/*
 * Convertit coordonnées écran vers pointeur VRAM
 * ATTENTION: Pas de vérification bounds - assume coordonnées valides
 */
INLINE u16* screen_to_vram_ptr(u16 x, u16 y) {
    #if ENABLE_BOUNDS_CHECKING
    // Vérification des limites écran en mode debug
    if (x >= 320 || y >= 224) {
        return NULL; // Coordonnées invalides
    }
    #endif
    
    // Calcul direct du pointeur VRAM
    // Plan A base address = 0xC000 (par défaut SGDK)
    // Largeur plan = 64 tuiles = 64 words par ligne
    // ATTENTION: Assume que le VDP est configuré pour accès direct
    return (u16*)(VDP_VRAM_ADDR + (y * 64 + (x >> 3)) * 2);
}

/*
 * Fonction principale de rendu scanline optimisée
 * ATTENTION: Doit être compilée avec RAM_CODE pour performance maximale
 */
RAM_CODE void draw_poly_scanline(g_poly_t *poly, scanline_tx_t *scanline) {
    u16 *vid_pnt = scanline->vid_pnt;
    u16 *end_pnt = scanline->end_pnt;
    
    #if ENABLE_BOUNDS_CHECKING
    // Vérification sécurité en mode debug
    if (!vid_pnt || !end_pnt || vid_pnt >= end_pnt) {
        return; // Paramètres invalides
    }
    
    // Vérification limites VRAM
    if ((u32)vid_pnt < VDP_VRAM_ADDR || 
        (u32)end_pnt >= (VDP_VRAM_ADDR + 0x10000)) {
        return; // Hors limites VRAM
    }
    #endif
    
    #if ENABLE_VBLANK_PROTECTION
    // ATTENTION: Protection contre les interruptions VBlank
    // Peut causer du tearing mais évite la corruption VRAM
    u16 vdp_status;
    do {
        vdp_status = VDP_getStatus();
    } while (vdp_status & 0x08); // Attendre fin VBlank
    #endif
    
    // Mise à jour statistiques
    render_stats.scanlines_processed++;
    u32 pixel_count = end_pnt - vid_pnt;
    render_stats.pixels_drawn += pixel_count;
    
    #if ENABLE_FOG || ENABLE_DITHERED_ALPHA
    if (!poly->flags.dithered_alpha) {
    #endif
        
        #if ENABLE_SEMITRANSPARENCY
        if (!poly->flags.has_transparency) {
        #endif
            // === RENDU SOLIDE STANDARD ===
            // Boucle optimisée avec unrolling pour performance maximale
            #pragma GCC unroll 4
            while (vid_pnt < end_pnt) {
                *vid_pnt++ = poly->color;
            }
            
        #if ENABLE_SEMITRANSPARENCY
        } else {
            // === RENDU AVEC ALPHA BLENDING ===
            // ATTENTION: Alpha blending approximatif pour performance
            // Vraie transparence serait trop lente sur 68000
            render_stats.transparency_operations += pixel_count;
            
            #pragma GCC unroll 4
            while (vid_pnt < end_pnt) {
                u16 fb_color = *vid_pnt;
                
                // TODO FUTUR: Alpha blending plus précis
                // Actuellement: addition simple des composantes
                // AMELIORATION: Utiliser table de lookup pour blend précis
                #if 0
                // Version future avec blend précis (plus lent)
                u16 r1 = (fb_color >> 10) & 0x1F;
                u16 g1 = (fb_color >> 5) & 0x1F;
                u16 b1 = fb_color & 0x1F;
                
                u16 r2 = (poly->color >> 10) & 0x1F;
                u16 g2 = (poly->color >> 5) & 0x1F;
                u16 b2 = poly->color & 0x1F;
                
                u16 alpha = poly->alpha_level; // 0-255
                r1 = (r1 * (255 - alpha) + r2 * alpha) >> 8;
                g1 = (g1 * (255 - alpha) + g2 * alpha) >> 8;
                b1 = (b1 * (255 - alpha) + b2 * alpha) >> 8;
                
                *vid_pnt++ = (r1 << 10) | (g1 << 5) | b1;
                #else
                // Version rapide actuelle
                fb_color += 0x404; // Ajoute à chaque composante RGB
                *vid_pnt++ = fb_color;
                #endif
            }
        }
        #endif
        
    #if ENABLE_FOG || ENABLE_DITHERED_ALPHA
    } else {
        // === RENDU AVEC DITHERING (EFFET CHECKERBOARD) ===
        // Technique pour simuler transparence 50% sur hardware limité
        
        #ifdef PC
        // Version PC (pointeurs 64-bit)
        if ((scanline->y_line & 1) != (((u64)vid_pnt >> 1) & 1)) {
        #else
        // Version Mega Drive (pointeurs 32-bit)
        if ((scanline->y_line & 1) != (((u32)vid_pnt >> 1) & 1)) {
        #endif
            vid_pnt++; // Skip premier pixel pour pattern checkerboard
        }
        
        #if ENABLE_SEMITRANSPARENCY
        if (!poly->flags.has_transparency) {
        #endif
            // Dithering sans alpha blending
            #pragma GCC unroll 4
            while (vid_pnt < end_pnt) {
                *vid_pnt = poly->color;
                vid_pnt += 2; // Skip pixel suivant pour pattern
            }
            
        #if ENABLE_SEMITRANSPARENCY
        } else {
            // Dithering avec alpha blending
            #pragma GCC unroll 4
            while (vid_pnt < end_pnt) {
                u16 fb_color = *vid_pnt;
                fb_color += 0x404;
                *vid_pnt = fb_color;
                vid_pnt += 2;
            }
        }
        #endif
    }
    #endif
}

/*
 * Rendu d'un segment de route avec le moteur avancé
 * INTEGRATION: Remplace renderRoadStripsASM() pour effets avancés
 */
void render_road_segment_advanced(RoadStrip* strip, u16 road_color, u16 grass_color) {
    scanline_tx_t scanline;
    g_poly_t road_poly, grass_poly;
    
    // Configuration polygone route
    road_poly.color = road_color;
    road_poly.flags.has_transparency = FALSE;
    road_poly.flags.dithered_alpha = FALSE;
    road_poly.flags.is_textured = FALSE;
    road_poly.flags.fog_enabled = FALSE;
    
    // Configuration polygone herbe
    grass_poly.color = grass_color;
    grass_poly.flags.has_transparency = FALSE;
    grass_poly.flags.dithered_alpha = FALSE;
    
    // Calcul des limites de la route sur cette scanline
    u16 screen_center = 160; // Centre écran
    u16 road_left = screen_center - (strip->roadWidth >> 1) + strip->roadXOffset;
    u16 road_right = screen_center + (strip->roadWidth >> 1) + strip->roadXOffset;
    
    // Contraintes écran
    if (road_left < 0) road_left = 0;
    if (road_right >= 320) road_right = 319;
    
    // Configuration scanline
    scanline.y_line = strip->screenY;
    
    // TODO FUTUR: Rendu herbe gauche
    if (road_left > 0) {
        scanline.vid_pnt = screen_to_vram_ptr(0, strip->screenY);
        scanline.end_pnt = screen_to_vram_ptr(road_left, strip->screenY);
        if (scanline.vid_pnt && scanline.end_pnt) {
            draw_poly_scanline(&grass_poly, &scanline);
        }
    }
    
    // Rendu route
    scanline.vid_pnt = screen_to_vram_ptr(road_left, strip->screenY);
    scanline.end_pnt = screen_to_vram_ptr(road_right, strip->screenY);
    if (scanline.vid_pnt && scanline.end_pnt) {
        draw_poly_scanline(&road_poly, &scanline);
    }
    
    // TODO FUTUR: Rendu herbe droite
    if (road_right < 319) {
        scanline.vid_pnt = screen_to_vram_ptr(road_right + 1, strip->screenY);
        scanline.end_pnt = screen_to_vram_ptr(319, strip->screenY);
        if (scanline.vid_pnt && scanline.end_pnt) {
            draw_poly_scanline(&grass_poly, &scanline);
        }
    }
}

/*
 * Rendu d'effet de brouillard/distance
 * TODO FUTUR: Intégrer dans le système de track segments
 */
#if ENABLE_FOG
void render_fog_effect(u16 y_start, u16 y_end, u16 fog_color, u8 intensity) {
    scanline_tx_t scanline;
    g_poly_t fog_poly;
    
    fog_poly.color = fog_color;
    fog_poly.flags.has_transparency = TRUE;
    fog_poly.flags.dithered_alpha = (intensity < 128);
    fog_poly.alpha_level = intensity;
    
    for (u16 y = y_start; y <= y_end; y++) {
        scanline.y_line = y;
        scanline.vid_pnt = screen_to_vram_ptr(0, y);
        scanline.end_pnt = screen_to_vram_ptr(319, y);
        
        if (scanline.vid_pnt && scanline.end_pnt) {
            draw_poly_scanline(&fog_poly, &scanline);
        }
    }
}
#endif

/*
 * Rendu de particules/effets spéciaux
 * TODO FUTUR: Système de particules pour crashes, poussière, etc.
 */
#if ENABLE_PARTICLE_SYSTEM
typedef struct {
    s16 x, y;           // Position
    s16 vx, vy;         // Vélocité
    u16 color;          // Couleur
    u8 life;            // Durée de vie
    u8 size;            // Taille (1-4 pixels)
} particle_t;

#define MAX_PARTICLES 32
static particle_t particles[MAX_PARTICLES];
static u8 active_particles = 0;

void spawn_particle(s16 x, s16 y, s16 vx, s16 vy, u16 color, u8 life) {
    if (active_particles >= MAX_PARTICLES) return;
    
    particle_t* p = &particles[active_particles++];
    p->x = x;
    p->y = y;
    p->vx = vx;
    p->vy = vy;
    p->color = color;
    p->life = life;
    p->size = 1;
}

void update_and_render_particles(void) {
    for (u8 i = 0; i < active_particles; i++) {
        particle_t* p = &particles[i];
        
        // Mise à jour physique
        p->x += p->vx;
        p->y += p->vy;
        p->vy += 1; // Gravité
        p->life--;
        
        // Rendu si visible
        if (p->life > 0 && p->x >= 0 && p->x < 320 && p->y >= 0 && p->y < 224) {
            u16* pixel_ptr = screen_to_vram_ptr(p->x, p->y);
            if (pixel_ptr) {
                *pixel_ptr = p->color;
                
                // TODO: Rendu multi-pixel selon size
                if (p->size > 1) {
                    // Render 2x2, 3x3 ou 4x4 particle
                }
            }
        }
        
        // Suppression si morte
        if (p->life == 0) {
            // Compactage du tableau
            particles[i] = particles[--active_particles];
            i--; // Retraiter cet index
        }
    }
}
#endif

/*
 * Génération d'effets de vitesse/motion blur
 * TODO FUTUR: Intégrer avec la vitesse du joueur
 */
void render_speed_effect(u16 player_speed) {
    if (player_speed < 3) return; // Pas d'effet à basse vitesse
    
    // Effet de lignes de vitesse sur les côtés
    u16 line_color = 0x7FFF; // Blanc
    u8 alpha = min(player_speed * 20, 255);
    
    scanline_tx_t scanline;
    g_poly_t speed_poly;
    
    speed_poly.color = line_color;
    speed_poly.flags.has_transparency = TRUE;
    speed_poly.flags.dithered_alpha = TRUE;
    speed_poly.alpha_level = alpha;
    
    // Lignes verticales sur les bords
    for (u16 y = 80; y < 224; y += 4) {
        // Ligne gauche
        scanline.y_line = y;
        scanline.vid_pnt = screen_to_vram_ptr(10, y);
        scanline.end_pnt = screen_to_vram_ptr(12, y);
        if (scanline.vid_pnt && scanline.end_pnt) {
            draw_poly_scanline(&speed_poly, &scanline);
        }
        
        // Ligne droite
        scanline.vid_pnt = screen_to_vram_ptr(308, y);
        scanline.end_pnt = screen_to_vram_ptr(310, y);
        if (scanline.vid_pnt && scanline.end_pnt) {
            draw_poly_scanline(&speed_poly, &scanline);
        }
    }
}

/*
 * Fonction de debug pour afficher les statistiques de rendu
 */
void display_render_stats(void) {
    #ifdef DEBUG
    char debug_text[32];
    
    sprintf(debug_text, "PIX:%ld", render_stats.pixels_drawn);
    VDP_drawText(debug_text, 1, 24);
    
    sprintf(debug_text, "SCAN:%ld", render_stats.scanlines_processed);
    VDP_drawText(debug_text, 12, 24);
    
    sprintf(debug_text, "ALPHA:%ld", render_stats.transparency_operations);
    VDP_drawText(debug_text, 24, 24);
    
    // Reset pour la frame suivante
    render_stats.pixels_drawn = 0;
    render_stats.scanlines_processed = 0;
    render_stats.transparency_operations = 0;
    #endif
}

/*
 * Fonction d'intégration avec le moteur Road Rash existant
 * UTILISATION: Remplacer l'appel à renderRoadStripsASM() par ceci
 */
void render_road_strips_advanced(RoadStrip* strips, u16 numStrips) {
    for (u16 i = 0; i < numStrips; i++) {
        RoadStrip* strip = &strips[i];
        
        // Couleurs basées sur la distance (effet de profondeur)
        u16 road_color = 0x0444; // Gris sombre
        u16 grass_color = 0x0240; // Vert
        
        // Variation couleur selon distance (plus clair = plus proche)
        if (strip->scale > 200) {
            road_color = 0x0666; // Plus clair
            grass_color = 0x0360;
        } else if (strip->scale > 150) {
            road_color = 0x0555;
            grass_color = 0x0250;
        }
        
        // Rendu de ce segment
        render_road_segment_advanced(strip, road_color, grass_color);
    }
    
    // TODO FUTUR: Ajouter effets post-processing
    // render_fog_effect(200, 224, 0x7BDE, 64); // Brouillard à l'horizon
    // render_speed_effect(playerSpeed);
    // update_and_render_particles();
    
    // Statistiques debug
    display_render_stats();
}

/*
 * NOTES D'INTÉGRATION FUTURE:
 * 
 * 1. REMPLACEMENT PROGRESSIF:
 *    - Commencer par remplacer clearPlanA() par ce système
 *    - Garder renderRoadStripsASM() en parallèle pour comparaison
 *    - Migration étape par étape pour valider performance
 * 
 * 2. OPTIMISATIONS AVANCÉES:
 *    - Ajouter support DMA pour transferts VRAM rapides
 *    - Implémenter z-buffer pour objets 3D complexes
 *    - Ajouter texture mapping pour routes détaillées
 * 
 * 3. EFFETS VISUELS:
 *    - Système de particules pour crashes/poussière
 *    - Effets météo (pluie, neige) avec alpha blending
 *    - Reflets sur route mouillée avec dithering
 * 
 * 4. INTÉGRATION SGDK:
 *    - Utiliser SPR_addSprite() pour objets complexes
 *    - Garder VDP_drawText() pour UI/debug
 *    - Mixer avec système tilemap existant
 * 
 * 5. PERFORMANCE:
 *    - Profiler avec different niveaux d'optimisation
 *    - Mesurer impact sur frame rate 60 FPS
 *    - Ajuster features selon budget performance
 */