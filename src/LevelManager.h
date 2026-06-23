#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "cJSON.h"
#include "Enemy.h"
#include "EnemyType.h"

enum StageState {
    STAGE_LOADING,   // 关卡加载中
    STAGE_RUNNING,   // 道中/关卡进行中
    STAGE_MIDBOSS,   // 道中Boss战
    STAGE_BOSS,      // 关底Boss战
    STAGE_CLEAR,     // 关卡通关，等待切换
    STAGE_ALL_CLEAR, // 全部关卡通关
};

class LevelManager {
private:
    cJSON* stage_enemies_data;
    std::vector<Enemy*> enemy_pool;
    int current_stage;
    StageState stage_state;
    static const int TOTAL_STAGES = 2;

    void clear_enemy_pool();
    std::string stage_key() const;
    bool isClearingForMidboss;//整个关卡中只需要执行一次为midboss清理，因此初始设为true，执行清理后改为false即可避免死循环
    int midbossIndex_;

public:
    LevelManager();
    ~LevelManager();
    void load_stage(int stage);
    void load_boss_stage(int stage);
    void init_enemy_pool();

    // 关卡状态机
    void start_stage();            // 开始当前关卡 (LOADING → RUNNING)
    void trigger_boss();           // 进入 Boss 战 (RUNNING → BOSS)
    void trigger_midboss();        // 进入道中Boss (RUNNING → MIDBOSS)
    void clear_stage();            // 关卡通关 (BOSS → CLEAR)
    void next_stage();             // 加载下一关 (CLEAR → LOADING → RUNNING)
    bool update_stage_state();     // 每帧更新: 检测midboss出现/击破
    StageState get_stage_state() const { return stage_state; }
    int  get_current_stage() const { return current_stage; }
    bool is_stage_cleared() const { return stage_state == STAGE_CLEAR || stage_state == STAGE_ALL_CLEAR; }

    // Enemy pool management interfaces
    int get_enemy_count() const;
    Enemy* get_enemy(int index);
    void trigger_midboss_clear(Uint32 &frameCounter, Uint32 &midbossEnterFrame_, StageState &prevStageState, StageState &currentStageState);
    void update_all_enemies(float px, float py, Uint32 &frameCounter_, Uint32 &midbossEnterFrame_, float dt_, StageState &prevStageState, StageState &currentStageState);
    void move_all_enemies(float dt_);
    void attack_all_enemies(float dt_);
    void set_bullet_manager_for_all(EnemyBulletManager* mgr);
    void show_all_enemies();
};