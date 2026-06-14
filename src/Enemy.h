#include "UI.h"
#include "EnemyType.h"
#include "MovePattern.h"
#include "EnemyBulletManager.h"
#include <cmath>
#define PI 3.14159265
class Player;
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
    float vertAmplitude;
    float vertPeriod;
    float horizAmplitude;
    float horizPeriod;
    float halfLife;
    float bezierP1x, bezierP1y;
    float bezierP2x, bezierP2y;
    float bezierEndX, bezierEndY;
    float bezierDuration;
    // 新追踪参数
    float moveAngle;
    float angularVelocity;
    float acceleration;      // 标量加速度 px/s²
    float minPlayerDist;     // 与玩家最小距离
    int enemyType;
    int enemyID;
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
    int enemyType;
    int enemyID;
    bool isActive;
    bool isDead;   // 被击破后阻止 enemy_move 重新激活
    float speedX;
    float speedY;
    float timeAlive;
    float durationTime;
    float hitboxWidth;
    float hitboxHeight;
    int movePattern;
    float emergeTime;
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
    // th06 风格运动字段
    float moveAngle;          // 当前运动角度 (弧度)
    float angularVelocity;    // 角速度 (rad/s)
    float accel;              // 标量加速度 (px/s²)
    float minPlayerDist;      // 与玩家保持的最小距离
    int spriteRow;            // 精灵图行索引 (0-15)
    float spriteAnimTimer;    // 动画计时器
    float clamp(float value, float min_, float max_);
    float cubic_bezier(float t, float p0, float p1, float p2, float p3);
    void compute_axis_speed();  // 根据 movePattern 计算 axisSpeed
    float axisSpeedX;         // 本帧速度 x 分量
    float axisSpeedY;         // 本帧速度 y 分量
    SDL_Surface* get_zako_sprite(int row);
public:
    struct EmitterRuntime {
        float timer;
        int burstRemaining;
        int cycleCount;
    };
    std::vector<EmitterConfig> emitterConfig;
    std::vector<EmitterRuntime> emitterRuntime;
    EnemyBulletManager* bulletManager;
public:
    Enemy();
    void init(EnemyConfig config_, float x_, float y_);
    void update_player_info(float px, float py, size_t frameCounter__);
    void apply_movement(float dt);
    void enemy_move(float dt);
    void enemy_show();
    void enemy_attack(float dt);
    bool is_active();
    void deactivate();
    float get_x() const { return x; }
    float get_y() const { return y; }
    float get_hitbox_w() const { return hitboxWidth; }
    float get_hitbox_h() const { return hitboxHeight; }
    // 屏幕敌机追踪: 碰撞检测遍历此列表
    static Enemy* onScreenList[256];
    static int onScreenCount;
    int  get_hp() const { return hp; }
    // AABB 碰撞检测: 子弹矩形 vs 敌机判定矩形
    bool check_bullet_hit(float bx, float by, float bhw, float bhh);
    // 受到伤害, 返回剩余 hp
    int take_damage(int dmg);
    void test_enemy();
};
