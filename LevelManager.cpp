#include "LevelManager.h"

LevelManager::LevelManager() : stage_enemies_data(NULL) {
}

LevelManager::~LevelManager() {
    if (stage_enemies_data != NULL) {
        cJSON_Delete(stage_enemies_data);
        stage_enemies_data = NULL;
    }
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
