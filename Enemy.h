#include "UI.h"
#include "EnemyType.h"
#include "MovePattern.h"
#include <cmath>
class Player;
struct EnemyConfig{
    int hp;
    int movePattern;
    size_t emergeTime;
    size_t durationTime;
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
    void linear_move();
    void sin_wave_move();
    void u_turn_move();
    void stop_and_go_move();
    void homing_move();
    void interception_move();
public:
    Enemy();
    void init(EnemyConfig config_, float x_, float y_);
    void update_player_info(float px, float py, size_t frameCounter__);
    void enemy_move();
    void enemy_show();
    void enemy_attack();
    bool is_active();
    void test_enemy();
};