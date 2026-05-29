#include "Enemy.h"
#include "player.h"
#include <iostream>
static SDL_Surface* zakoImage = NULL;
Enemy::Enemy() : x(0.0), y(0.0), startX(0.0), startY(0.0), playerX(0.0), playerY(0.0), hp(0), isActive(false),
    speedX(0.0), speedY(0.0), timeAlive(0.0), durationTime(0.0), targetX(0.0), targetY(0.0), type(0),
    bezierP1x(0), bezierP1y(0), bezierP2x(0), bezierP2y(0), bezierEndX(0), bezierEndY(0),
    bezierDuration(0), bezierTime(0), stateStartX(0), stateStartY(0),
    moveAngle(0), angularVelocity(0), accel(0), minPlayerDist(80.0f),
    axisSpeedX(0), axisSpeedY(0) {
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
    moveAngle = config.moveAngle;
    angularVelocity = config.angularVelocity;
    accel = config.acceleration;
    minPlayerDist = (config.minPlayerDist > 0.0f) ? config.minPlayerDist : 80.0f;
    emitterRuntime.resize(emitterConfig.size());
    for (size_t i = 0; i < emitterRuntime.size(); i++) {
        emitterRuntime[i].timer = 0.0f;
        emitterRuntime[i].burstRemaining = 0;
        emitterRuntime[i].cycleCount = 0;
    }
}
bool Enemy::is_active(){
    return isActive;
}
float Enemy::clamp(float value, float min_, float max_){
    if(value < min_)  return min_;
    else if(value > max_) return max_;
    else return value;
}

float Enemy::cubic_bezier(float t, float p0, float p1, float p2, float p3){
    float u = 1.0f - t;
    return u*u*u * p0 + 3*u*u*t * p1 + 3*u*t*t * p2 + t*t*t * p3;
}
void Enemy::update_player_info(float px, float py, size_t frameCounter__){
    frameCounter_ = frameCounter__;
    playerX = px;
    playerY = py;
}

void Enemy::compute_axis_speed(){
    switch (movePattern){
        case LINER:
            axisSpeedX = 0.0f;
            axisSpeedY = speedY;
            break;

        case SINWAVE:
            axisSpeedX = speedX + cosf(timeAlive * 2.0f * PI / horizPeriod) * horizAmplitude;
            axisSpeedY = sinf(timeAlive * 2.0f * PI / vertPeriod) * vertAmplitude;
            break;

        case HOMING: {
            float dx = playerX - x;
            float dy = playerY - y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist > 0.1f) {
                float targetAngle = atan2f(dy, dx);

                // 平滑转向：角速度限制
                if (angularVelocity > 0.0f && fabsf(targetAngle - moveAngle) > 0.001f) {
                    float diff = targetAngle - moveAngle;
                    // 角度标准化到 [-PI, PI]
                    while (diff > PI)  diff -= 2.0f * PI;
                    while (diff < -PI) diff += 2.0f * PI;
                    float maxTurn = angularVelocity * (1.0f / 60.0f);
                    if (diff > maxTurn)       moveAngle += maxTurn;
                    else if (diff < -maxTurn) moveAngle -= maxTurn;
                    else                       moveAngle = targetAngle;
                } else {
                    moveAngle = targetAngle;
                }

                // 距离越近速度越慢，小于 minPlayerDist 就停
                float curSpeed = speedX; 
                if (dist < minPlayerDist) {
                    curSpeed = speedX * (dist / minPlayerDist);
                }
                // 加速度
                curSpeed += accel * (1.0f / 60.0f);
                if (curSpeed < 0.0f) curSpeed = 0.0f;

                axisSpeedX = cosf(moveAngle) * curSpeed;
                axisSpeedY = sinf(moveAngle) * curSpeed;
            } else {
                axisSpeedX = 0.0f;
                axisSpeedY = 0.0f;
            }
            break;
        }

        case BEZIER:
            bezierTime += (1.0f / 60.0f);
            if (bezierDuration > 0.0f) {
                float t = clamp(bezierTime / bezierDuration, 0.0f, 1.0f);
                float nextX = cubic_bezier(t, stateStartX, bezierP1x, bezierP2x, bezierEndX);
                float nextY = cubic_bezier(t, stateStartY, bezierP1y, bezierP2y, bezierEndY);
                axisSpeedX = (nextX - x) * 60.0f; 
                axisSpeedY = (nextY - y) * 60.0f;
            }
            break;

        case STOPANDGO:
            {
                float cycle = fmodf(timeAlive, 2.0f);
                if (cycle < 1.0f) {
                    axisSpeedX = 0.0f;
                    axisSpeedY = speedY;
                } else {
                    axisSpeedX = 0.0f;
                    axisSpeedY = 0.0f;
                }
            }
            break;

        case INTERCEPTION:
            // 预判玩家位置，类似 homing 但瞄准预测点
            {
                float predictTime = 0.5f;
                float predX = playerX; 
                float predY = playerY;
                float dx = predX - x;
                float dy = predY - y;
                float dist = sqrtf(dx*dx + dy*dy);
                if (dist > 0.1f) {
                    float targetAngle = atan2f(dy, dx);
                    if (angularVelocity > 0.0f) {
                        float diff = targetAngle - moveAngle;
                        while (diff > PI)  diff -= 2.0f * PI;
                        while (diff < -PI) diff += 2.0f * PI;
                        float maxTurn = angularVelocity * (1.0f / 60.0f);
                        diff = clamp(diff, -maxTurn, maxTurn);
                        moveAngle += diff;
                    } else {
                        moveAngle = targetAngle;
                    }
                    float curSpeed = speedX;
                    if (dist < minPlayerDist) curSpeed = speedX * (dist / minPlayerDist);
                    axisSpeedX = cosf(moveAngle) * curSpeed;
                    axisSpeedY = sinf(moveAngle) * curSpeed;
                } else {
                    axisSpeedX = 0.0f;
                    axisSpeedY = 0.0f;
                }
            }
            break;

        default:
            axisSpeedX = 0.0f;
            axisSpeedY = 0.0f;
            break;
    }
}

void Enemy::enemy_move(float dt){
    timeAlive += dt;
    if(x + 40 < 0 || x >= 544 - 40 || y + 40 < 0 || y >= 800 - 40 ||
       timeAlive < emergeTime || timeAlive >= emergeTime + durationTime) {
        isActive = false;
    } else {
        isActive = true;
    }
    if(!isActive) return;

    if(playerPtr != NULL){
        PlayerPosition playerPos = playerPtr->get_player_position();
        playerX = playerPos.x;
        playerY = playerPos.y;
    }

    compute_axis_speed();

    // 统一应用位移
    x += axisSpeedX * dt;
    y += axisSpeedY * dt;

    if (x < 8.0f)        x = 8.0f;
    if (x > 536.0f)      x = 536.0f;
    if (y < 0.0f)        { /* 允许从上方出场 */ }
    if (y > 592.0f)      y = 592.0f;
}

void Enemy::enemy_attack(float dt){
    if (!isActive || bulletManager == NULL) return;
    if (emitterConfig.size() != emitterRuntime.size()) return;

    for (size_t i = 0; i < emitterConfig.size(); i++) {
        EmitterConfig& ec = emitterConfig[i];
        EmitterRuntime& rt = emitterRuntime[i];

        rt.timer += dt;
        if (rt.timer < ec.startDelay) continue;

        float activeTime = rt.timer - ec.startDelay;
        int  burstMax   = (ec.burstCount > 0) ? ec.burstCount : 1;

        if (burstMax == 1) {
            if (activeTime >= ec.emitInterval) {
                bulletManager->spawn_pattern(ec.patternDesc, x, y, playerX, playerY);
                rt.timer = ec.startDelay;
            }
            continue;
        }

        if (rt.burstRemaining <= 0) {
            if (activeTime >= ec.emitInterval) {
                rt.burstRemaining = burstMax;
                rt.timer = ec.startDelay;
                activeTime = 0.0f;
            } else {
                continue;
            }
        }

        if (activeTime >= ec.burstInterval) {
            bulletManager->spawn_pattern(ec.patternDesc, x, y, playerX, playerY);
            rt.burstRemaining--;
            rt.timer = ec.startDelay;
            if (rt.burstRemaining > 0) {
                rt.timer = ec.startDelay;
            }
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
