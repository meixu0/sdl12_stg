#include "Enemy.h"
static SDL_Surface* zakoImage = NULL;
Enemy::Enemy() : x(0.0), y(0.0), playerX(0.0), playerY(0.0), hp(0), isActive(false), speedX(0.0), speedY(0.0), durationTime(0.0), targetX(0.0), targetY(0.0), type(0){
    if(zakoImage == NULL)   zakoImage = load_image("res/zako.png", 40.0, 40.0);
}
void Enemy::init(EnemyConfig config, float x_, float y_){
    x = x_;
    y = y_;
    hp = config.hp;
    movePattern = config.movePattern;
    emergeTime = config.emergeTime;
    durationTime = config.durationTime;
    hitboxHeight = config.hitboxHeight;
    hitboxWidth = config.hitboxWidth;
    speedX = config.speedX;
    speedY = config.speedY;
    emergeSpeedY = 5.0;//todo: set more emergeSpeed;
    isActive = true;
    targetX = config.targetX;
    targetY = config.targetY;
}
void Enemy::linear_move(){
        if(isActive){
            if(y <= targetY){
            y += emergeSpeedY;
        }else{
            y += speedY;
        }
    }else return;
}
void Enemy::enemy_move(){
    if(isActive){
        switch (movePattern){
            case(LINER): linear_move(); break;
            //todo: more move pattern setting
        }
    }else return;
}
void Enemy::enemy_show(){
    if(isActive){
        switch (type){
            case(ZAKO): apply_surface((int)x, int(y), zakoImage, screen);
        }
    }else return;
}