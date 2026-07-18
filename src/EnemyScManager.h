#pragma once
#include <string>
#include <vector>
#include "EnemyBulletManager.h"
#include "cJSON.h"

struct SpellcardDef {
    int id;
    std::string name;
    std::string scName;
    int hp;
    int timeout;
    std::vector<EmitterConfig> patterns;
    SpellcardDef() : id(0), hp(2000), timeout(45) {}
};

class EnemyScManager {
public:
    EnemyScManager();
    ~EnemyScManager();

    bool load(int stage);

    int  count() const { return int(spellcards_.size()); }
    bool is_active() const { return active_; }
    bool is_timeout() const;
    int  current_index() const { return current_index_; }
    int  spellcards_remaining() const { return count() - current_index_; }

    const SpellcardDef* current() const;
    const SpellcardDef* get(int index) const;
    const std::string&  current_name() const;

    bool start(int index);
    void update(float dt);
    void damage(int dmg);
    int  end(bool captured);
    void reset();

    float time_remaining() const;
    float time_elapsed() const { return timer_; }

    static void clear_bullets(EnemyBulletManager* mgr);
    int get_current_hp(){return current_hp_;}
    int get_total_hp() const { return current() ? current()->hp : 0; }

private:
    std::vector<SpellcardDef> spellcards_;
    int  current_index_;
    float timer_;
    int  current_hp_;
    bool active_;

    EmitterConfig parse_pattern(cJSON* pat_json);
};

inline float frames_to_sec(int frames) { return frames / 60.0f; }
