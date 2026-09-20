#ifndef F_H
#define F_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// ================= CONSTANTES =================
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRAVITE 0.8f
#define MAX_PLATEFORMES 100
#define MAX_COINS 100
#define MAX_ENNEMIS 50
#define MAX_ZONES 20

// ================= TYPES =================
typedef enum {
    MODE_MONO,
    MODE_DUO
} ModeAffichage;

typedef enum {
    FIXE,
    MOBILE,
    DESTRUCTIBLE
} TypePlateforme;

typedef struct {
    SDL_Rect pos;
    int actif;
    float vitesse;
    int direction;
    int type;
    int borneMin;
    int borneMax;
    int hits;
} Plateforme;

typedef struct {
    SDL_Rect pos;
    int actif;
    int collecte;
} Coin;

typedef struct {
    SDL_Rect pos;
    int actif;
    float vitesse;
    int direction;
    int hostile;
    int borneMin;
    int borneMax;
} Ennemi;

typedef struct {
    SDL_Rect rect;
    int actif;
} ZoneRalentissement;

typedef struct {
    SDL_Rect pos;
    float velociteY;
    int enSaut;
    int saut;
    int vitesse;
} Joueur;

typedef struct {
    // SDL
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* small_font;
    
    // État du jeu
    int running;
    int paused;
    int show_guide;
    ModeAffichage mode_affichage;
    int niveau_actuel;
    int difficulte;
    
    // Score et vies
    int score;
    int vies;
    
    // Map
    int mapW;
    int mapH;
    int niveau;
    
    // Joueur
    Joueur joueur1;
    Joueur joueur2;  // Non utilisé mais gardé pour compatibilité
    float facteur_vitesse_j1;
    float facteur_vitesse_j2;
    
     int fin_niveau_atteint;
    
    // Éléments du jeu
    Plateforme plats[MAX_PLATEFORMES];
    int nb_plats;
    
    Coin coins[MAX_COINS];
    int nb_coins;
    
    Ennemi ennemis[MAX_ENNEMIS];
    int nb_ennemis;
    
    ZoneRalentissement zones_ralentissement[MAX_ZONES];
    int nb_zones_ralentissement;
    
    // Background
    SDL_Texture* background;  // ✅ AJOUTÉ pour initialiser_background
    SDL_Texture* img;
    
    // Caméra
    SDL_Rect camera_mono;
    
    // Temps
    Uint32 tempsDebut;
    Uint32 tempsPauseTotal;
    float tempsEcoule;
    
} Jeu;

// ================= PROTOTYPES =================

// Initialisation
void initialiser_jeu(Jeu* B);
void initialiser_niveau2(Jeu* B);
void initialiser_background(Jeu* B);

// Gestion du temps
void demarrerTemps(Jeu* B);
void mettreAJourTemps(Jeu* B);
void afficherTemps(Jeu* B);

// Gestion du jeu
void basculerPause(Jeu* B);
void afficherMessagePause(Jeu* B);
void afficherMode(Jeu* B);
void afficherGuideTransparent(Jeu* B);

// Physique
void gerer_physique_joueur(Jeu* B, Joueur* joueur, float facteur_vitesse);
void verifier_collisions_plateformes(Jeu* B, Joueur* joueur);
void mettre_a_jour_plateformes(Jeu* B);
void mettre_a_jour_ennemis(Jeu* B);

// Gameplay
void gerer_collectibles(Jeu* B);
void gerer_ralentissement(Jeu* B);
void gerer_collisions_ennemis(Jeu* B);

// Ajout d'éléments
void ajouter_plateforme(Jeu* B, int x, int y, int w, int h, int type);
void ajouter_coin(Jeu* B, int x, int y);
void ajouter_ennemi(Jeu* B, int x, int y, int borneMin, int borneMax, int vitesse);
void ajouter_zone_ralentissement(Jeu* B, int x, int y, int w, int h);
void liberer_plateformes(Jeu* B);

// Affichage
void dessiner_texte(Jeu* B, const char* texte, int x, int y, SDL_Color couleur, TTF_Font* font);
void dessiner_coeurs(Jeu* B);
void dessiner_coins(Jeu* B, SDL_Renderer* renderer, SDL_Rect camera, SDL_Rect ecran);
void dessiner_ennemis(Jeu* B, SDL_Renderer* renderer, SDL_Rect camera, SDL_Rect ecran);
void dessiner_zones_ralentissement(Jeu* B, SDL_Renderer* renderer, SDL_Rect camera, SDL_Rect ecran);
void afficher_score(Jeu* B);
void affichage_mono(Jeu* B, SDL_Renderer *re);

// Scrolling
void gerer_scrolling(Jeu* B, SDL_Rect* camera);

// Nettoyage
void nettoyer_ressources(Jeu* B);

#endif // F_H
