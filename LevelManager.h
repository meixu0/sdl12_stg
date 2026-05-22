#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "cJSON.h"
#include "Enemy.h"

class LevelManager {
private:
    cJSON* stage_enemies_data;
    std::vector<Enemy*> enemy_pool;
    
    void clear_enemy_pool();
    
public:
    LevelManager();
    ~LevelManager();
    void read_stage_data(const std::string file_path);
    void init_enemy_pool();
    
    // Enemy pool management interfaces
    int get_enemy_count() const;
    Enemy* get_enemy(int index);
    void update_all_enemies(float px, float py, size_t frameCounter_);
    void move_all_enemies();
    void show_all_enemies();
};