#include "UI.h"

class OptionMenu{
private:
    static SDL_Surface* titleSheet;
    static SDL_Surface* select00img;
    int currentRow;     // 0=Player, 1=Bomb
    int playerLives_;   // 1-9
    int playerBombs_;   // 1-9
    static Mix_Chunk* select00;
    static Mix_Chunk* cancel00;
    void switch_up();
    void switch_down();
    void switch_left();
    void switch_right();
    void back_to_main();
public:
    OptionMenu();
    ~OptionMenu();
    void handle_events(SDL_Event &e);
    void render();
};
