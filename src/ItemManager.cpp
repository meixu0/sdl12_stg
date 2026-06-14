#include "ItemManager.h"
SDL_Surface* ItemManager::itemPowerSmallImage = NULL;
SDL_Surface* ItemManager::itemPointImage = NULL;
SDL_Surface* ItemManager::itemPowerBigImage = NULL;
SDL_Surface* ItemManager::itemBombImage = NULL;
SDL_Surface* ItemManager::itemFullPowerImage = NULL;
SDL_Surface* ItemManager::itemLifeImage = NULL;
SDL_Surface* ItemManager::itemPointBulletImage = NULL;
ItemManager::ItemManager(){
    if(itemPowerSmallImage == NULL){
        itemPowerSmallImage = load_sprite("res/etama/etama2.png", 0, 64, 16, 16, 16.0, 16.0);
    }
    if(itemPointImage == NULL){
        itemPointImage = load_sprite("res/etama/etama2.png", 16, 64, 16, 16, 16.0, 16.0);
    }
    if(itemPowerBigImage == NULL){
        itemPowerBigImage = load_sprite("res/etama/etama2.png", 32, 64, 16, 16, 16.0, 16.0);
    }
    if(itemBombImage == NULL){
        itemBombImage = load_sprite("res/etama/etama2.png", 48, 64, 16, 16, 16.0, 16.0);
    }
    if(itemFullPowerImage == NULL){
        itemFullPowerImage = load_sprite("res/etama/etama2.png", 64, 64, 16, 16, 16.0, 16.0);
    }
    if(itemLifeImage == NULL){
        itemLifeImage = load_sprite("res/etama/etama2.png", 80, 64, 16, 16, 16.0, 16.0);
    }
    if(itemPointBulletImage == NULL){
        itemPointBulletImage = load_sprite("res/etama/etama2.png", 96, 64, 16, 16, 16.0, 16.0);
    }
}