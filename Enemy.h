#include "UI.h"
#include "EnemyType.h"
#include "MovePattern.h"
#include <cmath>
struct EnemyConfig{
    int hp;
    int movePattern;
    float emergeTime;
    float durationTime;
    int hitboxWidth;
    int hitboxHeight;
    float speedX;
    float speedY;
    float targetX;
    float targetY;
    float startX;
    float startY;
};
class Enemy{
private:
    float x;
    float y;
    float startX;
    float startY;
    float playerX;
    float playerY;
    int hp;
    bool isActive;
    float speedX;
    float speedY;
    float timeAlive;        // 敌机存活时间（秒）
    float durationTime;
    float hitboxWidth;
    float hitboxHeight;
    int movePattern;
    float emergeTime;
    float emergeSpeedY;
    float targetX;
    float targetY;
    int type;
    void linear_move();
    void sin_wave_move();
    void u_turn_move();
    void stop_and_go_move();
    void homing_move();
    void interception_move();
public:
    Enemy();
    void init(EnemyConfig config_, float x_, float y_);
    void enemy_move();
    void enemy_show();
    void enemy_attack();
    bool is_active();
    void test_enemy();
};