#include "LevelManager.h"

LevelManager::LevelManager() : stage_enemies_data(NULL), current_stage(1), stage_state(STAGE_LOADING) {
}

// ── 关卡状态机 ──

void LevelManager::start_stage() {
    stage_state = STAGE_RUNNING;
    std::cout << "Stage " << current_stage << " started." << std::endl;
}

void LevelManager::trigger_boss() {
    stage_state = STAGE_BOSS;
    std::cout << "Stage " << current_stage << " boss triggered." << std::endl;
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

LevelManager::~LevelManager() {
    clear_enemy_pool();
    if (stage_enemies_data != NULL) {
        cJSON_Delete(stage_enemies_data);
        stage_enemies_data = NULL;
    }
}

std::string LevelManager::stage_key() const {
    return "stage_" + std::to_string(current_stage);
}

void LevelManager::clear_enemy_pool() {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL) {
            delete enemy_pool[i];
            enemy_pool[i] = NULL;
        }
    }
    enemy_pool.clear();
}

void LevelManager::load_stage(int stage) {
    current_stage = stage;
    stage_state = STAGE_LOADING;
    std::string file_path = "level/level" + std::to_string(stage) + ".json";

    std::ifstream file(file_path);
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

    std::string key = stage_key();
    cJSON* stage_array = cJSON_GetObjectItem(stage_enemies_data, key.c_str());
    if (!cJSON_IsArray(stage_array)) {
        std::cerr << key << " is not a valid array in JSON" << std::endl;
        return;
    }

    std::cout << "Successfully loaded: " << file_path << std::endl;
    std::cout << "Total enemies in " << key << ": " << cJSON_GetArraySize(stage_array) << std::endl;
}

void LevelManager::init_enemy_pool() {
    if (stage_enemies_data == NULL) {
        std::cerr << "Error: stage_enemies_data is NULL. Call read_stage_data first." << std::endl;
        return;
    }
    
    // Clear existing pool
    clear_enemy_pool();
    
    // Get stage array by current stage key
    std::string key = stage_key();
    cJSON* stage_array = cJSON_GetObjectItem(stage_enemies_data, key.c_str());
    if (!cJSON_IsArray(stage_array)) {
        std::cerr << "Error: " << key << " is not a valid array in JSON" << std::endl;
        return;
    }

    int array_size = cJSON_GetArraySize(stage_array);
    
    // Iterate through each enemy in stage_1
    cJSON* enemy_item = NULL;
    for (int i = 0; i < array_size; ++i) {
        enemy_item = cJSON_GetArrayItem(stage_array, i);
        if (enemy_item == NULL) {
            continue;
        }
        
        // Parse enemy configuration from JSON
        EnemyConfig config;
        memset(&config, 0, sizeof(config));

        // Look up defaults from enemy_types by enemyType field
        cJSON* enemy_types = cJSON_GetObjectItem(stage_enemies_data, "enemy_types");
        cJSON* type_name = cJSON_GetObjectItem(enemy_item, "enemyType");
        cJSON* defaults = NULL;
        if (enemy_types && type_name && cJSON_IsString(type_name))
            defaults = cJSON_GetObjectItem(enemy_types, type_name->valuestring);
        if (!defaults || !cJSON_IsObject(defaults))
            defaults = enemy_item;

#define GET_FLD(obj, field, def) \
    cJSON* _##field = cJSON_GetObjectItem(enemy_item, #field); \
    if (!_##field && defaults != enemy_item) _##field = cJSON_GetObjectItem(defaults, #field); \
    float field = (_##field && cJSON_IsNumber(_##field)) ? (float)_##field->valuedouble : (float)def;
        
        // Extract hp
        cJSON* hp_item = cJSON_GetObjectItem(enemy_item, "hp");
        config.hp = (hp_item && cJSON_IsNumber(hp_item)) ? (int)hp_item->valuedouble : 50;
        
        // Extract start_pos [x, y]
        cJSON* start_pos = cJSON_GetObjectItem(enemy_item, "start_pos");
        float start_x = 0.0f;
        float start_y = 0.0f;
        if (start_pos && cJSON_IsArray(start_pos)) {
            cJSON* pos_x = cJSON_GetArrayItem(start_pos, 0);
            cJSON* pos_y = cJSON_GetArrayItem(start_pos, 1);
            start_x = (pos_x && cJSON_IsNumber(pos_x)) ? (float)pos_x->valuedouble : 0.0f;
            start_y = (pos_y && cJSON_IsNumber(pos_y)) ? (float)pos_y->valuedouble : 0.0f;
        }
        // Extract move pattern
        int move_type = LINER;  // Default move pattern
        cJSON* move_pattern_value = cJSON_GetObjectItem(enemy_item, "movePattern");
        if (move_pattern_value && cJSON_IsNumber(move_pattern_value)) {
            move_type = (int)move_pattern_value->valuedouble;
        } else {
            cJSON* move_pattern = cJSON_GetObjectItem(enemy_item, "move_pattern");
            if (move_pattern && cJSON_IsObject(move_pattern)) {
                cJSON* type_item = cJSON_GetObjectItem(move_pattern, "type");
                if (type_item && cJSON_IsString(type_item)) {
                    std::string type_str = type_item->valuestring;
                    if (type_str == "curve_left") {
                        move_type = SINWAVE;
                    } else if (type_str == "linear") {
                        move_type = LINER;
                    } else if (type_str == "bezier") {
                        move_type = BEZIER;
                    } else if (type_str == "homing") {
                        move_type = HOMING;
                    }
                }
            }
        }

        // extract spawn_time
        size_t emergeTime = 0;
        cJSON* spawn_time = cJSON_GetObjectItem(enemy_item, "spawn_time");
        if (spawn_time && cJSON_IsNumber(spawn_time)) {
            emergeTime = (size_t)spawn_time->valuedouble;
        }

        // extract durationTime
        float durationTime = 12.0f;
        cJSON* duration_time_item = cJSON_GetObjectItem(enemy_item, "durationTime");
        if (duration_time_item && cJSON_IsNumber(duration_time_item)) {
            durationTime = (float)duration_time_item->valuedouble;
        }

        // extract hitbox size
        int hitboxWidth = 40;
        int hitboxHeight = 40;
        cJSON* hitbox_width_item = cJSON_GetObjectItem(enemy_item, "hitboxWidth");
        cJSON* hitbox_height_item = cJSON_GetObjectItem(enemy_item, "hitboxHeight");
        if (hitbox_width_item && cJSON_IsNumber(hitbox_width_item)) {
            hitboxWidth = (int)hitbox_width_item->valuedouble;
        }
        if (hitbox_height_item && cJSON_IsNumber(hitbox_height_item)) {
            hitboxHeight = (int)hitbox_height_item->valuedouble;
        }

        // extract speedX and speedY
        float speedX = 0.0f;
        float speedY = 1.0f;
        cJSON* speedX_item = cJSON_GetObjectItem(enemy_item, "speedX");
        cJSON* speedY_item = cJSON_GetObjectItem(enemy_item, "speedY");
        if (speedX_item && cJSON_IsNumber(speedX_item)) {
            speedX = (float)speedX_item->valuedouble;
        }
        if (speedY_item && cJSON_IsNumber(speedY_item)) {
            speedY = (float)speedY_item->valuedouble;
        }

        // extract targetX and targetY
        float targetX = start_x;
        float targetY = start_y;
        cJSON* targetX_item = cJSON_GetObjectItem(enemy_item, "targetX");
        cJSON* targetY_item = cJSON_GetObjectItem(enemy_item, "targetY");
        if (targetX_item && cJSON_IsNumber(targetX_item)) {
            targetX = (float)targetX_item->valuedouble;
        }
        if (targetY_item && cJSON_IsNumber(targetY_item)) {
            targetY = (float)targetY_item->valuedouble;
        }

        // Set configuration values
        config.movePattern = move_type;
        config.emergeTime = emergeTime;
        config.durationTime = durationTime;
        config.hitboxWidth = hitboxWidth;
        config.hitboxHeight = hitboxHeight;
        config.speedX = speedX;
        config.speedY = speedY;
        config.targetX = targetX;
        config.targetY = targetY;
        config.startX = start_x;
        config.startY = start_y;

        float halfLife = 0.0f;
        cJSON* halfLife_item = cJSON_GetObjectItem(enemy_item, "halfLife");
        if (halfLife_item && cJSON_IsNumber(halfLife_item))
            halfLife = (float)halfLife_item->valuedouble;
        config.halfLife = halfLife;

        float vertAmplitude = 0.0f, vertPeriod = 1.0f, horizAmplitude = 0.0f, horizPeriod = 1.0f;
        cJSON* va = cJSON_GetObjectItem(enemy_item, "vertAmplitude");
        cJSON* vp = cJSON_GetObjectItem(enemy_item, "vertPeriod");
        cJSON* ha = cJSON_GetObjectItem(enemy_item, "horizAmplitude");
        cJSON* hp = cJSON_GetObjectItem(enemy_item, "horizPeriod");
        if (va && cJSON_IsNumber(va)) vertAmplitude = (float)va->valuedouble;
        if (vp && cJSON_IsNumber(vp)) vertPeriod   = (float)vp->valuedouble;
        if (ha && cJSON_IsNumber(ha)) horizAmplitude = (float)ha->valuedouble;
        if (hp && cJSON_IsNumber(hp)) horizPeriod   = (float)hp->valuedouble;
        config.vertAmplitude  = vertAmplitude;
        config.vertPeriod     = vertPeriod;
        config.horizAmplitude = horizAmplitude;
        config.horizPeriod    = horizPeriod;

        float bezierP1x = 0, bezierP1y = 0, bezierP2x = 0, bezierP2y = 0;
        float bezierEndX = 0, bezierEndY = 0, bezierDuration = 1.0f;
        cJSON* bezier = cJSON_GetObjectItem(enemy_item, "bezier");
        if (bezier && cJSON_IsObject(bezier)) {
            cJSON* p1 = cJSON_GetObjectItem(bezier, "p1");
            if (p1 && cJSON_IsArray(p1)) {
                bezierP1x = (float)cJSON_GetArrayItem(p1, 0)->valuedouble;
                bezierP1y = (float)cJSON_GetArrayItem(p1, 1)->valuedouble;
            }
            cJSON* p2 = cJSON_GetObjectItem(bezier, "p2");
            if (p2 && cJSON_IsArray(p2)) {
                bezierP2x = (float)cJSON_GetArrayItem(p2, 0)->valuedouble;
                bezierP2y = (float)cJSON_GetArrayItem(p2, 1)->valuedouble;
            }
            cJSON* end = cJSON_GetObjectItem(bezier, "end");
            if (end && cJSON_IsArray(end)) {
                bezierEndX = (float)cJSON_GetArrayItem(end, 0)->valuedouble;
                bezierEndY = (float)cJSON_GetArrayItem(end, 1)->valuedouble;
            }
            cJSON* dur = cJSON_GetObjectItem(bezier, "duration");
            if (dur && cJSON_IsNumber(dur))
                bezierDuration = (float)dur->valuedouble;
        }
        config.bezierP1x = bezierP1x;
        config.bezierP1y = bezierP1y;
        config.bezierP2x = bezierP2x;
        config.bezierP2y = bezierP2y;
        config.bezierEndX = bezierEndX;
        config.bezierEndY = bezierEndY;
        config.bezierDuration = bezierDuration;

        float moveAngle = 0.0f, angularVelocity = 0.0f, acceleration = 0.0f, minPlayerDist = 80.0f;
        cJSON* ma = cJSON_GetObjectItem(enemy_item, "moveAngle");
        cJSON* av = cJSON_GetObjectItem(enemy_item, "angularVelocity");
        cJSON* ac = cJSON_GetObjectItem(enemy_item, "acceleration");
        cJSON* md = cJSON_GetObjectItem(enemy_item, "minPlayerDist");
        if (ma && cJSON_IsNumber(ma)) moveAngle = (float)ma->valuedouble;
        if (av && cJSON_IsNumber(av)) angularVelocity = (float)av->valuedouble;
        if (ac && cJSON_IsNumber(ac)) acceleration = (float)ac->valuedouble;
        if (md && cJSON_IsNumber(md)) minPlayerDist = (float)md->valuedouble;
        config.moveAngle = moveAngle;
        config.angularVelocity = angularVelocity;
        config.acceleration = acceleration;
        config.minPlayerDist = minPlayerDist;

        Enemy* new_enemy = new Enemy();

        // --- emitterConfig parsing ---
        cJSON* emitters_json = cJSON_GetObjectItem(enemy_item, "emitters");
        if (!emitters_json && defaults != enemy_item)
            emitters_json = cJSON_GetObjectItem(defaults, "emitters");
        if (emitters_json && cJSON_IsArray(emitters_json)) {
            int em_cnt = cJSON_GetArraySize(emitters_json);
            for (int e = 0; e < em_cnt; e++) {
                cJSON* ej = cJSON_GetArrayItem(emitters_json, e);
                if (!ej || !cJSON_IsObject(ej)) continue;
                EmitterConfig ec;

                cJSON* v;
                #define GET(key, def) ((v = cJSON_GetObjectItem(ej, #key)) && cJSON_IsNumber(v)) ? (float)v->valuedouble : (float)(def)
                ec.startDelay    = GET(startDelay, 0.0f);
                ec.emitInterval  = GET(emitInterval, 1.0f);
                ec.burstCount    = (int)GET(burstCount, 1);
                ec.burstInterval = GET(burstInterval, 0.05f);
                #undef GET

                cJSON* pd = cJSON_GetObjectItem(ej, "patternDesc");
                if (pd && cJSON_IsObject(pd)) {
                    #define GET(key, def) ((v = cJSON_GetObjectItem(pd, #key)) && cJSON_IsNumber(v)) ? (float)v->valuedouble : (float)(def)
                    ec.patternDesc.patternType   = (int)GET(patternType, 0);
                    ec.patternDesc.mainCnt       = (int)GET(mainCnt, 6);
                    ec.patternDesc.subCnt        = (int)GET(subCnt, 1);
                    ec.patternDesc.angleOffset   = GET(angleOffset, 0.0f);
                    ec.patternDesc.angleInterval = GET(angleInterval, 0.15f);
                    ec.patternDesc.speed1        = GET(speed1, 100.0f);
                    ec.patternDesc.speed2        = GET(speed2, 50.0f);
                    ec.patternDesc.spriteID      = (int)GET(spriteID, 0);
                    ec.patternDesc.hitboxRadius  = GET(hitboxRadius, 4.0f);
                    ec.patternDesc.lifeTime      = GET(lifeTime, 6.0f);
                    ec.patternDesc.spawnEffect   = (int)GET(spawnEffect, 0);
                    ec.patternDesc.soundEffect   = (int)GET(soundEffect, 0);
                    ec.patternDesc.reboundEffect = (int)GET(reboundEffect, 0);
                    #undef GET
                }
                new_enemy->emitterConfig.push_back(ec);
            }
        }

        new_enemy->init(config, start_x, start_y);
        
        // Add to pool
        enemy_pool.push_back(new_enemy);
        
        std::cout << "Enemy " << i << " created: hp=" << config.hp 
                  << ", pos=(" << start_x << "," << start_y << ")" << " emergeTime=" << config.emergeTime << std::endl;
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

void LevelManager::update_all_enemies(float px, float py, size_t frameCounter_) {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL) {
            enemy_pool[i]->update_player_info(px, py, frameCounter_);
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
