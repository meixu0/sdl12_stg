#include "Enemy.h"
#include "player.h"
#include <iostream>
static SDL_Surface* zakoImage = NULL;
Enemy::Enemy() : x(0.0), y(0.0), startX(0.0), startY(0.0), playerX(0.0), playerY(0.0), hp(0), isActive(false), speedX(0.0), speedY(0.0), timeAlive(0.0), durationTime(0.0), targetX(0.0), targetY(0.0), type(0),
    bezierP1x(0), bezierP1y(0), bezierP2x(0), bezierP2y(0), bezierEndX(0), bezierEndY(0), bezierDuration(0), bezierTime(0), stateStartX(0), stateStartY(0) {
    if(zakoImage == NULL)   zakoImage = load_image("res/zako.png", 40.0, 40.0);
}
void Enemy::init(EnemyConfig config, float x_, float y_){
    x = x_;
    y = y_;
    startX = x_;
    startY = y_;
    hp = config.hp;
    movePattern = config.movePattern;
    emergeTime = config.emergeTime;
    durationTime = config.durationTime;
    hitboxHeight = config.hitboxHeight;
    hitboxWidth = config.hitboxWidth;
    speedX = config.speedX;
    speedY = config.speedY;
    emergeSpeedY = 5.0;
    timeAlive = 0.0;
    isActive = false;
    targetX = config.targetX;
    targetY = config.targetY;
    vertAmplitude = config.vertAmplitude;
    vertPeriod = config.vertPeriod;
    horizAmplitude = config.horizAmplitude;
    horizPeriod = config.horizPeriod;
    if (config.halfLife > 0.0f)
        homingRate = 0.693f / config.halfLife;
    else
        homingRate = 0.0f;
    bezierP1x = config.bezierP1x;
    bezierP1y = config.bezierP1y;
    bezierP2x = config.bezierP2x;
    bezierP2y = config.bezierP2y;
    bezierEndX = config.bezierEndX;
    bezierEndY = config.bezierEndY;
    bezierDuration = config.bezierDuration;
    bezierTime = 0.0f;
    stateStartX = x_;
    stateStartY = y_;
}
bool Enemy::is_active(){
    return isActive;
}
float Enemy::clamp(float value, float min_, float max_){
    if(value < min_)  return min_;
    else if(value > max_) return max_;
    else return value;
}

float Enemy::cubicBezier(float t, float p0, float p1, float p2, float p3){
    float u = 1.0f - t;
    return u*u*u * p0 + 3*u*u*t * p1 + 3*u*t*t * p2 + t*t*t * p3;
}
void Enemy::update_player_info(float px, float py, size_t frameCounter__){
    frameCounter_ = frameCounter__;
    playerX = px;
    playerY = py;
}
void Enemy::enemy_move(float dt){
    timeAlive += dt;
    if(x + 40 < 0 || x >= 544 - 40 || y + 40 < 0 || y >= 800 - 40 || timeAlive < emergeTime || timeAlive >= emergeTime + durationTime)  isActive = false;
    else    isActive = true;
    if(isActive){
        if(playerPtr != NULL){
            PlayerPosition playerPos = playerPtr->get_player_position();
            playerX = playerPos.x;
            playerY = playerPos.y;
        }        
        switch (movePattern){
            case(LINER): y += speedY * dt; break; 
            case(SINWAVE): y += sin(timeAlive * 2*PI / vertPeriod) * vertAmplitude * dt; x += speedX * dt + cos(timeAlive * 2*PI / horizPeriod) * horizAmplitude * dt; break;
            case(HOMING): x += clamp((playerX - x) * homingRate * dt, -speedX, speedX); y += clamp((playerY - y) * homingRate * dt, -speedY, speedY); break;
	            case(BEZIER):
	                bezierTime += dt;
	                if (bezierDuration > 0.0f) {
	                    float t = clamp(bezierTime / bezierDuration, 0.0f, 1.0f);
	                    x = cubicBezier(t, stateStartX, bezierP1x, bezierP2x, bezierEndX);
	                    y = cubicBezier(t, stateStartY, bezierP1y, bezierP2y, bezierEndY);
	                }
	                break;
        }    
    }
}
void Enemy::enemy_show(){
    if(isActive){
        switch (type){
            case(ZAKO): apply_surface((int)x, int(y), zakoImage, screen);
            std::cout << "enemy show at " << x << "," << y << " time is"<< timeAlive << std::endl;
        }
    }else return;
}
