#include "UI.h"
#include "EnemyType.h"
#include "MovePattern.h"
#include "EnemyBulletManager.h"
#include "EnemyScManager.h"
#include <cmath>
#define PI 3.14159265
class Player;
class EnemyScManager;
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
    float moveAngle;
    float angularVelocity;
    float acceleration; /* 标量加速度 px/s^2 */
    float minPlayerDist; /* 与玩家最小距离 */
    int enemyType;
    int enemyID;
    bool isMidboss;
};
/* ── Phase sequencer struct (ECL Sub26 attack cycle) ───────────────────── */
struct BossPhaseDef {
    float duration;
    std::vector<EmitterConfig> patterns;
    float moveToX, moveToY;
    float moveDuration;
    int   moveEasing;
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
    int totalHp; /* 初始总 HP */
    int enemyType;
    int enemyID;
    bool isActive;
    bool isDead;
    bool isMidboss;
    bool isEntering;
    float entryTargetY;
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
    float moveDuration;       // ECL position_interp duration (seconds)
    float moveElapsed;        // ECL position_interp elapsed time
    int   moveEasing;         // ECL easing: 0=linear, 4=decelerate
    float moveAngle; /* 当前运动角度 (弧度) */
    float angularVelocity; /* 角速度 (rad/s) */
    float accel; /* 标量加速度 (px/s^2) */
    float minPlayerDist; /* 与玩家保持的最小距离 */
/* ECL move_bounds_set support — constrains boss movement area */
    float moveMinX, moveMaxX, moveMinY, moveMaxY;
    bool  hasMoveBounds;
    int spriteRow; /* 精灵图行索引 (0-15) */
    float spriteAnimTimer; /* 动画计时器 */
    float clamp(float value, float min_, float max_);
    float cubic_bezier(float t, float p0, float p1, float p2, float p3);
    void compute_axis_speed(); /* 根据 movePattern 计算 axisSpeed */
    float axisSpeedX; /* 本帧速度 x 分量 */
    float axisSpeedY; /* 本帧速度 y 分量 */
    SDL_Surface* get_zako_sprite(int row);
    SDL_Surface* get_boss_sprite(int col);
    static Mix_Chunk* tan00;
    //Phase sequencer
    std::vector<BossPhaseDef> bossPhases_;
    int currentPhase_;
    float phaseTimer_;
    bool isPhasedBoss_;
    void advance_phase();
public:
    struct EmitterRuntime {
        float timer;
        int burstRemaining;
        int cycleCount;
    };
    std::vector<EmitterConfig> emitterConfig;
    std::vector<EmitterRuntime> emitterRuntime;
    EnemyBulletManager* bulletManager;
    Enemy();
    ~Enemy();
    void init(EnemyConfig config_, float x_, float y_);
    void update_player_info(float px, float py, size_t frameCounter__);
    void apply_movement(float dt);
    void force_retreat();
    void enemy_move(float dt);
    void enemy_show();
    void enemy_attack(float dt);
    bool is_active();
    void deactivate();
    void boss_entry();
    float get_x() const { return x; }
    float get_y() const { return y; }
    float get_hitbox_w() const { return hitboxWidth; }
    float get_hitbox_h() const { return hitboxHeight; }
    static Enemy* onScreenList[256];
    static int onScreenCount;
    int  get_hp() const { return hp; }
    int  get_total_hp() const { return totalHp; }
    bool is_using_spellcard() const { return scManager && scManager->is_active(); }
    int  get_sc_total_hp() const { return scManager ? scManager->get_total_hp() : 0; }
    int  get_sc_current_hp() const { return scManager ? scManager->get_current_hp() : 0; }
    float get_sc_time_remaining() const { return scManager ? scManager->time_remaining() : 0.0f; }
    float get_time_alive() const { return timeAlive; }
    float get_emerge_time() const { return emergeTime; }
    float get_duration_time() const { return durationTime; }
    bool get_is_midboss() const { return isMidboss; }
    int  get_enemy_type() const { return enemyType; }
    void adjust_time_alive(float offset) { timeAlive += offset; }
    void set_duration_time(float t) { durationTime = t; }
    void set_speedY(float sy) { speedY = sy; }
    void set_move_pattern(int mp) { movePattern = mp; }
    void start_position_interp(float dur, int easing, float tx, float ty);  // ECL move_position_interp
    void set_move_bounds(float minX, float minY, float maxX, float maxY);   // ECL move_bounds_set

    //ECL phase sequencer (attack cycle)
    void set_phases(const std::vector<BossPhaseDef>& phases);
    void start_phases();
    void stop_phases() { isPhasedBoss_ = false; currentPhase_ = -1; }

    //Spellcard boss support
    bool isSpellcardBoss;
    EnemyScManager* scManager;

    bool is_spellcard_boss() const { return isSpellcardBoss; }
    void link_spellcard_manager(EnemyScManager* mgr);
    void set_spellcard_hp(int newHp);
    bool has_pending_spawn() const { return !isDead && timeAlive < emergeTime; }
    bool check_bullet_hit(float bx, float by, float bhw, float bhh);
    int take_damage(int dmg);
    void test_enemy();
};
