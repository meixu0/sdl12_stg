#include "LevelManager.h"

LevelManager::LevelManager() : stage_enemies_data(NULL) {
}

LevelManager::~LevelManager() {
    clear_enemy_pool();
    if (stage_enemies_data != NULL) {
        cJSON_Delete(stage_enemies_data);
        stage_enemies_data = NULL;
    }
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

void LevelManager::read_stage_data(const std::string file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return;
    }
    
    // Read entire file into a string using getline
    std::string json_string;
    std::string line;
    while (std::getline(file, line)) {
        json_string += line;
    }
    file.close();
    
    // Delete old data if exists
    if (stage_enemies_data != NULL) {
        cJSON_Delete(stage_enemies_data);
        stage_enemies_data = NULL;
    }
    
    // Parse JSON string
    stage_enemies_data = cJSON_Parse(json_string.c_str());
    if (stage_enemies_data == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            std::cerr << "JSON parse error before: " << error_ptr << std::endl;
        }
        return;
    }
    
    // Get stage_1 array from the root object
    cJSON* stage_1 = cJSON_GetObjectItem(stage_enemies_data, "stage_1");
    if (!cJSON_IsArray(stage_1)) {
        std::cerr << "stage_1 is not a valid array in JSON" << std::endl;
        return;
    }
    
    std::cout << "Successfully loaded enemy data from: " << file_path << std::endl;
    std::cout << "Total enemies in stage_1: " << cJSON_GetArraySize(stage_1) << std::endl;
}

void LevelManager::init_enemy_pool() {
    if (stage_enemies_data == NULL) {
        std::cerr << "Error: stage_enemies_data is NULL. Call read_stage_data first." << std::endl;
        return;
    }
    
    // Clear existing pool
    clear_enemy_pool();
    
    // Get stage_1 array
    cJSON* stage_1 = cJSON_GetObjectItem(stage_enemies_data, "stage_1");
    if (!cJSON_IsArray(stage_1)) {
        std::cerr << "Error: stage_1 is not a valid array in JSON" << std::endl;
        return;
    }
    
    int array_size = cJSON_GetArraySize(stage_1);
    
    // Iterate through each enemy in stage_1
    cJSON* enemy_item = NULL;
    for (int i = 0; i < array_size; ++i) {
        enemy_item = cJSON_GetArrayItem(stage_1, i);
        if (enemy_item == NULL) {
            continue;
        }
        
        // Parse enemy configuration from JSON
        EnemyConfig config;
        
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
        
        // Extract move_pattern
        cJSON* move_pattern = cJSON_GetObjectItem(enemy_item, "move_pattern");
        int move_type = LINER;  // Default move pattern
        if (move_pattern && cJSON_IsObject(move_pattern)) {
            cJSON* speed_item = cJSON_GetObjectItem(move_pattern, "speed");
            speed = (speed_item && cJSON_IsNumber(speed_item)) ? (float)speed_item->valuedouble : 3.0f;
            
            cJSON* type_item = cJSON_GetObjectItem(move_pattern, "type");
            if (type_item && cJSON_IsString(type_item)) {
                std::string type_str = type_item->valuestring;
                if (type_str == "curve_left") {
                    move_type = SINWAVE;  // Use sine wave for curve movement
                } else if (type_str == "linear") {
                    move_type = LINER;
                } else if (type_str == "uturn") {
                    move_type = UTURN;
                } else if (type_str == "homing") {
                    move_type = HOMING;
                }
            }
        }

        //extract spawn_time
        cJSON* spawn_time = cJSON_GetObjectItem(enemy_item, "spawn_time");
        size_t emergeTime;
        if(spawn_time && cJSON_IsObject(spawn_time)){
            emergeTime = spawn_time->valueint;
        }

        //extract speed
        float speedX, speedY;
        
        
        // Set default configuration values
        config.movePattern = move_type;
        config.emergeTime = emergeTime;
        config.durationTime = 10.0f;
        config.hitboxWidth = 40;
        config.hitboxHeight = 40;
        config.speedX = speedX;
        config.speedY = speedY;
        config.targetX = start_x;
        config.targetY = start_y;
        config.startX = start_x;
        config.startY = start_y;
        
        // Create Enemy object and initialize
        Enemy* new_enemy = new Enemy();
        new_enemy->init(config, start_x, start_y);
        
        // Add to pool
        enemy_pool.push_back(new_enemy);
        
        std::cout << "Enemy " << i << " created: hp=" << config.hp 
                  << ", pos=(" << start_x << "," << start_y << ")" << std::endl;
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

void LevelManager::move_all_enemies() {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL) {
            enemy_pool[i]->enemy_move();
        }
    }
}

void LevelManager::show_all_enemies() {
    for (size_t i = 0; i < enemy_pool.size(); ++i) {
        if (enemy_pool[i] != NULL && enemy_pool[i]->is_active()) {
            enemy_pool[i]->enemy_show();
            std::cout << i << " " << "is rendered" << std::endl;
        }
    }
}
