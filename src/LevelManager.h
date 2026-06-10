#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "cJSON.h"
#include "Enemy.h"

enum StageState {
    STAGE_LOADING,   // 关卡加载中
    STAGE_RUNNING,   // 道中/关卡进行中
    STAGE_BOSS,      // Boss 战
    STAGE_CLEAR,     // 关卡通关，等待切换
    STAGE_ALL_CLEAR, // 全部关卡通关
};

class LevelManager {
private:
    cJSON* stage_enemies_data;
    std::vector<Enemy*> enemy_pool;
    int current_stage;
    StageState stage_state;
    static const int TOTAL_STAGES = 8;

    void clear_enemy_pool();
    std::string stage_key() const;

public:
    LevelManager();
    ~LevelManager();
    void load_stage(int stage);
    void init_enemy_pool();

    // 关卡状态机
    void start_stage();            // 开始当前关卡 (LOADING → RUNNING)
    void trigger_boss();           // 进入 Boss 战 (RUNNING → BOSS)
    void clear_stage();            // 关卡通关 (BOSS → CLEAR)
    void next_stage();             // 加载下一关 (CLEAR → LOADING → RUNNING)
    StageState get_stage_state() const { return stage_state; }
    int  get_current_stage() const { return current_stage; }
    bool is_stage_cleared() const { return stage_state == STAGE_CLEAR || stage_state == STAGE_ALL_CLEAR; }

    // Enemy pool management interfaces
    int get_enemy_count() const;
    Enemy* get_enemy(int index);
    void update_all_enemies(float px, float py, size_t frameCounter_);
    void move_all_enemies(float dt_);
    void attack_all_enemies(float dt_);
    void set_bullet_manager_for_all(EnemyBulletManager* mgr);
    void show_all_enemies();
};