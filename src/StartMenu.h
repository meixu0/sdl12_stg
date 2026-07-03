#include "UI.h"
class StartMenu{
private:
    static SDL_Surface* pl00;
    static SDL_Surface* pl01;
    static SDL_Surface* pl02;
    static SDL_Surface* select00img;
    static SDL_Surface* reimuTsuiti[2];
    static SDL_Surface* reimuSokusha[2];
    static SDL_Surface* marisaIryokujuushi[2];
    static SDL_Surface* marisaKantsuu[2];
    static SDL_Surface* sakuyaKouhanni[2];
    static SDL_Surface* sakuyaTokushu[2];
    int currentPlayerIndex;
    int currentScIndex;
	static Mix_Chunk* select00;
	static Mix_Chunk* ok00;
    static Mix_Chunk* cancel00;
    void switch_to_next_player();
    void switch_to_previous_player();
    void switch_to_previous_sc();
	void switch_to_next_sc();
    void jump_to_game();
    void back_to_difficulty_menu();
public:
    StartMenu();
    ~StartMenu();
    void handle_events(SDL_Event &e);
	void render();
};