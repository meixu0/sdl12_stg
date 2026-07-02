#include "LevelManager.h"
#include "GameSettings.h"
#include "ItemManager.h"
#include "ItemType.h"
#include <cstring>
#include <map>

// ── String→enum lookup tables (for V2 format) ──────────────────────────────

static int str_to_move_pattern(const char* s) {
    if (!s) return LINER;
    if (strcmp(s, "linear")            == 0) return LINER;
    if (strcmp(s, "sine")              == 0) return SINWAVE;
    if (strcmp(s, "bezier")            == 0) return BEZIER;
    if (strcmp(s, "stop_and_go")       == 0) return STOPANDGO;
    if (strcmp(s, "homing")            == 0) return HOMING;
    if (strcmp(s, "interception")      == 0) return INTERCEPTION;
    if (strcmp(s, "position_interp")   == 0) return POSITION_INTERP;
    return LINER;
}

static int str_to_pattern_type(const char* s) {
    if (!s) return 0;
    if (strcmp(s, "fan_aimed")          == 0) return 0;
    if (strcmp(s, "fan")                == 0) return 1;
    if (strcmp(s, "aimed")              == 0) return 2;
    if (strcmp(s, "ring")               == 0) return 3;
    if (strcmp(s, "ring_aimed")         == 0) return 4;
    if (strcmp(s, "spiral")             == 0) return 5;
    if (strcmp(s, "spread")             == 0) return 6;
    if (strcmp(s, "ring_aimed_variable")== 0) return 7;
    return 0;
}

// ── Behavior name → enemyType mapping ──────────────────────────────────────

static int behavior_to_enemy_type(const char* name) {
    if (!name) return ZAKO_BLUE;
    if (strcmp(name, "boss_entry")       == 0) return BOSS_ENTRY;
    if (strcmp(name, "boss_rumia")       == 0) return BOSS_RUMIA;
    if (strcmp(name, "boss_final")       == 0) return BOSS_FINAL;
    if (strcmp(name, "boss_daiyousei")   == 0) return BOSS_DAIYOUSEI;
    if (strcmp(name, "boss_cirno")       == 0) return BOSS_CIRNO;
    if (strcmp(name, "boss_patchouli")   == 0) return BOSS_PATCHOULI;
    if (strcmp(name, "boss_sakuya")      == 0) return BOSS_SAKUYA;
    if (strcmp(name, "boss_remilia")     == 0) return BOSS_REMILIA;
    if (strcmp(name, "boss_flandre")     == 0) return BOSS_FLANDRE;
    if (strcmp(name, "boss_extra")       == 0) return BOSS_EXTRA;
    if (strcmp(name, "fairy_red")        == 0) return FAIRY_RED;
    if (strcmp(name, "fairy_green")      == 0) return FAIRY_GREEN;
    if (strcmp(name, "fairy_blue")       == 0) return FAIRY_BLUE;
    // Boss prefix fallback
    if (strncmp(name, "boss_", 5) == 0) return BOSS_ENTRY;
    // Fairy prefix fallback
    if (strncmp(name, "fairy_", 6) == 0 && name[6] != '\0') return FAIRY_RED;
    return ZAKO_BLUE;
}

// ── Safe cJSON access helpers ──────────────────────────────────────────────

static float cjson_float(cJSON* obj, const char* key, float def) {
    cJSON* v = cJSON_GetObjectItem(obj, key);
    return (v && cJSON_IsNumber(v)) ? (float)v->valuedouble : def;
}

static int cjson_int(cJSON* obj, const char* key, int def) {
    cJSON* v = cJSON_GetObjectItem(obj, key);
    return (v && cJSON_IsNumber(v)) ? (int)v->valuedouble : def;
}

static const char* cjson_str(cJSON* obj, const char* key) {
    cJSON* v = cJSON_GetObjectItem(obj, key);
    return (v && cJSON_IsString(v)) ? v->valuestring : NULL;
}

// ── V2 format data structures ──────────────────────────────────────────────

struct BehaviorDef {
    int hp = 50;
    int hitboxW = 32, hitboxH = 32;
    int score = 100;
    float halfLife = 0.0f;
    int movePattern = LINER;
    float speedX = 0.0f, speedY = 120.0f;
    float targetX = -1.0f, targetY = -1.0f;  // stop_and_go target; <0 means "use spawn pos"
    float moveDuration = 0.0f;   // position_interp duration (seconds)
    int   moveEasing = 0;        // position_interp easing: 0=linear, 4=decelerate
    float vertAmp = 0.0f, vertPeriod = 1.0f;
    float horizAmp = 0.0f, horizPeriod = 1.0f;
    float angularVelocity = 0.0f;
    float minPlayerDist = 80.0f;
    float accel = 0.0f;
    float lifeTime = 12.0f;
    bool isMidboss = false;
    std::string itemDrop;
    std::vector<EmitterConfig> emitters;
};

// ── Difficulty name table (indexed by gameDifficulty 0-4) ───────────────────
static const char* DIFFICULTY_KEYS[] = {
    "easy", "normal", "hard", "lunatic", "extra"
};

// ── Apply difficulty overrides from a JSON "difficulty" object ────────────
// Mutates `def` in place by overlaying the current difficulty's overrides.

static void apply_behavior_difficulty(BehaviorDef& def, cJSON* diff_obj) {
    if (!diff_obj || !cJSON_IsObject(diff_obj)) return;

    int diff_level = gameDifficulty;
    if (diff_level < 0 || diff_level > 4) diff_level = DIFFICULTY_NORM;

    const char* key = DIFFICULTY_KEYS[diff_level];
    cJSON* over = cJSON_GetObjectItem(diff_obj, key);
    if (!over || !cJSON_IsObject(over)) return;

    // Scalar overrides
    cJSON* v;
    if ((v = cJSON_GetObjectItem(over, "hp"))       && cJSON_IsNumber(v)) def.hp       = (int)v->valuedouble;
    if ((v = cJSON_GetObjectItem(over, "score"))     && cJSON_IsNumber(v)) def.score    = (int)v->valuedouble;
    if ((v = cJSON_GetObjectItem(over, "halfLife"))  && cJSON_IsNumber(v)) def.halfLife = (float)v->valuedouble;
    if ((v = cJSON_GetObjectItem(over, "lifeTime"))  && cJSON_IsNumber(v)) def.lifeTime = (float)v->valuedouble;

    // Hitbox override
    cJSON* hb = cJSON_GetObjectItem(over, "hitbox");
    if (hb && cJSON_IsArray(hb) && cJSON_GetArraySize(hb) >= 2) {
        def.hitboxW = (int)cJSON_GetArrayItem(hb, 0)->valuedouble;
        def.hitboxH = (int)cJSON_GetArrayItem(hb, 1)->valuedouble;
    }

    // Movement overrides (merge into current move)
    cJSON* move = cJSON_GetObjectItem(over, "move");
    if (move && cJSON_IsObject(move)) {
        const char* mt = cjson_str(move, "type");
        if (mt) def.movePattern = str_to_move_pattern(mt);

        float s = cjson_float(move, "speed", -1.0f);
        if (s >= 0) { def.speedX = s; def.speedY = s; }
        def.speedX = cjson_float(move, "speedX", def.speedX);
        def.speedY = cjson_float(move, "speedY", def.speedY);
        def.vertAmp         = cjson_float(move, "vertAmp", def.vertAmp);
        def.vertPeriod      = cjson_float(move, "vertPeriod", def.vertPeriod);
        def.horizAmp        = cjson_float(move, "horizAmp", def.horizAmp);
        def.horizPeriod     = cjson_float(move, "horizPeriod", def.horizPeriod);
        def.angularVelocity = cjson_float(move, "angularVelocity", def.angularVelocity);
        def.minPlayerDist   = cjson_float(move, "minDist", def.minPlayerDist);
        def.accel           = cjson_float(move, "accel", def.accel);
    }

    // Attack-pattern overrides — keyed by index string "0", "1", etc.
    cJSON* atk = cJSON_GetObjectItem(over, "attack");
    if (atk && cJSON_IsObject(atk)) {
        cJSON* child = NULL;
        cJSON_ArrayForEach(child, atk) {
            if (!child || !cJSON_IsObject(child)) continue;
            int idx = atoi(child->string);
            if (idx < 0 || idx >= (int)def.emitters.size()) continue;

            EmitterConfig& ec = def.emitters[idx];
            ec.emitInterval  = cjson_float(child, "interval", ec.emitInterval);
            ec.startDelay    = cjson_float(child, "time", ec.startDelay);
            ec.burstCount    = cjson_int(child, "burst", ec.burstCount);
            ec.burstInterval = cjson_float(child, "burstInterval", ec.burstInterval);

            cJSON* pat = cJSON_GetObjectItem(child, "pattern");
            if (pat && cJSON_IsObject(pat)) {
                const char* ptype = cjson_str(pat, "type");
                if (ptype) ec.patternDesc.patternType = str_to_pattern_type(ptype);
                ec.patternDesc.mainCnt       = cjson_int(pat, "mainCnt", ec.patternDesc.mainCnt);
                ec.patternDesc.subCnt        = cjson_int(pat, "subCnt", ec.patternDesc.subCnt);
                ec.patternDesc.angleOffset   = cjson_float(pat, "spread", ec.patternDesc.angleOffset);
                ec.patternDesc.angleInterval = cjson_float(pat, "angleStep", ec.patternDesc.angleInterval);
                ec.patternDesc.speed1        = cjson_float(pat, "speed", ec.patternDesc.speed1);
                ec.patternDesc.speed2        = cjson_float(pat, "speedStep", ec.patternDesc.speed2);
                ec.patternDesc.spriteID      = cjson_int(pat, "spriteID", ec.patternDesc.spriteID);
                ec.patternDesc.hitboxRadius  = cjson_float(pat, "hitboxRadius", ec.patternDesc.hitboxRadius);
                ec.patternDesc.lifeTime      = cjson_float(pat, "lifeTime", ec.patternDesc.lifeTime);
            }
        }
    }
}

// ── Helper: parse a behavior JSON object → BehaviorDef ─────────────────────

static BehaviorDef parse_behavior(cJSON* beh) {
    BehaviorDef def;
    if (!beh || !cJSON_IsObject(beh)) return def;

    def.hp       = cjson_int(beh, "hp", 50);
    def.score    = cjson_int(beh, "score", 100);
    def.halfLife = cjson_float(beh, "halfLife", 0.0f);
    def.lifeTime = cjson_float(beh, "lifeTime", 12.0f);

    // hitbox array [w, h]
    cJSON* hb = cJSON_GetObjectItem(beh, "hitbox");
    if (hb && cJSON_IsArray(hb) && cJSON_GetArraySize(hb) >= 2) {
        def.hitboxW = (int)cJSON_GetArrayItem(hb, 0)->valuedouble;
        def.hitboxH = (int)cJSON_GetArrayItem(hb, 1)->valuedouble;
    }

    // move object
    cJSON* move = cJSON_GetObjectItem(beh, "move");
    if (move && cJSON_IsObject(move)) {
        const char* mt = cjson_str(move, "type");
        if (mt) def.movePattern = str_to_move_pattern(mt);

        def.speedX = cjson_float(move, "speedX", cjson_float(move, "speed", 0.0f));
        def.speedY = cjson_float(move, "speedY", cjson_float(move, "speed", 120.0f));

        def.vertAmp         = cjson_float(move, "vertAmp", 0.0f);
        def.vertPeriod      = cjson_float(move, "vertPeriod", 1.0f);
        def.horizAmp        = cjson_float(move, "horizAmp", 0.0f);
        def.horizPeriod     = cjson_float(move, "horizPeriod", 1.0f);
        def.angularVelocity = cjson_float(move, "angularVelocity", 0.0f);
        def.minPlayerDist   = cjson_float(move, "minDist", 80.0f);
        def.accel           = cjson_float(move, "accel", 0.0f);

        // target for stop_and_go / position_interp
        cJSON* tgt = cJSON_GetObjectItem(move, "target");
        if (tgt && cJSON_IsArray(tgt) && cJSON_GetArraySize(tgt) >= 2) {
            def.targetX = (float)cJSON_GetArrayItem(tgt, 0)->valuedouble;
            def.targetY = (float)cJSON_GetArrayItem(tgt, 1)->valuedouble;
        }

        // position_interp specific: duration (frames) and easing
        def.moveDuration = cjson_float(move, "duration", 0.0f) / 60.0f;  // frames → seconds
        def.moveEasing   = cjson_int(move, "easing", 0);
    }

    // attack array → emitter configs
    cJSON* attack = cJSON_GetObjectItem(beh, "attack");
    if (attack && cJSON_IsArray(attack)) {
        int n = cJSON_GetArraySize(attack);
        for (int i = 0; i < n; i++) {
            cJSON* at = cJSON_GetArrayItem(attack, i);
            if (!at || !cJSON_IsObject(at)) continue;

            EmitterConfig ec;
            memset(&ec, 0, sizeof(ec));
            ec.startDelay    = cjson_float(at, "time", 0.0f);
            ec.emitInterval  = cjson_float(at, "interval", 1.0f);
            ec.burstCount    = cjson_int(at, "burst", 1);
            ec.burstInterval = cjson_float(at, "burstInterval", 0.05f);

            cJSON* pat = cJSON_GetObjectItem(at, "pattern");
            if (pat && cJSON_IsObject(pat)) {
                const char* ptype = cjson_str(pat, "type");
                if (ptype) ec.patternDesc.patternType = str_to_pattern_type(ptype);
                ec.patternDesc.mainCnt       = cjson_int(pat, "mainCnt", 6);
                ec.patternDesc.subCnt        = cjson_int(pat, "subCnt", 1);
                ec.patternDesc.angleOffset   = cjson_float(pat, "spread", 0.0f);
                ec.patternDesc.angleInterval = cjson_float(pat, "angleStep", 0.15f);
                ec.patternDesc.speed1        = cjson_float(pat, "speed", 100.0f);
                ec.patternDesc.speed2        = cjson_float(pat, "speedStep", 50.0f);
                ec.patternDesc.spriteID      = cjson_int(pat, "spriteID", 0);
                ec.patternDesc.hitboxRadius  = cjson_float(pat, "hitboxRadius", 4.0f);
                ec.patternDesc.lifeTime      = cjson_float(pat, "lifeTime", 6.0f);
            }
            def.emitters.push_back(ec);
        }
    }

    // death → itemDrop
    cJSON* death = cJSON_GetObjectItem(beh, "death");
    if (death && cJSON_IsObject(death)) {
        const char* drop = cjson_str(death, "itemDrop");
        if (drop) def.itemDrop = drop;
    }

    // flags array
    cJSON* flags = cJSON_GetObjectItem(beh, "flags");
    if (flags && cJSON_IsArray(flags)) {
        for (int i = 0; i < cJSON_GetArraySize(flags); i++) {
            cJSON* f = cJSON_GetArrayItem(flags, i);
            if (f && cJSON_IsString(f) && strcmp(f->valuestring, "midboss") == 0)
                def.isMidboss = true;
        }
    }

    // Apply difficulty overrides (uses global gameDifficulty)
    cJSON* diff_obj = cJSON_GetObjectItem(beh, "difficulty");
    if (diff_obj && cJSON_IsObject(diff_obj))
        apply_behavior_difficulty(def, diff_obj);

    return def;
}

// ── Helper: apply timeline overrides to an EnemyConfig ─────────────────────

static void apply_overrides(EnemyConfig& config, cJSON* overrides) {
    if (!overrides || !cJSON_IsObject(overrides)) return;

    cJSON* hp_ov = cJSON_GetObjectItem(overrides, "hp");
    if (hp_ov && cJSON_IsNumber(hp_ov)) config.hp = (int)hp_ov->valuedouble;

    cJSON* half_ov = cJSON_GetObjectItem(overrides, "halfLife");
    if (half_ov && cJSON_IsNumber(half_ov)) config.halfLife = (float)half_ov->valuedouble;

    cJSON* hitbox_ov = cJSON_GetObjectItem(overrides, "hitbox");
    if (hitbox_ov && cJSON_IsArray(hitbox_ov) && cJSON_GetArraySize(hitbox_ov) >= 2) {
        config.hitboxWidth  = (int)cJSON_GetArrayItem(hitbox_ov, 0)->valuedouble;
        config.hitboxHeight = (int)cJSON_GetArrayItem(hitbox_ov, 1)->valuedouble;
    }

    cJSON* move_ov = cJSON_GetObjectItem(overrides, "move");
    if (move_ov && cJSON_IsObject(move_ov)) {
        const char* mt = cjson_str(move_ov, "type");
        if (mt) config.movePattern = str_to_move_pattern(mt);
        config.speedX = cjson_float(move_ov, "speedX", config.speedX);
        config.speedY = cjson_float(move_ov, "speedY", config.speedY);
    }
}

// ── V2 parser: read behaviors + timeline → create enemies ──────────────────

void LevelManager::init_enemy_pool_v2() {
    clear_enemy_pool();

    // 1. Parse behaviors → map
    std::map<std::string, BehaviorDef> behaviorMap;
    cJSON* behaviors = cJSON_GetObjectItem(stage_enemies_data, "behaviors");
    if (behaviors && cJSON_IsObject(behaviors)) {
        cJSON* child = NULL;
        cJSON_ArrayForEach(child, behaviors) {
            if (child && cJSON_IsObject(child)) {
                const char* name = child->string ? child->string : "";
                BehaviorDef def = parse_behavior(child);
                behaviorMap[name] = def;
            }
        }
    }

    // 2. Parse timeline
    cJSON* timeline = cJSON_GetObjectItem(stage_enemies_data, "timeline");
    if (!timeline || !cJSON_IsArray(timeline)) {
        std::cerr << "Error: no timeline array in V2 format" << std::endl;
        return;
    }

    float accumTime = 0.0f;
    int tlLen = cJSON_GetArraySize(timeline);
    for (int i = 0; i < tlLen; i++) {
        cJSON* cmd = cJSON_GetArrayItem(timeline, i);
        if (!cmd || !cJSON_IsObject(cmd)) continue;

        cJSON* delta_j = cJSON_GetObjectItem(cmd, "delta");
        if (delta_j && cJSON_IsNumber(delta_j))
            accumTime += (float)delta_j->valuedouble;

        cJSON* cmdType = cJSON_GetObjectItem(cmd, "cmd");
        if (!cmdType || !cJSON_IsString(cmdType)) continue;
        const char* command = cmdType->valuestring;

        if (strcmp(command, "spawn") == 0) {
            cJSON* behName = cJSON_GetObjectItem(cmd, "behavior");
            cJSON* posArr  = cJSON_GetObjectItem(cmd, "pos");
            if (!behName || !cJSON_IsString(behName)) continue;
            if (!posArr  || !cJSON_IsArray(posArr))   continue;

            const char* bname = behName->valuestring;
            float px = (float)cJSON_GetArrayItem(posArr, 0)->valuedouble;
            float py = (float)cJSON_GetArrayItem(posArr, 1)->valuedouble;

            BehaviorDef def;
            std::map<std::string, BehaviorDef>::iterator it = behaviorMap.find(bname);
            if (it != behaviorMap.end()) {
                def = it->second;
            } else {
                // Unknown behavior — this shouldn't happen
                std::cerr << "Warning: unknown behavior '" << bname << "'" << std::endl;
            }

            // Build config
            EnemyConfig config;
            memset(&config, 0, sizeof(config));
            config.hp            = def.hp;
            config.hitboxWidth   = def.hitboxW;
            config.hitboxHeight  = def.hitboxH;
            config.speedX        = def.speedX;
            config.speedY        = def.speedY;
            config.movePattern   = def.movePattern;
            config.halfLife      = def.halfLife;
            config.angularVelocity = def.angularVelocity;
            config.minPlayerDist = def.minPlayerDist;
            config.acceleration  = def.accel;
            config.vertAmplitude  = def.vertAmp;
            config.vertPeriod     = def.vertPeriod;
            config.horizAmplitude = def.horizAmp;
            config.horizPeriod    = def.horizPeriod;

            // Apply overrides
            cJSON* overrides = cJSON_GetObjectItem(cmd, "overrides");
            apply_overrides(config, overrides);

            config.emergeTime   = accumTime;
            config.durationTime = def.lifeTime;
            config.enemyType    = behavior_to_enemy_type(bname);
            config.isMidboss    = def.isMidboss;
            config.targetX      = (def.targetX >= 0) ? def.targetX : px;
            config.targetY      = (def.targetY >= 0) ? def.targetY : py;
            config.startX       = px;
            config.startY       = py;

            Enemy* enemy = new Enemy();
            // Copy emitters from behavior def
            enemy->emitterConfig = def.emitters;
            enemy->init(config, px, py);
            if (def.movePattern == POSITION_INTERP && def.moveDuration > 0) {
                enemy->start_position_interp(def.moveDuration, def.moveEasing, config.targetX, config.targetY);
            }
            enemy_pool.push_back(enemy);
        }
        else if (strcmp(command, "wave") == 0) {
            cJSON* behName = cJSON_GetObjectItem(cmd, "behavior");
            cJSON* posArr  = cJSON_GetObjectItem(cmd, "pos");
            cJSON* countJ  = cJSON_GetObjectItem(cmd, "count");
            cJSON* dxJ     = cJSON_GetObjectItem(cmd, "dx");
            cJSON* gapJ    = cJSON_GetObjectItem(cmd, "gap");
            if (!behName || !cJSON_IsString(behName)) continue;
            if (!posArr  || !cJSON_IsArray(posArr))   continue;

            const char* bname = behName->valuestring;
            float baseX = (float)cJSON_GetArrayItem(posArr, 0)->valuedouble;
            float baseY = (float)cJSON_GetArrayItem(posArr, 1)->valuedouble;
            int    count   = countJ && cJSON_IsNumber(countJ) ? (int)countJ->valuedouble : 1;
            float  dx      = dxJ    && cJSON_IsNumber(dxJ)    ? (float)dxJ->valuedouble    : 0.0f;
            float  gap     = gapJ   && cJSON_IsNumber(gapJ)   ? (float)gapJ->valuedouble   : 0.0f;

            // Look up behavior
            BehaviorDef def;
            std::map<std::string, BehaviorDef>::iterator it = behaviorMap.find(bname);
            if (it != behaviorMap.end()) {
                def = it->second;
            }

            EnemyConfig config;
            memset(&config, 0, sizeof(config));
            config.hp            = def.hp;
            config.hitboxWidth   = def.hitboxW;
            config.hitboxHeight  = def.hitboxH;
            config.speedX        = def.speedX;
            config.speedY        = def.speedY;
            config.movePattern   = def.movePattern;
            config.halfLife      = def.halfLife;
            config.angularVelocity = def.angularVelocity;
            config.minPlayerDist = def.minPlayerDist;
            config.acceleration  = def.accel;
            config.vertAmplitude  = def.vertAmp;
            config.vertPeriod     = def.vertPeriod;
            config.horizAmplitude = def.horizAmp;
            config.horizPeriod    = def.horizPeriod;
            config.durationTime   = def.lifeTime;
            config.isMidboss      = def.isMidboss;

            for (int k = 0; k < count; k++) {
                float ex = baseX + k * dx;
                config.emergeTime = accumTime + k * gap;
                config.startX = ex;
                config.startY = baseY;
                // stop_and_go target: use behavior target + dx offset, or spawn pos (stay put)
                config.targetX = (def.targetX >= 0) ? (def.targetX + k * dx) : ex;
                config.targetY = (def.targetY >= 0) ? def.targetY : baseY;

                Enemy* enemy = new Enemy();
                enemy->emitterConfig = def.emitters;
                enemy->init(config, ex, baseY);
                if (def.movePattern == POSITION_INTERP && def.moveDuration > 0) {
                    enemy->start_position_interp(def.moveDuration, def.moveEasing, config.targetX, config.targetY);
                }
                enemy_pool.push_back(enemy);
            }
        }
        // "spellcard" and "defeat" commands: ignored at pool-creation time;
        // spellcard phases are driven by EnemyScManager at runtime.
    }

    // ── Link spellcards to boss enemy ─────────────────────────────────────
    bossEnemyIndex_ = -1;
    spellcardTriggerHp_ = 0;
    for (size_t i = 0; i < enemy_pool.size(); i++) {
        Enemy* e = enemy_pool[i];
        if (!e) continue;
        if (e->get_enemy_type() >= BOSS_RUMIA && !e->get_is_midboss()) {
            bossEnemyIndex_ = (int)i;
            e->set_duration_time(999999.0f);

            // Parse move_bounds from behavior if present
            cJSON* behaviors = cJSON_GetObjectItem(stage_enemies_data, "behaviors");
            if (behaviors && cJSON_IsObject(behaviors)) {
                cJSON* child = NULL;
                cJSON_ArrayForEach(child, behaviors) {
                    if (!child || !cJSON_IsObject(child)) continue;
                    int etype = behavior_to_enemy_type(child->string ? child->string : "");
                    if (etype == e->get_enemy_type()) {
                        cJSON* mb = cJSON_GetObjectItem(child, "move_bounds");
                        if (mb && cJSON_IsArray(mb) && cJSON_GetArraySize(mb) >= 4) {
                            float mminX = (float)cJSON_GetArrayItem(mb, 0)->valuedouble;
                            float mminY = (float)cJSON_GetArrayItem(mb, 1)->valuedouble;
                            float mmaxX = (float)cJSON_GetArrayItem(mb, 2)->valuedouble;
                            float mmaxY = (float)cJSON_GetArrayItem(mb, 3)->valuedouble;
                            e->set_move_bounds(mminX, mminY, mmaxX, mmaxY);
                        }

                        // ── Parse ECL phase sequence from behavior ──
                        cJSON* phases_j = cJSON_GetObjectItem(child, "phases");
                        if (phases_j && cJSON_IsArray(phases_j)) {
                            std::vector<BossPhaseDef> phases;
                            int np = cJSON_GetArraySize(phases_j);
                            for (int pi = 0; pi < np; pi++) {
                                cJSON* ph = cJSON_GetArrayItem(phases_j, pi);
                                if (!ph || !cJSON_IsObject(ph)) continue;

                                BossPhaseDef phase;
                                phase.duration  = cjson_float(ph, "duration", 1.0f);
                                phase.moveDuration = cjson_float(ph, "move_duration", 0.0f) / 60.0f;
                                phase.moveEasing   = cjson_int(ph, "move_easing", 4);

                                cJSON* mv = cJSON_GetObjectItem(ph, "move_after");
                                if (mv && cJSON_IsArray(mv) && cJSON_GetArraySize(mv) >= 2) {
                                    phase.moveToX = (float)cJSON_GetArrayItem(mv, 0)->valuedouble;
                                    phase.moveToY = (float)cJSON_GetArrayItem(mv, 1)->valuedouble;
                                }

                                cJSON* pats = cJSON_GetObjectItem(ph, "patterns");
                                if (pats && cJSON_IsArray(pats)) {
                                    int npat = cJSON_GetArraySize(pats);
                                    for (int pj = 0; pj < npat; pj++) {
                                        cJSON* at = cJSON_GetArrayItem(pats, pj);
                                        if (!at || !cJSON_IsObject(at)) continue;
                                        EmitterConfig ec;
                                        memset(&ec, 0, sizeof(ec));
                                        ec.startDelay    = cjson_float(at, "time", 0.0f);
                                        ec.emitInterval  = cjson_float(at, "interval", 1.0f);
                                        ec.burstCount    = cjson_int(at, "burst", 1);
                                        ec.burstInterval = cjson_float(at, "burstInterval", 0.05f);
                                        cJSON* ptn = cJSON_GetObjectItem(at, "pattern");
                                        if (ptn && cJSON_IsObject(ptn)) {
                                            const char* ptype = cjson_str(ptn, "type");
                                            if (ptype) ec.patternDesc.patternType = str_to_pattern_type(ptype);
                                            ec.patternDesc.mainCnt       = cjson_int(ptn, "mainCnt", 6);
                                            ec.patternDesc.subCnt        = cjson_int(ptn, "subCnt", 1);
                                            ec.patternDesc.angleOffset   = cjson_float(ptn, "spread", 0.0f);
                                            ec.patternDesc.angleInterval = cjson_float(ptn, "angleStep", 0.15f);
                                            ec.patternDesc.speed1        = cjson_float(ptn, "speed", 100.0f);
                                            ec.patternDesc.speed2        = cjson_float(ptn, "speedStep", 50.0f);
                                            ec.patternDesc.spriteID      = cjson_int(ptn, "spriteID", 0);
                                            ec.patternDesc.hitboxRadius  = cjson_float(ptn, "hitboxRadius", 4.0f);
                                            ec.patternDesc.lifeTime      = cjson_float(ptn, "lifeTime", 6.0f);
                                        }
                                        phase.patterns.push_back(ec);
                                    }
                                }
                                phases.push_back(phase);
                            }
                            if (!phases.empty()) {
                                e->set_phases(phases);
                                e->start_phases();
                                std::cout << "Boss loaded with " << phases.size() << " ECL attack phases" << std::endl;
                            }
                        }
                        break;
                    }
                }
            }

            // Load spellcard data but DON'T start phase 0 immediately
            // Boss uses its own attack patterns from the behavior initially.
            // Spellcard phase starts when boss HP drops below the threshold.
            if (scManager_.load(current_stage)) {
                e->link_spellcard_manager(&scManager_);
                if (scManager_.count() > 0) {
                    const SpellcardDef* firstSc = scManager_.get(0);
                    spellcardTriggerHp_ = (firstSc) ? firstSc->hp : 0;
                }
                std::cout << "Boss loaded with " << scManager_.count()
                          << " spellcard(s), trigger at HP ≤ " << spellcardTriggerHp_
                          << std::endl;
            } else {
                std::cout << "No spellcard data found for stage " << current_stage << std::endl;
            }
            break;
        }
    }

    std::cout << "Enemy pool initialized (V2 format) with " << enemy_pool.size() << " enemies" << std::endl;
}

LevelManager::LevelManager() : stage_enemies_data(NULL), current_stage(1), stage_state(STAGE_LOADING), isClearingForMidboss(true), midbossIndex_(1e5), bullet_mgr_(NULL), item_mgr_(NULL), bgm_music_(NULL), bossEnemyIndex_(-1), spellcardTriggerHp_(0), spellcardEntryTimer_(0.0f), nonSpellcardTimer_(0.0f), midbossDefeatedProcessed_(false) {
}

void LevelManager::bgm_play(int stage, bool is_boss) {
    // Stop previous BGM
    if (bgm_music_ != NULL) {
        Mix_HaltMusic();
        // SDL_mixer 1.2 + FluidSynth on Linux has a known bug where
        // Mix_FreeMusic on MIDI files crashes inside delete_fluid_synth().
        // Only free non-MIDI music; MIDI tracks are leaked (a few KB each).
        if (Mix_GetMusicType(bgm_music_) != MUS_MID) {
            Mix_FreeMusic(bgm_music_);
        }
        bgm_music_ = NULL;
    }

    // th07_02, th07_04, ... = stage themes (道中)
    // th07_03, th07_05, ... = boss themes
    // Map: stage N → stage theme = th07_(N*2), boss theme = th07_(N*2+1)
    const char* music_file = NULL;
    switch (stage) {
        case 1: music_file = is_boss ? "res/music/th07_03.mid" : "res/music/th07_02.mid"; break;
        case 2: music_file = is_boss ? "res/music/th07_05.mid" : "res/music/th07_04.mid"; break;
        case 3: music_file = is_boss ? "res/music/th07_07.mid" : "res/music/th07_06.mid"; break;
        case 4: music_file = is_boss ? "res/music/th07_09.mid" : "res/music/th07_08.mid"; break;
        case 5: music_file = is_boss ? "res/music/th07_11.mid" : "res/music/th07_10.mid"; break;
        case 6: music_file = is_boss ? "res/music/th07_13.mid" : "res/music/th07_12.mid"; break;
        case 7: music_file = is_boss ? "res/music/th07_15.mid" : "res/music/th07_14.mid"; break;
        case 8: music_file = is_boss ? "res/music/th07_17.mid" : "res/music/th07_16.mid"; break;
    }

    if (music_file != NULL) {
        bgm_music_ = Mix_LoadMUS(music_file);
        if (bgm_music_ != NULL) {
            Mix_PlayMusic(bgm_music_, -1);
        }
    }
}

void LevelManager::start_stage() {
    stage_state = STAGE_RUNNING;
    if(current_stage != 1) bgm_play(current_stage, false);
    std::cout << "Stage " << current_stage << " started." << std::endl;
}

void LevelManager::trigger_boss() {
    stage_state = STAGE_BOSS;
    bgm_play(current_stage, true);
    std::cout << "Stage " << current_stage << " boss triggered." << std::endl;
}

void LevelManager::trigger_midboss() {
    stage_state = STAGE_MIDBOSS;
}

void LevelManager::clear_stage() {
    stage_state = STAGE_CLEAR;
    std::cout << "Stage " << current_stage << " cleared!" << std::endl;
    // TODO: 掉落结算、分数统计
}

void LevelManager::next_stage() {
    if (current_stage >= TOTAL_STAGES) {
        stage_state = STAGE_ALL_CLEAR;
        std::cout << "All stages cleared!" << std::endl;
        return;
    }
    clear_enemy_pool();
    current_stage++;
    load_stage(current_stage);
    init_enemy_pool();
    stage_state = STAGE_RUNNING;
    std::cout << "Stage " << current_stage << " started." << std::endl;
}

// TH06-style lifecycle transitions — clean, load, init, start/trigger in one step
void LevelManager::enter_stage(int stage) {
	clear_enemy_pool();
	load_stage(stage);
	init_enemy_pool();
	if (bullet_mgr_) set_bullet_manager_for_all(bullet_mgr_);
	start_stage();
}

void LevelManager::enter_boss(int stage) {
	clear_enemy_pool();
	load_boss_stage(stage);
	init_enemy_pool();
	if (bullet_mgr_) set_bullet_manager_for_all(bullet_mgr_);
	trigger_boss();
}

bool LevelManager::auto_transition(Uint32 frameCounter) {
	if (stage_state == STAGE_RUNNING) {
		// 检测当前关卡的所有敌人是否已生成并全部被击破
		for (size_t i = 0; i < enemy_pool.size(); ++i) {
			Enemy* e = enemy_pool[i];
			if (e == NULL) continue;
			if (e->has_pending_spawn()) return false;
			if (e->is_active()) return false;
		}
		enter_boss(current_stage);
		return true;
	}
	else if (stage_state == STAGE_BOSS) {
		// Spellcard boss: check if all phases done and boss is dead
		if (bossEnemyIndex_ >= 0) {
			Enemy* boss = enemy_pool[bossEnemyIndex_];
			if (boss && (boss->has_pending_spawn() || boss->is_active())) return false;
			if (scManager_.is_active()) return false;
			// Also check non-boss enemies (familiars etc.)
			for (size_t i = 0; i < enemy_pool.size(); ++i) {
				if ((int)i == bossEnemyIndex_) continue;
				Enemy* e = enemy_pool[i];
				if (e && (e->has_pending_spawn() || e->is_active())) return false;
			}
		} else {
			// Check all enemies for non-spellcard boss
			for (size_t i = 0; i < enemy_pool.size(); ++i) {
				Enemy* e = enemy_pool[i];
				if (e == NULL) continue;
				if (e->has_pending_spawn()) return false;
				if (e->is_active()) return false;
			}
		}
		clear_stage();
		return true;
	}
	else if (stage_state == STAGE_CLEAR) {
		// 自动进入下一关
		int next = current_stage + 1;
		if (next > TOTAL_STAGES) {
			stage_state = STAGE_ALL_CLEAR;
			clear_enemy_pool();
			std::cout << "All stages cleared!" << std::endl;
		} else {
			enter_stage(next);
		}
		return true;
	}
	return false;
}



// ── Spellcard phase management ────────────────────────────────────────────

void LevelManager::start_spellcard_phase(int phaseIndex) {
    if (bossEnemyIndex_ < 0 || bossEnemyIndex_ >= (int)enemy_pool.size()) return;
    Enemy* boss = enemy_pool[bossEnemyIndex_];
    if (!boss) return;

    const SpellcardDef* sc = scManager_.get(phaseIndex);
    if (!sc) {
        std::cerr << "Spellcard phase " << phaseIndex << " not found!" << std::endl;
        return;
    }

    scManager_.start(phaseIndex);

    // TH06: reset boss HP to spellcard threshold — gives the spellcard
    // its own HP pool instead of sharing the boss's remaining HP
    boss->set_spellcard_hp(sc->hp);

    // ECL Sub29: convert bullets to P-items, then clear for clean spellcard start
    convert_boss_bullets_to_p_items();
    EnemyScManager::clear_bullets(bullet_mgr_);

    // ECL Sub29: move boss to spellcard position (192, 140) over 120 frames with deceleration
    boss->start_position_interp(120.0f / 60.0f, 4, 192.0f, 140.0f);

    // Keep boss in place during spellcard (position_interp handles this)
    boss->set_speedY(0.0f);

    // Stop phase sequencer — spellcard takes over
    boss->stop_phases();

    // Swap boss emitters to spellcard patterns
    boss->emitterConfig = sc->patterns;
    boss->emitterRuntime.resize(boss->emitterConfig.size());
    for (size_t i = 0; i < boss->emitterRuntime.size(); i++) {
        boss->emitterRuntime[i].timer = 0.0f;
        boss->emitterRuntime[i].burstRemaining = 0;
        boss->emitterRuntime[i].cycleCount = 0;
    }

    spellcardEntryTimer_ = 0.0f;

    std::cout << ">> Spellcard " << phaseIndex << " started: \""
              << sc->name << "\" (timeout="
              << sc->timeout << "s), patterns=" << (int)sc->patterns.size() << std::endl;
}

void LevelManager::update_spellcards(float dt) {
    if (bossEnemyIndex_ < 0 || bossEnemyIndex_ >= (int)enemy_pool.size()) return;
    Enemy* boss = enemy_pool[bossEnemyIndex_];
    if (!boss || !boss->is_active()) return;

    // ── Check if spellcard should trigger ─────────────────────────────────────
    if (!scManager_.is_active() && scManager_.count() > 0 && spellcardTriggerHp_ > 0) {
        // ECL Sub20: non-spellcard timer (1680 frames = 28s)
        nonSpellcardTimer_ += dt;

        bool hpTrigger = (boss->get_hp() <= spellcardTriggerHp_);
        bool timerTrigger = (nonSpellcardTimer_ >= 28.0f);

        if (hpTrigger) {
            std::cout << "Boss HP (" << boss->get_hp() << ") ≤ threshold ("
                      << spellcardTriggerHp_ << ") — starting spellcard!" << std::endl;
            start_spellcard_phase(0);
            return;
        }
        if (timerTrigger) {
            std::cout << "Non-spellcard timer expired (" << nonSpellcardTimer_
                      << "s) — starting spellcard!" << std::endl;
            start_spellcard_phase(0);
            return;
        }
        return;
    }

    if (!scManager_.is_active()) return;

    // Advance spellcard timer (2-second entry grace period before timeout starts)
    spellcardEntryTimer_ += dt;
    if (spellcardEntryTimer_ >= 2.0f) {
        scManager_.update(dt);
    }

    // ── Check timeout ─────────────────────────────────────────────────────────
    if (scManager_.is_timeout()) {
        scManager_.end(false);  // not captured
        std::cout << "Spellcard timeout! Converting bullets to P items." << std::endl;
        convert_boss_bullets_to_p_items();
        EnemyScManager::clear_bullets(bullet_mgr_);

        // ECL Sub28: spellcard_end, move offscreen, delete
        std::cout << "Spellcard timeout!" << std::endl;
        boss->start_position_interp(60.0f / 60.0f, 4, -128.0f, 32.0f);
        boss->deactivate();
        if (item_mgr_) {
            float ex = boss->get_x();
            float ey = boss->get_y();
            item_mgr_->spawn_item(ex, ey, ITEM_POWER_SMALL, 8);
        }
        return;
    }

    // ── Check capture (HP depleted) ───────────────────────────────────────────
    std::cout << "[SPELLCARD] bossHP=" << boss->get_hp() << " active=" << boss->is_active()
              << " entryTimer=" << spellcardEntryTimer_ << std::endl;
    if (boss->get_hp() <= 0) {
        scManager_.end(true);  // captured
        std::cout << "Spellcard captured! Converting bullets to P items." << std::endl;
        convert_boss_bullets_to_p_items();
        EnemyScManager::clear_bullets(bullet_mgr_);

        // ECL Sub27: spellcard_end, drop items, boss_set(-1), delete
        std::cout << "Spellcard captured! Boss defeated." << std::endl;
        if (item_mgr_) {
            float ex = boss->get_x() + boss->get_hitbox_w() * 0.5f;
            float ey = boss->get_y() + boss->get_hitbox_h() * 0.5f;
            item_mgr_->spawn_item(ex, ey, ITEM_POWER_SMALL, 8);
        }
        boss->deactivate();
    }
}

void LevelManager::convert_boss_bullets_to_p_items() {
    if (bullet_mgr_ && item_mgr_) {
        std::cout << "Converting all enemy bullets to P items..." << std::endl;
        bullet_mgr_->convert_all_to_p_items(item_mgr_);
    }
}

void LevelManager::skip_stage_time(float seconds) {
    if (seconds <= 0) return;
    std::cout << "Skipping stage time by " << seconds << "s..." << std::endl;
    for (size_t i = 0; i < enemy_pool.size(); i++) {
        if (enemy_pool[i] && enemy_pool[i]->has_pending_spawn())
            enemy_pool[i]->adjust_time_alive(seconds);
    }
    nonSpellcardTimer_ = std::max(0.0f, nonSpellcardTimer_ - seconds);
}

LevelManager::~LevelManager() {
    clear_enemy_pool();
    if (bgm_music_ != NULL) {
        Mix_HaltMusic();
        if (Mix_GetMusicType(bgm_music_) != MUS_MID) {
            Mix_FreeMusic(bgm_music_);
        }
        bgm_music_ = NULL;
    }
    if (stage_enemies_data != NULL) {
        cJSON_Delete(stage_enemies_data);
        stage_enemies_data = NULL;
    }
}

std::string LevelManager::stage_key() const {
    std::ostringstream ss;
    ss << current_stage;
    return "stage_" + ss.str();
}

void LevelManager::clear_enemy_pool() {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL) {
            delete enemy_pool[i];
            enemy_pool[i] = NULL;
        }
    }
    enemy_pool.clear();
    bossEnemyIndex_ = -1;
    spellcardTriggerHp_ = 0;
    nonSpellcardTimer_ = 0.0f;
    midbossDefeatedProcessed_ = false;
    scManager_.reset();
}

void LevelManager::load_stage(int stage) {
    current_stage = stage;
    stage_state = STAGE_LOADING;
    std::ostringstream ss;
    ss << stage;
    std::string file_path = "level/level" + ss.str() + ".json";

    std::ifstream file(file_path.c_str());
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return;
    }

    std::string json_string;
    std::string line;
    while (std::getline(file, line)) {
        json_string += line;
    }
    file.close();

    if (stage_enemies_data != NULL) {
        cJSON_Delete(stage_enemies_data);
        stage_enemies_data = NULL;
    }

    stage_enemies_data = cJSON_Parse(json_string.c_str());
    if (stage_enemies_data == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            std::cerr << "JSON parse error before: " << error_ptr << std::endl;
        }
        return;
    }

    // V2 format uses "meta" key instead of stage_N array
    cJSON* meta = cJSON_GetObjectItem(stage_enemies_data, "meta");
    if (meta && cJSON_IsObject(meta)) {
        std::cout << "Successfully loaded: " << file_path << " (V2 format)" << std::endl;
        return;
    }

    std::string key = stage_key();
    cJSON* stage_array = cJSON_GetObjectItem(stage_enemies_data, key.c_str());
    if (!cJSON_IsArray(stage_array)) {
        std::cerr << key << " is not a valid array in JSON" << std::endl;
        return;
    }

    std::cout << "Successfully loaded: " << file_path << std::endl;
    std::cout << "Total enemies in " << key << ": " << cJSON_GetArraySize(stage_array) << std::endl;
}

void LevelManager::load_boss_stage(int stage) {
    current_stage = stage;
    stage_state = STAGE_LOADING;
    std::ostringstream ss;
    ss << stage;
    std::string file_path = "level/level" + ss.str() + "bs.json";

    std::ifstream file(file_path.c_str());
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return;
    }

    std::string json_string;
    std::string line;
    while (std::getline(file, line))
        json_string += line;
    file.close();

    if (stage_enemies_data != NULL) {
        cJSON_Delete(stage_enemies_data);
        stage_enemies_data = NULL;
    }
    stage_enemies_data = cJSON_Parse(json_string.c_str());
    if (stage_enemies_data == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
            std::cerr << "JSON parse error: " << error_ptr << std::endl;
        return;
    }
    std::cout << "Successfully loaded: " << file_path << std::endl;
}

void LevelManager::init_enemy_pool() {
    if (stage_enemies_data == NULL) {
        std::cerr << "Error: stage_enemies_data is NULL. Call read_stage_data first." << std::endl;
        return;
    }

    // 事实上只有v2的json，已经把v1的解析去了，所以这里只调用v2的解析函数
    cJSON* meta = cJSON_GetObjectItem(stage_enemies_data, "meta");
    if (meta && cJSON_IsObject(meta)) {
        init_enemy_pool_v2();
        return;
    }
    std::cout << "Enemy pool initialized with " << enemy_pool.size() << " enemies" << std::endl;
}

int LevelManager::get_enemy_count() const {
    return (int)enemy_pool.size();
}

Enemy* LevelManager::get_enemy(int index) {
    if (index >= 0 && index < (int)enemy_pool.size()) {
        return enemy_pool[index];
    }
    return NULL;
}

void LevelManager::trigger_midboss_clear(Uint32 &frameCounter, Uint32 &midbossEnterFrame_, StageState &prevStageState, StageState &currentStageState){
    isClearingForMidboss = false;
    midbossEnterFrame_ = (Uint32)frameCounter;
    for(int i = 0; i < midbossIndex_; i++)  enemy_pool[i]->deactivate();


}

void LevelManager::update_all_enemies(float px, float py, Uint32 &frameCounter_, Uint32 &midbossEnterFrame_, float dt_, StageState &prevStageState, StageState &currentStageState) {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if(enemy_pool[i] == NULL)   continue;

        if (enemy_pool[i] != NULL) {
            enemy_pool[i]->update_player_info(px, py, frameCounter_);
        }
        if(enemy_pool[i] != NULL && enemy_pool[i]->get_is_midboss() && enemy_pool[i]->is_active() && isClearingForMidboss){
            midbossIndex_ = i;
            std::cout<<"it's time to trigger clear";
            trigger_midboss_clear(frameCounter_, midbossEnterFrame_, prevStageState, currentStageState);
        }
    }

    // ── Midboss/boss defeat detection → bullet conversion + time jump ──
    if (!midbossDefeatedProcessed_) {
        for (size_t i = 0; i < enemy_pool.size(); ++i) {
            Enemy* e = enemy_pool[i];
            if (!e) continue;
            bool isMid = e->get_is_midboss();
            bool isBoss = (!isMid && e->get_enemy_type() >= BOSS_RUMIA && !e->has_pending_spawn());
            if (!isMid && !isBoss) continue;
            bool dead = !e->is_active();
            bool hpZero = e->get_hp() <= 0;
            std::cout << "[DEFEAT] i=" << i << " isMid=" << isMid << " isBoss=" << isBoss
                      << " dead=" << dead << " hpZero=" << hpZero << " hp=" << e->get_hp()
                      << " type=" << e->get_enemy_type() << std::endl;
            if (dead && hpZero) {
                midbossDefeatedProcessed_ = true;
                std::cout << "Main enemy defeated! Converting bullets to P items." << std::endl;
                convert_boss_bullets_to_p_items();
                if (isMid) {
                    float skip_amount = (e->get_emerge_time() + e->get_duration_time()) - e->get_time_alive();
                    if (skip_amount < 0) skip_amount = 0;
                    std::cout << "[DEFEAT] midboss time_jump: skip=" << skip_amount << " emerge=" << e->get_emerge_time() << " dur=" << e->get_duration_time() << " alive=" << e->get_time_alive() << std::endl;
                    if (skip_amount > 0) skip_stage_time(skip_amount);
                }
                break;
            }
        }
    }
}

void LevelManager::move_all_enemies(float dt_) {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL) {
            enemy_pool[i]->enemy_move(dt_);
        }
    }
}

void LevelManager::attack_all_enemies(float dt_) {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL && enemy_pool[i]->is_active()) {
            enemy_pool[i]->enemy_attack(dt_);
        }
    }
}

void LevelManager::set_bullet_manager_for_all(EnemyBulletManager* mgr) {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL) {
            enemy_pool[i]->bulletManager = mgr;
        }
    }
}

void LevelManager::show_all_enemies() {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL && enemy_pool[i]->is_active()) {
            enemy_pool[i]->enemy_show();
            //std::cout << i << " " << "is rendered" << std::endl;
        }
    }
}