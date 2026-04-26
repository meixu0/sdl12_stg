#include "GameBackground.h"
const int GameBackground::BG_WIDTH = 544;
const int GameBackground::BG_HEIGHT = 600;
SDL_Surface* GameBackground::backgroundImage = NULL;
GameBackground::GameBackground(){
    backgroundImage = NULL;
    backgroundImage = load_image("level/game_background.jpeg", 544,600);
    bgX = 0.0;
    bgY = 0.0;
}
void GameBackground::background_update(){
    if(backgroundImage == NULL) return;
    bgY += 5;
    if(bgY >= BG_HEIGHT){
        bgY -= BG_HEIGHT;
    }
}
void GameBackground::background_show(){
    apply_surface(bgX, bgY - BG_HEIGHT, backgroundImage, screen);
    apply_surface(bgX, bgY, backgroundImage, screen);
}