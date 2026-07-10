#include "EnemyScManager.h"
#include "GameSettings.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>

// Difficulty key names matching GameSettings.h defines
static const char* SC_DIFF_KEYS[] = {
    "easy", "normal", "hard", "lunatic", "extra"
};

//Helpers
static float cj_float(cJSON* obj, const char* key, float def) {
    cJSON* v = cJSON_GetObjectItem(obj, key);
    return (v && cJSON_IsNumber(v)) ? float(v->valuedouble) : def;
}

static int cj_int(cJSON* obj, const char* key, int def) {
    cJSON* v = cJSON_GetObjectItem(obj, key);
    return (v && cJSON_IsNumber(v)) ? int(v->valuedouble) : def;
}

static const char* cj_str(cJSON* obj, const char* key) {
    cJSON* v = cJSON_GetObjectItem(obj, key);
    return (v && cJSON_IsString(v)) ? v->valuestring : nullptr;
}

// Apply difficulty overrides to a partially-parsed SpellcardDef
static void apply_spellcard_difficulty(SpellcardDef& sc, cJSON* entry) {
    cJSON* diff_obj = cJSON_GetObjectItem(entry, "difficulty");
    if (!diff_obj || !cJSON_IsObject(diff_obj)) return;

    int diff_level = gameDifficulty;
    if (diff_level < 0 || diff_level > 4) diff_level = DIFFICULTY_NORM;

    const char* key = SC_DIFF_KEYS[diff_level];
    cJSON* over = cJSON_GetObjectItem(diff_obj, key);
    if (!over || !cJSON_IsObject(over)) return;

    // Scalar overrides
    cJSON* v;
    if ((v = cJSON_GetObjectItem(over, "hp"))      && cJSON_IsNumber(v)) sc.hp      = (int)v->valuedouble;
    if ((v = cJSON_GetObjectItem(over, "timeout"))  && cJSON_IsNumber(v)) sc.timeout = (int)v->valuedouble;

    // Pattern-level overrides (keyed by index "0", "1", ...)
    cJSON* pats_ov = cJSON_GetObjectItem(over, "patterns");
    if (pats_ov && cJSON_IsObject(pats_ov)) {
        cJSON* child = NULL;
        cJSON_ArrayForEach(child, pats_ov) {
            if (!child || !cJSON_IsObject(child)) continue;
            int idx = atoi(child->string);
            if (idx < 0 || idx >= (int)sc.patterns.size()) continue;

            EmitterConfig& ec = sc.patterns[idx];
            ec.emitInterval  = cj_float(child, "interval", ec.emitInterval);
            ec.startDelay    = cj_float(child, "time", ec.startDelay);
            ec.burstCount    = cj_int(child, "burst", ec.burstCount);
            ec.burstInterval = cj_float(child, "burstInterval", ec.burstInterval);

            cJSON* pat = cJSON_GetObjectItem(child, "pattern");
            if (pat && cJSON_IsObject(pat)) {
                const char* ptype = cj_str(pat, "type");
                if (ptype) {
                    if      (strcmp(ptype, "fan_aimed")           == 0) ec.patternDesc.patternType = 0;
                    else if (strcmp(ptype, "fan")                 == 0) ec.patternDesc.patternType = 1;
                    else if (strcmp(ptype, "aimed")               == 0) ec.patternDesc.patternType = 2;
                    else if (strcmp(ptype, "ring")                == 0) ec.patternDesc.patternType = 3;
                    else if (strcmp(ptype, "ring_aimed")          == 0) ec.patternDesc.patternType = 4;
                    else if (strcmp(ptype, "spiral")              == 0) ec.patternDesc.patternType = 5;
                    else if (strcmp(ptype, "spread")              == 0) ec.patternDesc.patternType = 6;
                    else if (strcmp(ptype, "ring_aimed_variable") == 0) ec.patternDesc.patternType = 7;
                }
                ec.patternDesc.mainCnt       = cj_int(pat, "mainCnt", ec.patternDesc.mainCnt);
                ec.patternDesc.subCnt        = cj_int(pat, "subCnt", ec.patternDesc.subCnt);
                ec.patternDesc.angleOffset   = cj_float(pat, "spread", ec.patternDesc.angleOffset);
                ec.patternDesc.angleInterval = cj_float(pat, "angleStep", ec.patternDesc.angleInterval);
                ec.patternDesc.speed1        = cj_float(pat, "speed", ec.patternDesc.speed1);
                ec.patternDesc.speed2        = cj_float(pat, "speedStep", ec.patternDesc.speed2);
                ec.patternDesc.spriteID      = cj_int(pat, "spriteID", ec.patternDesc.spriteID);
                ec.patternDesc.hitboxRadius  = cj_float(pat, "hitboxRadius", ec.patternDesc.hitboxRadius);
                ec.patternDesc.lifeTime      = cj_float(pat, "lifeTime", ec.patternDesc.lifeTime);
            }
        }
    }
}

EnemyScManager::EnemyScManager() = default;
EnemyScManager::~EnemyScManager() = default;

//Pattern parser (mirrors LevelManager::parse_behavior attack parsing)

EmitterConfig EnemyScManager::parse_pattern(cJSON* pat_json) {
    EmitterConfig ec;
    ec.startDelay    = cj_float(pat_json, "time", 0.0f);
    ec.emitInterval  = cj_float(pat_json, "interval", 1.0f);
    ec.burstCount    = cj_int(pat_json, "burst", 1);
    ec.burstInterval = cj_float(pat_json, "burstInterval", 0.05f);

    cJSON* pd = cJSON_GetObjectItem(pat_json, "pattern");
    if (pd && cJSON_IsObject(pd)) {
        const char* ptype = cj_str(pd, "type");
        // String → int pattern type (same table as LevelManager)
        if (ptype) {
            if      (strcmp(ptype, "fan_aimed")           == 0) ec.patternDesc.patternType = 0;
            else if (strcmp(ptype, "fan")                 == 0) ec.patternDesc.patternType = 1;
            else if (strcmp(ptype, "aimed")               == 0) ec.patternDesc.patternType = 2;
            else if (strcmp(ptype, "ring")                == 0) ec.patternDesc.patternType = 3;
            else if (strcmp(ptype, "ring_aimed")          == 0) ec.patternDesc.patternType = 4;
            else if (strcmp(ptype, "spiral")              == 0) ec.patternDesc.patternType = 5;
            else if (strcmp(ptype, "spread")              == 0) ec.patternDesc.patternType = 6;
            else if (strcmp(ptype, "ring_aimed_variable") == 0) ec.patternDesc.patternType = 7;
        }
        ec.patternDesc.mainCnt       = cj_int(pd, "mainCnt", 6);
        ec.patternDesc.subCnt        = cj_int(pd, "subCnt", 1);
        ec.patternDesc.angleOffset   = cj_float(pd, "spread", 0.0f);
        ec.patternDesc.angleInterval = cj_float(pd, "angleStep", 0.15f);
        ec.patternDesc.speed1        = cj_float(pd, "speed", 100.0f);
        ec.patternDesc.speed2        = cj_float(pd, "speedStep", 50.0f);
        ec.patternDesc.spriteID      = cj_int(pd, "spriteID", 0);
        ec.patternDesc.hitboxRadius  = cj_float(pd, "hitboxRadius", 4.0f);
        ec.patternDesc.lifeTime      = cj_float(pd, "lifeTime", 6.0f);
    }
    return ec;
}

//Loading

bool EnemyScManager::load(int stage) {
    reset();
    std::ostringstream ss;
    ss << stage;
    std::string path = "level/sc/stage" + ss.str() + ".json";
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "EnemyScManager: cannot open " << path << std::endl;
        return false;
    }

    std::string json_str;
    std::string line;
    while (std::getline(file, line))
        json_str += line;
    file.close();

    cJSON* root = cJSON_Parse(json_str.c_str());
    if (!root) {
        std::cerr << "EnemyScManager: JSON parse error in " << path << std::endl;
        return false;
    }

    // Parse spellcards array
    cJSON* sc_arr = cJSON_GetObjectItem(root, "spellcards");
    if (!sc_arr || !cJSON_IsArray(sc_arr)) {
        std::cerr << "EnemyScManager: no spellcards array in " << path << std::endl;
        cJSON_Delete(root);
        return false;
    }

    int n = cJSON_GetArraySize(sc_arr);
    for (int i = 0; i < n; i++) {
        cJSON* entry = cJSON_GetArrayItem(sc_arr, i);
        if (!entry || !cJSON_IsObject(entry)) continue;

        SpellcardDef sc;
        sc.id      = cj_int(entry, "id", i);
        sc.hp      = cj_int(entry, "hp", 2000);
        sc.timeout = cj_int(entry, "timeout", 45);

        const char* name = cj_str(entry, "name");
        sc.name = name ? name : "Spellcard";
        const char* scName = cj_str(entry, "scName");
        sc.scName = scName ? scName : sc.name;

        // Parse patterns array
        cJSON* pats = cJSON_GetObjectItem(entry, "patterns");
        if (pats && cJSON_IsArray(pats)) {
            int pn = cJSON_GetArraySize(pats);
            for (int p = 0; p < pn; p++) {
                cJSON* pobj = cJSON_GetArrayItem(pats, p);
                if (pobj && cJSON_IsObject(pobj)) {
                    sc.patterns.push_back(parse_pattern(pobj));
                }
            }
        }

        // Apply difficulty overrides (uses global gameDifficulty)
        apply_spellcard_difficulty(sc, entry);

        spellcards_.push_back(sc);
    }

    cJSON_Delete(root);
    std::cout << "EnemyScManager: loaded " << spellcards_.size()
              << " spellcards from " << path << std::endl;
    return true;
}

//Queries

const SpellcardDef* EnemyScManager::current() const {
    if (!active_ || current_index_ < 0 || current_index_ >= int(spellcards_.size()))
        return nullptr;
    return &spellcards_[current_index_];
}

const SpellcardDef* EnemyScManager::get(int index) const {
    if (index < 0 || index >= int(spellcards_.size()))
        return nullptr;
    return &spellcards_[index];
}

const std::string& EnemyScManager::current_name() const {
    static const std::string empty;
    const SpellcardDef* sc = current();
    return sc ? sc->name : empty;
}

bool EnemyScManager::is_timeout() const {
    if (!active_) return false;
    const SpellcardDef* sc = current();
    return sc && timer_ >= float(sc->timeout);
}

//Lifecycle

bool EnemyScManager::start(int index) {
    if (index < 0 || index >= int(spellcards_.size()))
        return false;

    current_index_ = index;
    timer_ = 0.0f;
    current_hp_ = spellcards_[index].hp;
    active_ = true;

    std::cout << "Spellcard " << index << " started: \""
              << spellcards_[index].name << "\" (HP="
              << current_hp_ << ", timeout="
              << spellcards_[index].timeout << "s)" << std::endl;
    return true;
}

void EnemyScManager::update(float dt) {
    if (!active_) return;
    timer_ += dt;
}

void EnemyScManager::damage(int dmg) {
    if (!active_) return;
    current_hp_ -= dmg;
    if (current_hp_ < 0) current_hp_ = 0;
}

int EnemyScManager::end(bool captured) {
    if (!active_) return 0;

    const SpellcardDef* sc = current();
    int hp_delta = sc ? (sc->hp - current_hp_) : 0;

    std::cout << "Spellcard " << current_index_
              << " ended: " << (captured ? "CAPTURED" : "TIMEOUT")
              << " (damage dealt: " << hp_delta << ")" << std::endl;

    active_ = false;
    return hp_delta;
}

void EnemyScManager::reset() {
    spellcards_.clear();
    current_index_ = 0;
    timer_ = 0.0f;
    current_hp_ = 0;
    active_ = false;
}

//Timer queries

float EnemyScManager::time_remaining() const {
    if (!active_) return 0.0f;
    const SpellcardDef* sc = current();
    if (!sc) return 0.0f;
    float rem = float(sc->timeout) - timer_;
    return rem > 0.0f ? rem : 0.0f;
}

//Static bullet clearing

void EnemyScManager::clear_bullets(EnemyBulletManager* mgr) {
    if (mgr) mgr->despawn_all_for_spellcard();
}
