#include "f.h"
#include "joueur.h"
#include "ennemi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    printf("=== DÉMARRAGE DU JEU PLATEFORMER ===\n");
    
    Jeu jeu;
    initialiser_jeu(&jeu);
    
    // Variables locales pour les inputs
    int gauche = 0, droite = 0, saut = 0, attaque = 0;
    
    Uint32 frameStart;
    int frameTime;
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;
    
    bool quit = false;
    SDL_Event event;
    
    printf("\n🎮 COMMANDES:\n");
    printf("  ← → : Déplacement\n");
    printf("  ↑ : Sauter\n");
    printf("  SPACE : Attaquer\n");
    printf("  P : Pause\n");
    printf("  G : Guide\n");
    printf("  R : Restart\n");
    printf("  ESC : Quitter\n\n");
    
    while (!quit && jeu.running) {
        frameStart = SDL_GetTicks();
        
        // Gestion des événements
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                    
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                        case SDLK_LEFT:
                            gauche = 1;
                            break;
                        case SDLK_RIGHT:
                            droite = 1;
                            break;
                        case SDLK_UP:
                            saut = 1;
                            break;
                        case SDLK_SPACE:
                            attaque = 1;
                            jeu.joueur1.en_combat = 1;
                            break;
                        case SDLK_p:
                            basculerPause(&jeu);
                            break;
                        case SDLK_g:
                            jeu.show_guide = !jeu.show_guide;
                            break;
                        case SDLK_r:
                            initialiser_niveau2(&jeu);
                            jeu.joueur1.x = 100;
                            jeu.joueur1.y = 700;
                            jeu.joueur1.vitesse_y = 0;
                            jeu.joueur1.en_saut = 0;
                            jeu.score = 0;
                            jeu.vies = 3;
                            jeu.tempsEcoule = 0;
                            demarrerTemps(&jeu);
                            break;
                    }
                    break;
                    
                case SDL_KEYUP:
                    switch(event.key.keysym.sym) {
                        case SDLK_LEFT:
                            gauche = 0;
                            break;
                        case SDLK_RIGHT:
                            droite = 0;
                            break;
                        case SDLK_UP:
                            saut = 0;
                            break;
                        case SDLK_SPACE:
                            attaque = 0;
                            break;
                    }
                    break;
            }
        }
        
        if (!jeu.paused) {
            // Mise à jour du temps
            mettreAJourTemps(&jeu);
            
            // Mouvement du joueur
            if (gauche) {
                jeu.joueur1.vitesse_x = -5.0f * jeu.facteur_vitesse_j1;
                jeu.joueur1.direction = -1;
            } else if (droite) {
                jeu.joueur1.vitesse_x = 5.0f * jeu.facteur_vitesse_j1;
                jeu.joueur1.direction = 1;
            } else {
                jeu.joueur1.vitesse_x = 0;
            }
            jeu.joueur1.x += jeu.joueur1.vitesse_x;
            
            // Saut
            if (saut && !jeu.joueur1.en_saut) {
                jeu.joueur1.vitesse_y = -12.0f;
                jeu.joueur1.en_saut = 1;
                saut = 0;
            }
            
            // Physique
            gerer_physique_joueur(&jeu, &jeu.joueur1);
            
            // Mise à jour des entités
            mettre_a_jour_plateformes(&jeu);
            mettre_a_jour_ennemis(jeu.ennemis, jeu.nb_ennemis, jeu.joueur1.x);
            gerer_ralentissement(&jeu);
            gerer_collectibles(&jeu);
            gerer_collisions_ennemis(&jeu);
            
            // Vérification game over
            if (jeu.vies <= 0) {
                printf("\n💀 GAME OVER! Score final: %d\n", jeu.score);
                printf("Appuyez sur R pour rejouer ou ESC pour quitter\n");
                jeu.running = 0;
                break;
            }
        }
        
        // Rendu
        SDL_SetRenderDrawColor(jeu.renderer, 0, 0, 0, 255);
        SDL_RenderClear(jeu.renderer);
        
        affichage_mono(&jeu, jeu.renderer);
        afficherGuideTransparent(&jeu);
        afficherMessagePause(&jeu);
        
        SDL_RenderPresent(jeu.renderer);
        
        // Contrôle du framerate
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }
    
    nettoyer_ressources(&jeu);
    printf("\n=== FIN DU JEU ===\n");
    return 0;
}
