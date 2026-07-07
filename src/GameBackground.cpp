#include "GameBackground.h"
const int GameBackground::BG_WIDTH = 544;
const int GameBackground::BG_HEIGHT = 600;
const int GameBackground::BG_IMG_H = 544;
SDL_Surface* GameBackground::stgbg[2] = {NULL, NULL};
SDL_Surface* GameBackground::stgbgLowestLayer[2] = {NULL, NULL};

const DecorPair GameBackground::decorPairs[4] = {
    {  0,   0, 416},
    {110,  25, 390},
    {210, -15, 430},
    {350,  45, 380},
};

GameBackground::GameBackground(){
    bgX = 0.0;
    bgY = 0.0;
    bgLayerY = 0.0;
    scrollSpeed = 300.0;
}

void GameBackground::background_update(float dt, int currentStage){
    if(stgbg[0] == NULL) {
        stgbg[0] = load_image("res/stg1bg/stg1bg.png", 512, 512);
    }
    if(stgbg[1] == NULL) {
        stgbg[1] = load_image("res/stg1bg/stg5bg.png", 512, 512);
    }
    if(stgbgLowestLayer[0] == NULL) {
        stgbgLowestLayer[0] = load_sprite("res/stg1bg/stg1bg.png", 0, 0, 256, 256, 544, 544);
    }
    if(stgbgLowestLayer[1] == NULL) {
        stgbgLowestLayer[1] = load_sprite("res/stg1bg/stg5bg.png", 0, 0, 256, 256, 544, 544);
    }

    bgY += scrollSpeed * dt;
    if(bgY >= BG_IMG_H){
        bgY -= BG_IMG_H;
    }

    bgLayerY += scrollSpeed * dt;
    if(bgLayerY >= BG_HEIGHT){
        bgLayerY -= BG_HEIGHT;
    }
}

void GameBackground::background_show(int currentStage){
    if(stgbgLowestLayer[0] == NULL || stgbgLowestLayer[1] == NULL) return;
    if(stgbg[0] == NULL || stgbg[1] == NULL) return;

    int idx = currentStage - 1;
    SDL_Rect bgSrc = {0, 0, BG_IMG_H, BG_IMG_H};
    int y0 = (int)bgY;
    SDL_Rect bgDest1 = {0, y0, 0, 0};
    SDL_Rect bgDest2 = {0, y0 - BG_IMG_H, 0, 0};
    SDL_Rect bgDest3 = {0, y0 + BG_IMG_H, 0, 0};
    SDL_BlitSurface(stgbgLowestLayer[idx], &bgSrc, screen, &bgDest1);
    SDL_BlitSurface(stgbgLowestLayer[idx], &bgSrc, screen, &bgDest2);
    SDL_BlitSurface(stgbgLowestLayer[idx], &bgSrc, screen, &bgDest3);
    SDL_Rect layerLeftSrc  = {256, 0, 128, 128};
    SDL_Rect layerRightSrc = {384, 0, 128, 128};
    const int WRAP_H = BG_HEIGHT;

    for(int i = 0; i < 4; i++){
        int rawY = (int)bgLayerY + decorPairs[i].yOffset;
        int y = rawY % WRAP_H;
        SDL_Rect leftDest1  = {decorPairs[i].leftX,  y, 0, 0};
        SDL_Rect leftDest2  = {decorPairs[i].leftX,  y - WRAP_H, 0, 0};
        SDL_Rect rightDest1 = {decorPairs[i].rightX, y, 0, 0};
        SDL_Rect rightDest2 = {decorPairs[i].rightX, y - WRAP_H, 0, 0};

        SDL_BlitSurface(stgbg[idx], &layerLeftSrc,  screen, &leftDest1);
        SDL_BlitSurface(stgbg[idx], &layerLeftSrc,  screen, &leftDest2);
        SDL_BlitSurface(stgbg[idx], &layerRightSrc, screen, &rightDest1);
        SDL_BlitSurface(stgbg[idx], &layerRightSrc, screen, &rightDest2);
    }
}
