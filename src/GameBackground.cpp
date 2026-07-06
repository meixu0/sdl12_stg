#include "GameBackground.h"
const int GameBackground::BG_WIDTH = 544;
const int GameBackground::BG_HEIGHT = 600;
SDL_Surface* GameBackground::stgbg[2] = {NULL, NULL};
GameBackground::GameBackground(){
    //backgroundImage = load_image("level/game_background.jpeg", 544,600);
    bgX = 0.0;
    bgY = 0.0;
    scrollSpeed = 300.0;
}
void GameBackground::background_update(float dt){
    if(stgbg[0] == NULL) {
        stgbg[0] = load_image("res/stg1bg/stg1bg.png", 512, 512);
    }
    if(stgbg[1] == NULL) {
        stgbg[1] = load_image("res/stg1bg/stg5bg.png", 512, 512);
    }
    bgY += scrollSpeed * dt;
    if(bgY >= BG_HEIGHT){
        bgY -= BG_HEIGHT;
    }
}
void GameBackground::background_show(){
    //apply_surface(bgX, (int)bgY - BG_HEIGHT, backgroundImage, screen);
    //apply_surface(bgX, (int)bgY, backgroundImage, screen);
    //todo:背景效果
}