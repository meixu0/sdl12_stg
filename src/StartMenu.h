#include "UI.h"
class StartMenu{
private:
    static SDL_Surface* pl00;
    static SDL_Surface* pl01;
    static SDL_Surface* pl02;
    int currentPlayerIndex;
	static Mix_Chunk* select00;
	static Mix_Chunk* ok00;
    void switch_to_next_player();
    void switch_to_previous_player();
    void jump_to_game();
    void back_to_mainmenu();
public:
    StartMenu();
    ~StartMenu();
    void handle_events(SDL_Event &e);
	void render();
};