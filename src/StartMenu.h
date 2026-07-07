#include "UI.h"
struct ShotRects{
    SDL_Rect portrait;
    SDL_Rect shotA_sel;
    SDL_Rect shotA_norm;
    SDL_Rect shotB_sel;
    SDL_Rect shotB_norm;
};
class StartMenu{
private:
    static ShotRects shotRects[4];
    static SDL_Surface* select00img;
    static SDL_Surface* playerSelectImage;
    static SDL_Surface* plSheet[4];
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