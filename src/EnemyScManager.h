#pragma once
#include <string>
#include <vector>
#include "EnemyBulletManager.h"
#include "cJSON.h"

/// Per-spellcard definition parsed from JSON
struct SpellcardDef {
    int id = 0;                      ///< Sequential index within this boss
    std::string name;                ///< Display name (base name, no diff suffix)
    std::string scName;              ///< Full original spellcard name (with diff suffix)
    int hp = 2000;                   ///< HP required to clear this spellcard
    int timeout = 45;                ///< Timeout in seconds
    std::vector<EmitterConfig> patterns;  ///< Bullet patterns for this phase
};

/// Manages spellcard definitions and runtime state for the current boss encounter.
///
/// Usage:
///   1. load(stage)    — load spellcard data for the current stage's boss
///   2. start(index)   — begin a specific spellcard
///   3. update(dt)     — advance the spellcard timer each frame
///   4. damage(hit)    — apply damage to the current spellcard HP
///   5. end(captured)  — finish the current spellcard (timeout or capture)
///   6. current()      — get the active spellcard's patterns for the enemy
class EnemyScManager {
public:
    EnemyScManager();
    ~EnemyScManager();

    //Loading
    /// Load spellcards for the given stage (reads level/sc/stage{N}.json).
    /// Returns false if the file cannot be loaded.
    bool load(int stage);

    //State queries
    int  count() const { return int(spellcards_.size()); }
    bool is_active() const { return active_; }
    bool is_timeout() const;
    int  current_index() const { return current_index_; }
    int  spellcards_remaining() const { return count() - current_index_; }

    const SpellcardDef* current() const;
    const SpellcardDef* get(int index) const;
    const std::string&  current_name() const;

    //Lifecycle
    /// Begin spellcard at `index`. Returns false if index out of range.
    bool start(int index);

    /// Advance the spellcard timer by `dt` seconds.
    void update(float dt);

    /// Apply `damage` to the current spellcard's remaining HP.
    void damage(int dmg);

    /// End the current spellcard. `captured = true` means the player cleared
    /// it before timeout. Resets active state. Returns current HP delta
    /// for score calculation.
    int  end(bool captured);

    /// Reset all state (cancels active spellcard, clears loaded data).
    void reset();

    //Timer
    float time_remaining() const;
    float time_elapsed() const { return timer_; }

    /// Despawn all enemy bullets — call when a spellcard starts or ends.
    static void clear_bullets(EnemyBulletManager* mgr);
    int get_current_hp(){return current_hp_;}
    int get_total_hp() const { return current() ? current()->hp : 0; }

private:
    std::vector<SpellcardDef> spellcards_;
    int  current_index_ = 0;
    float timer_ = 0.0f;
    int  current_hp_ = 0;
    bool active_ = false;

    /// Parse a single JSON pattern object → EmitterConfig.
    EmitterConfig parse_pattern(cJSON* pat_json);
};

/// Converts an ECL-style frame count to seconds.
inline float frames_to_sec(int frames) { return frames / 60.0f; }
