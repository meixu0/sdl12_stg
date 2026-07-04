#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "cJSON.h"
#include "Enemy.h"
#include "EnemyType.h"
#include "EnemyScManager.h"
#include <SDL_mixer.h>
class ItemManager;

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
    bool isClearingForMidboss;
    int midbossIndex_;
    EnemyBulletManager* bullet_mgr_;
    ItemManager* item_mgr_;
    Mix_Music* bgm_music_;            // current BGM track (owned, freed on stop/switch)

    // ── Spellcard boss management ──────────────────────────────────────────
    EnemyScManager scManager_;
    int bossEnemyIndex_;
    int spellcardTriggerHp_;       // Boss HP threshold to trigger spellcard
    float spellcardEntryTimer_;    // Timer for spellcard entry animation (2s)
    float nonSpellcardTimer_;      // ECL 1680-frame (28s) timer for non-spellcard timeout
    bool midbossDefeatedProcessed_ = false;

    void start_spellcard_phase(int phaseIndex);

public:
    LevelManager();
    ~LevelManager();
    void load_stage(int stage);
    void load_boss_stage(int stage);
    void init_enemy_pool();
    void init_enemy_pool_v2();
    void bgm_play(int stage, bool is_boss = false);

    void start_stage();            // 开始当前关卡 (LOADING → RUNNING)
    void trigger_boss();           // 进入 Boss 战 (RUNNING → BOSS)
    void trigger_midboss();        // 进入道中Boss (RUNNING → MIDBOSS)
    void clear_stage();            // 关卡通关 (BOSS → CLEAR)
    void next_stage();             // 加载下一关 (CLEAR → LOADING → RUNNING)

    void set_bullet_manager(EnemyBulletManager* mgr) { bullet_mgr_ = mgr; }
    void set_item_manager(ItemManager* mgr) { item_mgr_ = mgr; }
    void enter_stage(int stage);   // clean → load → init → RUNNING
    void enter_boss(int stage);    // clean → load boss → init → BOSS
    bool auto_transition(Uint32 frameCounter);  // 检测并自动处理所有关卡过渡

    // ── Spellcard lifecycle ────────────────────────────────────────────────
    void update_spellcards(float dt);  // advance timer, check capture/timeout, transition phases
    const EnemyScManager* get_sc_manager() const { return &scManager_; }

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

    // ── Boss/midboss defeat handling ──────────────────────────────────────
    void convert_boss_bullets_to_p_items();
    void skip_stage_time(float seconds);
};