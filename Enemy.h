#include "UI.h"
#include "EnemyType.h"
#include "MovePattern.h"
#include <cmath>
#define PI 3.14159265
class Player;
struct EnemyConfig{
    int hp;
    int movePattern;
    size_t emergeTime;
    float durationTime;
    int hitboxWidth;
    int hitboxHeight;
    float speedX;
    float speedY;
    float targetX;
    float targetY;
    float startX;
    float startY;
    float vertAmplitude;
    float vertPeriod;
    float horizAmplitude;
    float horizPeriod;
    float halfLife;
    float bezierP1x, bezierP1y;
    float bezierP2x, bezierP2y;
    float bezierEndX, bezierEndY;
    float bezierDuration;
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
    float timeAlive;  
    float durationTime;
    float hitboxWidth;
    float hitboxHeight;
    int movePattern;
    size_t emergeTime;
    float emergeSpeedY;
    float targetX;
    float targetY;
    int type;
    Player* playerPtr;
    size_t frameCounter_;
    float vertAmplitude;
    float vertPeriod;
    float horizAmplitude;
    float horizPeriod;
    float homingRate;
    float bezierP1x, bezierP1y;
    float bezierP2x, bezierP2y;
    float bezierEndX, bezierEndY;
    float bezierDuration;
    float bezierTime;
    float stateStartX, stateStartY;
    float clamp(float value, float min_, float max_);
    float cubicBezier(float t, float p0, float p1, float p2, float p3);
public:
    Enemy();
    void init(EnemyConfig config_, float x_, float y_);
    void update_player_info(float px, float py, size_t frameCounter__);
    void apply_movement(float dt);
    void enemy_move(float dt);
    void enemy_show();
    void enemy_attack();
    bool is_active();
    void test_enemy();
};