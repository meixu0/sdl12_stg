#include "UI.h"
class DifficultyMenu{
private:
    static SDL_Surface* select00_;
    static SDL_Surface* select01;
    static SDL_Rect select01Rect[5];
    static SDL_Surface* selectLevelImage;
    int currentSelectedDifficulty;
    static Mix_Chunk* select00;
	static Mix_Chunk* ok00;
    static Mix_Chunk* cancel00;
    void switch_to_previous_difficulty();
	void switch_to_next_difficulty();
	void jump_to_submenu();
    void back_to_mainmenu();
    SDL_Rect get_menu_src_rect(int id, bool isSelected);
public:
    DifficultyMenu();
    ~DifficultyMenu();
    void handle_events(SDL_Event &e);
    void render();
};