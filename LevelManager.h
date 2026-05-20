#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "cJSON.h"
#include "Enemy.h"

class LevelManager {
private:
    cJSON* stage_enemies_data;
    
public:
    LevelManager();
    ~LevelManager();
    void read_stage_data(const std::string file_path);
};