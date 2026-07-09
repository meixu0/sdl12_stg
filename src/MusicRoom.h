#include "UI.h"

class MusicRoom {
private:
    static SDL_Surface* bgImage;
    static SDL_Surface* asciiSheet;
    static SDL_Surface* asciiSelSheet;
    static Mix_Music*   currentBgm;
    static Mix_Chunk*   select00;
    static Mix_Chunk*   ok00;
    static Mix_Chunk*   cancel00;

    int selRow_;     // 0-4
    int selCol_;     // 0-3

    void draw_string(const char* s, int x, int y, bool selected);
    void draw_number(int n, int x, int y, bool selected, int minW);
    SDL_Rect get_char_src(char c) const;
    void play_track(int n);

public:
    MusicRoom();
    void handle_events(SDL_Event &e);
    void render();
};
