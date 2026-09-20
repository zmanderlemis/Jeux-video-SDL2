#include "f.h"
void dessiner_texte(Jeu* B, const char* texte, int x, int y, SDL_Color couleur, TTF_Font* font) {
    if (!font || !texte) return;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, texte, couleur);
    if (!surface) return;   
    SDL_Texture* texture = SDL_CreateTextureFromSurface(B->renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    
    SDL_RenderCopy(B->renderer, texture, NULL, &rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void ajouter_plateforme(Jeu* B, int x, int y, int w, int h, int type) {
    if (B->nb_plats >= MAX_PLATEFORMES) return;
    
    B->plats[B->nb_plats].pos.x = x;
    B->plats[B->nb_plats].pos.y = y;
    B->plats[B->nb_plats].pos.w = w;
    B->plats[B->nb_plats].pos.h = h;
    B->plats[B->nb_plats].type = type;
    B->plats[B->nb_plats].actif = 1;
    B->plats[B->nb_plats].direction = 1;
    B->plats[B->nb_plats].vitesse = 0;
    B->plats[B->nb_plats].borneMin = 0;
    B->plats[B->nb_plats].borneMax = 0;
    B->plats[B->nb_plats].hits = (type == DESTRUCTIBLE) ? 3 : 0;
    
    B->nb_plats++;
}

void ajouter_coin(Jeu* B, int x, int y) {
    if (B->nb_coins >= MAX_COINS) return;
    
    B->coins[B->nb_coins].pos.x = x;
    B->coins[B->nb_coins].pos.y = y;
    B->coins[B->nb_coins].pos.w = 15;
    B->coins[B->nb_coins].pos.h = 15;
    B->coins[B->nb_coins].actif = 1;
    B->coins[B->nb_coins].collecte = 0;
    
    B->nb_coins++;
}
// ennemi déplace entre borneMin et borneMax
void ajouter_ennemi(Jeu* B, int x, int y, int borneMin, int borneMax, int vitesse) {
    if (B->nb_ennemis >= MAX_ENNEMIS) return;
    
    B->ennemis[B->nb_ennemis].pos.x = x;
    B->ennemis[B->nb_ennemis].pos.y = y;
    B->ennemis[B->nb_ennemis].pos.w = 30;
    B->ennemis[B->nb_ennemis].pos.h = 50;
    B->ennemis[B->nb_ennemis].vitesse = vitesse;
    B->ennemis[B->nb_ennemis].direction = 1;
    B->ennemis[B->nb_ennemis].actif = 1;
    B->ennemis[B->nb_ennemis].hostile = 0;
    B->ennemis[B->nb_ennemis].borneMin = borneMin;
    B->ennemis[B->nb_ennemis].borneMax = borneMax;
    
    B->nb_ennemis++;
}

void ajouter_zone_ralentissement(Jeu* B, int x, int y, int w, int h) {
    if (B->nb_zones_ralentissement >= MAX_ZONES) return;
    
    B->zones_ralentissement[B->nb_zones_ralentissement].rect.x = x;
    B->zones_ralentissement[B->nb_zones_ralentissement].rect.y = y;
    B->zones_ralentissement[B->nb_zones_ralentissement].rect.w = w;
    B->zones_ralentissement[B->nb_zones_ralentissement].rect.h = h;
    B->zones_ralentissement[B->nb_zones_ralentissement].actif = 1;
    
    B->nb_zones_ralentissement++;
}

void liberer_plateformes(Jeu* B) {
    B->nb_plats = 0;
}

void mettre_a_jour_plateformes(Jeu *B) {
    if (B->paused) return;
    
    for (int i = 0; i < B->nb_plats; i++) {
        Plateforme *p = &B->plats[i];
        if (!p->actif || p->type != MOBILE) continue;
        
        p->pos.x += p->vitesse * p->direction;
        
        if (p->pos.x <= p->borneMin) {
            p->pos.x = p->borneMin;
            p->direction = 1;
        }
        if (p->pos.x + p->pos.w >= p->borneMax) {
            p->pos.x = p->borneMax - p->pos.w;
            p->direction = -1;
        }
    }
}
// ================= enemis ===============
//gère leur comportement.
void mettre_a_jour_ennemis(Jeu* B) {
    if (B->paused) return;
    
    for (int i = 0; i < B->nb_ennemis; i++) {
        Ennemi* e = &B->ennemis[i];
        if (!e->actif) continue;
        
        e->pos.x += e->vitesse * e->direction;
        
        if (e->pos.x <= e->borneMin) {
            e->pos.x = e->borneMin;
            e->direction = 1;
        }
        if (e->pos.x + e->pos.w >= e->borneMax) {
            e->pos.x = e->borneMax - e->pos.w;
            e->direction = -1;
        }
        
        SDL_Rect* joueur_pos = &B->joueur1.pos;
        int distance = abs(joueur_pos->x - e->pos.x);
        
        if (distance < 100 && !e->hostile) {
            e->hostile = 1;
        }
        
        if (e->hostile) {
            if (joueur_pos->x > e->pos.x) {
                e->pos.x += e->vitesse;
            } else if (joueur_pos->x < e->pos.x) {
                e->pos.x -= e->vitesse;
            }
        }
    }
}
//Applique l'effet des zones bleues
void gerer_ralentissement(Jeu* B) {
    if (B->paused) return;
    
    B->facteur_vitesse_j1 = 1.0f;
    
    for (int i = 0; i < B->nb_zones_ralentissement; i++) {
        ZoneRalentissement* zr = &B->zones_ralentissement[i];
        if (!zr->actif) continue;
        
        SDL_Rect joueur_rect = {B->joueur1.pos.x, B->joueur1.pos.y, 30, 50};
        if (SDL_HasIntersection(&joueur_rect, &zr->rect)) {
            B->facteur_vitesse_j1 = 0.4f;
        }
    }
}
//Gère la collecte des pièces.
void gerer_collectibles(Jeu* B) {
    if (B->paused) return;
    
    for (int i = 0; i < B->nb_coins; i++) {
        if (!B->coins[i].actif || B->coins[i].collecte) continue;
        
        SDL_Rect joueur_rect = {B->joueur1.pos.x, B->joueur1.pos.y, 30, 50};
        
        if (SDL_HasIntersection(&joueur_rect, &B->coins[i].pos)) {
            B->coins[i].collecte = 1;
            B->coins[i].actif = 0;
            B->score += 10;
            printf("💰 Coin collecté! Score: %d\n", B->score);
        }
    }
}

void gerer_collisions_ennemis(Jeu* B) {
    if (B->paused) return;
    
    for (int i = 0; i < B->nb_ennemis; i++) {
        Ennemi* e = &B->ennemis[i];
        if (!e->actif) continue;
        
        SDL_Rect joueur_rect = {B->joueur1.pos.x, B->joueur1.pos.y, 30, 50};
        
        if (SDL_HasIntersection(&joueur_rect, &e->pos)) {
            B->vies--;
            printf("💔 Vie perdue! Vies restantes: %d\n", B->vies);
            
            B->joueur1.pos.x = 100;
            B->joueur1.pos.y = 700;
            B->joueur1.velociteY = 0;
            B->joueur1.enSaut = 0;
            
            for (int j = 0; j < B->nb_ennemis; j++) {
                B->ennemis[j].hostile = 0;
            }
            
            if (B->vies <= 0) {
                printf("💀 GAME OVER! Score final: %d\n", B->score);
                B->running = 0;
            }
            break;
        }
    }
}

void dessiner_coeurs(Jeu* B) {
    SDL_Color rouge = {255, 0, 0, 255};
    char coeur[10];
    
    for (int i = 0; i < B->vies; i++) {
        sprintf(coeur, "❤");
        dessiner_texte(B, coeur, 10 + (i * 35), 10, rouge, B->small_font);
    }
}

void dessiner_coins(Jeu* B, SDL_Renderer* renderer, SDL_Rect camera, SDL_Rect ecran) {
    for (int i = 0; i < B->nb_coins; i++) {
        if (!B->coins[i].actif || B->coins[i].collecte) continue;
        
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        SDL_Rect d = {
            B->coins[i].pos.x - camera.x + ecran.x,
            B->coins[i].pos.y - camera.y + ecran.y,
            B->coins[i].pos.w,
            B->coins[i].pos.h
        };
        SDL_RenderFillRect(renderer, &d);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &d);
    }
}

void dessiner_ennemis(Jeu* B, SDL_Renderer* renderer, SDL_Rect camera, SDL_Rect ecran) {
    for (int i = 0; i < B->nb_ennemis; i++) {
        Ennemi* e = &B->ennemis[i];
        if (!e->actif) continue;
        
        if (e->hostile) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        }
        
        SDL_Rect d = {
            e->pos.x - camera.x + ecran.x,
            e->pos.y - camera.y + ecran.y,
            e->pos.w,
            e->pos.h
        };
        SDL_RenderFillRect(renderer, &d);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &d);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect oeil_gauche = {d.x + 5, d.y + 10, 5, 5};
        SDL_Rect oeil_droit = {d.x + d.w - 10, d.y + 10, 5, 5};
        SDL_RenderFillRect(renderer, &oeil_gauche);
        SDL_RenderFillRect(renderer, &oeil_droit);
    }
}

void dessiner_zones_ralentissement(Jeu* B, SDL_Renderer* renderer, SDL_Rect camera, SDL_Rect ecran) {
    for (int i = 0; i < B->nb_zones_ralentissement; i++) {
        ZoneRalentissement* zr = &B->zones_ralentissement[i];
        if (!zr->actif) continue;
        
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 100, 255, 100);
        
        SDL_Rect d = {
            zr->rect.x - camera.x + ecran.x,
            zr->rect.y - camera.y + ecran.y,
            zr->rect.w,
            zr->rect.h
        };
        SDL_RenderFillRect(renderer, &d);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &d);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        int centre_x = d.x + d.w / 2;
        int centre_y = d.y + d.h / 2;
        
        for (int angle = 0; angle < 360; angle += 30) {
            int x = centre_x + (int)(12 * cos(angle * M_PI / 180));
            int y = centre_y + (int)(12 * sin(angle * M_PI / 180));
            SDL_RenderDrawPoint(renderer, x, y);
        }
        
        SDL_RenderDrawLine(renderer, centre_x, centre_y, centre_x + 8, centre_y);
        SDL_RenderDrawLine(renderer, centre_x, centre_y, centre_x, centre_y - 8);
        
        SDL_Color blanc = {255, 255, 255, 255};
        dessiner_texte(B, "⏱️", d.x + d.w/2 - 10, d.y + d.h/2 - 10, blanc, B->small_font);
    }
}
// ================= physique=================
//Gère la gravité et les collisions avec plateformes
void gerer_physique_joueur(Jeu* B, Joueur* joueur, float facteur_vitesse) {
    (void)facteur_vitesse;
    
    if (B->paused) return;
    
    joueur->velociteY += GRAVITE;
    joueur->pos.y += joueur->velociteY;
    
    int sur_plateforme = 0;
    for (int i = 0; i < B->nb_plats; i++) {
        Plateforme* p = &B->plats[i];
        if (!p->actif) continue;
        
        if (joueur->velociteY >= 0 &&
            joueur->pos.y + joueur->pos.h <= p->pos.y + 20 &&
            joueur->pos.y + joueur->pos.h + joueur->velociteY >= p->pos.y &&
            joueur->pos.x + joueur->pos.w > p->pos.x &&
            joueur->pos.x < p->pos.x + p->pos.w) {
            
            joueur->pos.y = p->pos.y - joueur->pos.h;
            joueur->velociteY = 0;
            joueur->enSaut = 0;
            sur_plateforme = 1;
            break;
        }
    }
    
    if (!sur_plateforme && joueur->pos.y < 750) {
        joueur->enSaut = 1;
    }
    
    if (joueur->pos.x < 0) joueur->pos.x = 0;
    if (joueur->pos.x > B->mapW - joueur->pos.w) joueur->pos.x = B->mapW - joueur->pos.w;
    if (joueur->pos.y > 750) {
        joueur->pos.y = 700;
        joueur->velociteY = 0;
        joueur->enSaut = 0;
    }
}

        
// ==============niveau =================
void initialiser_niveau2(Jeu* B) {
    printf("\n=== 🌟 NIVEAU 2 : DÉFI MOYEN 🌟 ===\n");
    printf("Plateformes mobiles | Ennemis hostiles | Ralentissement temporel\n\n");
    
    B->mapW = 2500;
    B->mapH = 800;
    B->niveau = 2;
    
    B->nb_plats = 0;
    B->nb_ennemis = 0;
    B->nb_zones_ralentissement = 0;
    B->nb_coins = 0;
    
    // PLATEFORMES FIXES
    ajouter_plateforme(B, 0, 750, 2500, 50, FIXE);
    ajouter_plateforme(B, 100, 650, 200, 20, FIXE);
    ajouter_plateforme(B, 500, 600, 150, 20, FIXE);
    ajouter_plateforme(B, 900, 550, 180, 20, FIXE);
    ajouter_plateforme(B, 1300, 500, 200, 20, FIXE);
    ajouter_plateforme(B, 1700, 450, 150, 20, FIXE);
    ajouter_plateforme(B, 2100, 400, 150, 20, FIXE);
    
    // PLATEFORMES MOBILES
    Plateforme* p;
    
    p = &B->plats[B->nb_plats];
    ajouter_plateforme(B, 300, 550, 120, 20, MOBILE);
    p->vitesse = 3;
    p->direction = 1;
    p->borneMin = 200;
    p->borneMax = 700;
    
    p = &B->plats[B->nb_plats];
    ajouter_plateforme(B, 800, 500, 120, 20, MOBILE);
    p->vitesse = 4;
    p->direction = -1;
    p->borneMin = 700;
    p->borneMax = 1200;
    
    p = &B->plats[B->nb_plats];
    ajouter_plateforme(B, 1400, 400, 100, 20, MOBILE);
    p->vitesse = 5;
    p->direction = 1;
    p->borneMin = 1300;
    p->borneMax = 1900;
    
    // ZONES DE RALENTISSEMENT
    ajouter_zone_ralentissement(B, 600, 550, 150, 100);
    ajouter_zone_ralentissement(B, 1200, 450, 150, 100);
    ajouter_zone_ralentissement(B, 1800, 390, 150, 100);
    
    // ENNEMIS
    ajouter_ennemi(B, 400, 650, 300, 700, 2);
    ajouter_ennemi(B, 800, 600, 650, 1000, 2);
    ajouter_ennemi(B, 1300, 500, 1150, 1550, 2);
    ajouter_ennemi(B, 1900, 400, 1850, 2200, 2);
    
    // COLLECTIBLES
    ajouter_coin(B, 200, 700);
    ajouter_coin(B, 400, 600);
    ajouter_coin(B, 650, 540);
    ajouter_coin(B, 850, 490);
    ajouter_coin(B, 1050, 490);
    ajouter_coin(B, 1250, 440);
    ajouter_coin(B, 1450, 390);
    ajouter_coin(B, 1650, 390);
    ajouter_coin(B, 1850, 340);
    ajouter_coin(B, 2050, 340);
    ajouter_coin(B, 2250, 340);
    
    printf("✅ Niveau initialisé:\n");
    printf("   - %d plateformes (3 mobiles)\n", B->nb_plats);
    printf("   - %d ennemis\n", B->nb_ennemis);
    printf("   - %d zones de ralentissement\n", B->nb_zones_ralentissement);
    printf("   - %d pièces à collecter\n\n", B->nb_coins);
}



// ================= score=================
void afficher_score(Jeu* B) {
    char score_texte[50];
    sprintf(score_texte, "💰 %d", B->score);
    SDL_Color couleur_or = {255, 215, 0, 255};
    dessiner_texte(B, score_texte, SCREEN_WIDTH - 100, 10, couleur_or, B->font);
    
    char niveau_texte[50];
    sprintf(niveau_texte, "🌟 NIVEAU %d", B->niveau_actuel);
    SDL_Color couleur_cyan = {0, 255, 255, 255};
    dessiner_texte(B, niveau_texte, SCREEN_WIDTH - 120, 45, couleur_cyan, B->small_font);
}
// =================mono =================
void affichage_mono(Jeu* B, SDL_Renderer *re) {

    gerer_scrolling(B, &B->camera_mono); 
    
    if (B->background) {
        SDL_Rect src = {B->camera_mono.x, B->camera_mono.y, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_Rect dst = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(re, B->background, &src, &dst);
    } else {
        // Dégradé par défaut
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            int color = 30 + (i * 30 / SCREEN_HEIGHT);
            SDL_SetRenderDrawColor(re, 20, 20, color, 255);
            SDL_RenderDrawLine(re, 0, i, SCREEN_WIDTH, i);
        }
    }
    SDL_Rect ecran_plein = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    dessiner_zones_ralentissement(B, re, B->camera_mono, ecran_plein);
    dessiner_coins(B, re, B->camera_mono, ecran_plein);
    dessiner_ennemis(B, re, B->camera_mono, ecran_plein);
    
    for (int i = 0; i < B->nb_plats; i++) {
        if (!B->plats[i].actif) continue;
        
        switch (B->plats[i].type) {
            case FIXE:
                SDL_SetRenderDrawColor(re, 139, 69, 19, 255);
                break;
            case MOBILE:
                SDL_SetRenderDrawColor(re, 0, 0, 255, 255);
                break;
            case DESTRUCTIBLE:
                SDL_SetRenderDrawColor(re, 255, 0, 0, 255);
                break;
        }
        
        SDL_Rect d = {
            B->plats[i].pos.x - B->camera_mono.x,
            B->plats[i].pos.y - B->camera_mono.y,
            B->plats[i].pos.w,
            B->plats[i].pos.h
        };
        SDL_RenderFillRect(re, &d);
        SDL_SetRenderDrawColor(re, 0, 0, 0, 255);
        SDL_RenderDrawRect(re, &d);
    }
    
    SDL_SetRenderDrawColor(re, 0, 255, 0, 255);
    SDL_Rect joueur = {
        B->joueur1.pos.x - B->camera_mono.x,
        B->joueur1.pos.y - B->camera_mono.y,
        30, 50
    };
    SDL_RenderFillRect(re, &joueur);
    SDL_SetRenderDrawColor(re, 0, 100, 0, 255);
    SDL_RenderDrawRect(re, &joueur);
    
    dessiner_coeurs(B);
    afficher_score(B);
    afficherTemps(B);
    
    if (B->facteur_vitesse_j1 < 1.0f) {
        SDL_Color jaune = {255, 255, 0, 255};
        dessiner_texte(B, "⏱️ RALENTI !", SCREEN_WIDTH/2 - 50, 10, jaune, B->small_font);
    }
}
// ================= guide=================
void afficherGuideTransparent(Jeu* B) {
    if (!B->show_guide) return;
    
    SDL_SetRenderDrawBlendMode(B->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(B->renderer, 0, 0, 0, 220);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(B->renderer, &overlay);
    
    SDL_Color jaune = {255, 255, 0, 255};
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color cyan = {0, 255, 255, 255};
    SDL_Color vert = {0, 255, 0, 255};
    
    int y = 80;
    dessiner_texte(B, "=== GUIDE DU NIVEAU 2 ===", SCREEN_WIDTH/2 - 130, y, jaune, B->font);
    y += 50;
    
    dessiner_texte(B, "COMMANDES:", SCREEN_WIDTH/2 - 60, y, cyan, B->small_font);
    y += 30;
    dessiner_texte(B, "← →  : Deplacement", SCREEN_WIDTH/2 - 100, y, blanc, B->small_font);
    y += 25;
    dessiner_texte(B, "↑    : Sauter", SCREEN_WIDTH/2 - 100, y, blanc, B->small_font);
    y += 25;
    dessiner_texte(B, "P    : Pause", SCREEN_WIDTH/2 - 100, y, blanc, B->small_font);
    y += 25;
    dessiner_texte(B, "G    : Guide", SCREEN_WIDTH/2 - 100, y, blanc, B->small_font);
    y += 40;
    
    dessiner_texte(B, "OBJECTIFS:", SCREEN_WIDTH/2 - 60, y, cyan, B->small_font);
    y += 30;
    dessiner_texte(B, "💰 Collectez 11 pieces d'or (10 pts chacune)", SCREEN_WIDTH/2 - 180, y, vert, B->small_font);
    y += 25;
    dessiner_texte(B, "❤️ 3 cœurs de vie - Perdez une vie au contact ennemi", SCREEN_WIDTH/2 - 180, y, vert, B->small_font);
    y += 25;
    dessiner_texte(B, "⏱️ Zones bleues = ralentissement temporaire", SCREEN_WIDTH/2 - 180, y, vert, B->small_font);
    y += 25;
    dessiner_texte(B, "🔵 Plateformes bleues = mobiles", SCREEN_WIDTH/2 - 180, y, vert, B->small_font);
    y += 40;
    
    dessiner_texte(B, "Appuyez sur G pour fermer", SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT - 50, jaune, B->small_font);
}
// ================= SCROLLING =================

void gerer_scrolling(Jeu* B, SDL_Rect* camera)
{
    if (!B || !camera) return;
    // Centrer la caméra sur le joueur
    camera->x = B->joueur1.pos.x + B->joueur1.pos.w/2 - SCREEN_WIDTH/2;
    camera->y = B->joueur1.pos.y + B->joueur1.pos.h/2 - SCREEN_HEIGHT/2;
    
    // Limiter la caméra aux limites de la carte
    if (camera->x < 0) camera->x = 0;
    if (camera->y < 0) camera->y = 0;
    if (camera->x > B->mapW - SCREEN_WIDTH) camera->x = B->mapW - SCREEN_WIDTH;
    if (camera->y > B->mapH - SCREEN_HEIGHT) camera->y = B->mapH - SCREEN_HEIGHT;
}

void initialiser_background(Jeu* B)
{
    if (!B || !B->renderer) return;
    
    // Essayer de charger l'image
    B->background = IMG_LoadTexture(B->renderer, "./background.png");
    
    if (!B->background)
    {
        printf("Erreur background: %s\n", IMG_GetError());
        printf("Cr�ation d'un fond par d�faut...\n");
        
        // Créer une texture de fond par défaut de la taille de la carte
        SDL_Surface* surface = SDL_CreateRGBSurface(0, B->mapW, B->mapH, 32, 0, 0, 0, 0);
        
        if (surface) {
            // Remplir avec un dégradé bleu
            for (int y = 0; y < B->mapH; y++) {
                int color = 100 + (y * 155 / B->mapH);
                Uint32 pixel = SDL_MapRGB(surface->format, 50, 100, color);
                for (int x = 0; x < B->mapW; x++) {
                    ((Uint32*)surface->pixels)[y * B->mapW + x] = pixel;
                }
            }
            B->background = SDL_CreateTextureFromSurface(B->renderer, surface);
            SDL_FreeSurface(surface);
        }
    }
}

// ================= TEMPS =================

void demarrerTemps(Jeu* B)
{
    B->tempsDebut = SDL_GetTicks();
    B->tempsPauseTotal = 0;
    B->tempsEcoule = 0;
}

void mettreAJourTemps(Jeu* B)
{
    Uint32 maintenant = SDL_GetTicks();
    Uint32 total = maintenant - B->tempsDebut - B->tempsPauseTotal;
    B->tempsEcoule = total / 1000.0f;
}

void afficherTemps(Jeu* B)
{
    int sec = (int)B->tempsEcoule;
    int min = sec / 60;
    sec %= 60;

    char txt[50];
    sprintf(txt, "Temps: %02d:%02d", min, sec);

    SDL_Color couleur_blanche = {255, 255, 255, 255};
    // Afficher en haut à droite avec les autres informations
    dessiner_texte(B, txt, SCREEN_WIDTH - 150, 80, couleur_blanche, B->small_font);
}
void basculerPause(Jeu* B) {
    B->paused = !B->paused;
    printf("%s\n", B->paused ? "⏸️ JEU EN PAUSE" : "▶️ JEU REPRIS");
}

void afficherMessagePause(Jeu* B) {
    if (!B->paused) return;
    
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color jaune = {255, 255, 0, 255};
    
    SDL_SetRenderDrawBlendMode(B->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(B->renderer, 0, 0, 0, 200);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(B->renderer, &overlay);
    
    dessiner_texte(B, "⏸️ PAUSE ⏸️", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 - 40, jaune, B->font);
    dessiner_texte(B, "Appuyez sur P pour continuer", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2, blanc, B->small_font);
    dessiner_texte(B, "ESC pour quitter", SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 + 40, blanc, B->small_font);
}
// ==============initialiser jeu =================
void initialiser_jeu(Jeu* B) {
    memset(B, 0, sizeof(Jeu));
    
    // SDL Initialization (identique à votre code original)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("❌ Erreur SDL: %s\n", SDL_GetError());
        exit(1);
    }
    
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        printf("❌ Erreur SDL_image: %s\n", IMG_GetError());
        exit(1);
    }
    
    if (TTF_Init() < 0) {
        printf("❌ Erreur TTF: %s\n", TTF_GetError());
        exit(1);
    }
    
    B->window = SDL_CreateWindow("🏆 PLATEFORMER - NIVEAU 2 🏆", 
                                    SDL_WINDOWPOS_CENTERED, 
                                    SDL_WINDOWPOS_CENTERED,
                                    SCREEN_WIDTH, SCREEN_HEIGHT, 
                                    SDL_WINDOW_SHOWN);
    if (!B->window) {
        printf("❌ Erreur fenetre: %s\n", SDL_GetError());
        exit(1);
    }
    
    B->renderer = SDL_CreateRenderer(B->window, -1, SDL_RENDERER_ACCELERATED);
    if (!B->renderer) {
        printf("❌ Erreur renderer: %s\n", SDL_GetError());
        exit(1);
    }
    
    B->font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-M.ttf", 28);
    if (!B->font) {
        B->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28);
    }
    B->small_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18);
    if (!B->small_font && B->font) {  
        B->small_font = B->font;
    }
    
    // ✅ AJOUTS NÉCESSAIRES
    B->niveau_actuel = 2;
    B->difficulte = 2;
    B->running = 1;
    B->paused = 0;
    B->show_guide = 0;
    B->mode_affichage = MODE_MONO;
    B->fin_niveau_atteint = 0;  // ← CRUCIAL
    B->score = 0;
    B->vies = 3;
    
    B->joueur1.pos.x = 100;
    B->joueur1.pos.y = 700;
    B->joueur1.pos.w = 30;
    B->joueur1.pos.h = 50;
    B->joueur1.vitesse = 5;
    B->joueur1.saut = 0;
    B->joueur1.enSaut = 0;
    B->joueur1.velociteY = 0;
    
    // ✅ Initialisation de joueur2 (même si non utilisé)
    B->joueur2 = B->joueur1;
    B->joueur2.pos.x = 150;
    
    B->facteur_vitesse_j1 = 1.0f;
    B->facteur_vitesse_j2 = 1.0f;
    
    // ✅ Initialisation de la caméra
    B->camera_mono = (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    
    B->background = NULL;
    B->img = NULL;
 
    initialiser_niveau2(B);
    initialiser_background(B); 
    demarrerTemps(B);
    
    printf("\n✅ JEU INITIALISE - NIVEAU 2\n");
    printf("🎮 Utilisez les fleches pour vous deplacer\n");
    printf("💰 Collectez les pieces, evitez les ennemis!\n");
    printf("⏱️ Les zones bleues ralentissent le temps\n\n");
}

void nettoyer_ressources(Jeu* B) {
   
    if (!B) {
        printf("⚠️ Pointeur Jeu NULL, nettoyage ignoré\n");
        return;
    }
    
    printf("\n🧹 Nettoyage des ressources en cours...\n");
    
    // 1. Réinitialiser les données du jeu
    liberer_plateformes(B);
    B->nb_coins = 0;
    B->nb_ennemis = 0;
    B->nb_zones_ralentissement = 0;
       // Fermer la connexion Arduino
    if (B->use_arduino) {
        arduino_close(&B->arduino);
    }
    
    // 2. Détruire les textures SDL
    if (B->img) {
        SDL_DestroyTexture(B->img);
        B->img = NULL;
    }
    
    if (B->background) {
        SDL_DestroyTexture(B->background);
        B->background = NULL;
    }
    
    // 3. Fermer les polices TTF
    if (B->font) {
        TTF_CloseFont(B->font);
        B->font = NULL;
    }
    
    if (B->small_font && B->small_font != B->font) {
        TTF_CloseFont(B->small_font);
        B->small_font = NULL;
    }
    
    // 4. Détruire le renderer
    if (B->renderer) {
        SDL_DestroyRenderer(B->renderer);
        B->renderer = NULL;
    }
    
    // 5. Détruire la fenêtre
    if (B->window) {
        SDL_DestroyWindow(B->window);
        B->window = NULL;
    }
    
    // 6. Quitter les sous-systèmes SDL
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    printf("✅ Nettoyage terminé avec succès\n");
}
