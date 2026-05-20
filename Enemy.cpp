#include "Enemy.h"
static SDL_Surface* zakoImage = NULL;
Enemy::Enemy() : x(0.0), y(0.0), startX(0.0), startY(0.0), playerX(0.0), playerY(0.0), hp(0), isActive(false), speedX(0.0), speedY(0.0), timeAlive(0.0), durationTime(0.0), targetX(0.0), targetY(0.0), type(0){
    if(zakoImage == NULL)   zakoImage = load_image("res/zako.png", 40.0, 40.0);
}
void Enemy::init(EnemyConfig config, float x_, float y_){
    x = x_;
    y = y_;
    startX = x_;        // 保存初始位置
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
    timeAlive = 0.0;    // 初始化存活时间
    isActive = true;
    targetX = config.targetX;
    targetY = config.targetY;
}

// 线性移动：直线向下运动
void Enemy::linear_move(){
    if(!isActive) return;
    
    // 出场阶段
    if(y <= targetY){
        y += emergeSpeedY;
    } else {
        // 正常运动阶段
        x += speedX * timeAlive;  // speedX 作为基础速度因子
        y += speedY;
    }
}

// 正弦波移动：水平摆动
void Enemy::sin_wave_move(){
    if(!isActive) return;
    
    // 出场阶段
    if(y <= targetY){
        y += emergeSpeedY;
    } else {
        // 正常运动阶段：水平正弦振荡
        float sineOffset = sin(timeAlive * 3.0f) * 80.0f;  // 振幅80像素，频率3.0
        x = startX + sineOffset;
        y += speedY;
    }
}

// U形转向：先下降后改变方向
void Enemy::u_turn_move(){
    if(!isActive) return;
    
    if(y <= targetY){
        y += emergeSpeedY;
    } else {
        // 执行U形转向
        float turnDuration = 1.5f;  // 1.5秒内完成转向
        float turnProgress = timeAlive / turnDuration;
        
        if(turnProgress < 1.0f){
            // 使用二次贝塞尔曲线实现平滑转向
            x = startX + speedX * turnDuration * turnProgress;  // 逐渐改变x
            y += speedY * (1.0f - turnProgress * turnProgress);  // y速度逐渐减小
        } else {
            // 转向完成后继续移动
            x += speedX;
            y -= speedY * 0.5f;  // 向上运动
        }
    }
}

// 停止与移动：周期性停止
void Enemy::stop_and_go_move(){
    if(!isActive) return;
    
    if(y <= targetY){
        y += emergeSpeedY;
    } else {
        // 周期为2秒（60帧/秒 = 120帧）
        float cycleDuration = 1.0f;
        float cyclePhase = fmod(timeAlive, cycleDuration * 2.0f);
        
        if(cyclePhase < cycleDuration){
            // 移动阶段
            y += speedY;
            x += speedX;
        }
        // 否则停止不动
    }
}

// 追踪玩家：向玩家位置移动
void Enemy::homing_move(){
    if(!isActive) return;
    
    if(y <= targetY){
        y += emergeSpeedY;
    } else {
        // 计算指向玩家的方向
        float dx = playerX - x;
        float dy = playerY - y;
        float distance = sqrt(dx*dx + dy*dy);
        
        if(distance > 0.1f){
            // 归一化方向并应用速度
            x += (dx / distance) * speedX;
            y += (dy / distance) * speedY;
        }
    }
}

// 拦截移动：预判玩家位置
void Enemy::interception_move(){
    if(!isActive) return;
    
    if(y <= targetY){
        y += emergeSpeedY;
    } else {
        // 预测玩家位置（假设玩家以恒定速度移动）
        float predictDist = 100.0f;  // 预测距离
        float dx = (playerX + predictDist) - x;
        float dy = (playerY + predictDist) - y;
        float distance = sqrt(dx*dx + dy*dy);
        
        if(distance > 0.1f){
            x += (dx / distance) * speedX;
            y += (dy / distance) * speedY;
        }
    }
}
void Enemy::enemy_move(){
    if(isActive){
        timeAlive += (1.0f / 60.0f);  // 假设60fps，每帧增加1/60秒
        
        switch (movePattern){
            case LINER:         linear_move(); break;
            case SINWAVE:       sin_wave_move(); break;
            case UTURN:         u_turn_move(); break;
            case STOPANDGO:     stop_and_go_move(); break;
            case HOMING:        homing_move(); break;
            case INTERCEPTION:  interception_move(); break;
            default:            linear_move(); break;
        }
    }
}
void Enemy::enemy_show(){
    if(isActive){
        switch (type){
            case(ZAKO): apply_surface((int)x, int(y), zakoImage, screen);
        }
    }else return;
}