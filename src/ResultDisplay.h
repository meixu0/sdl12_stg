#include "UI.h"
#include "cJSON.h"

class ResultDisplay {
private:
    static SDL_Surface* bgImage;
    static SDL_Surface* asciiSheet;   // 未选中
    static SDL_Surface* asciiSelSheet; // 选中

    cJSON* records_;       // 所有记录数组
    int    recordCount_;    // 记录总数
    int    currentPage_;    // 当前页 0-based
    int    selCol_;         // 0=PREV, 1=NEXT
    Uint32 highScore_;

    static Mix_Chunk* select00;
    static Mix_Chunk* ok00;
    static Mix_Chunk* cancel00;

    void load_records();
    void draw_string(const char* s, int x, int y, bool selected);
    void draw_number(int n, int x, int y, bool selected, int minW);
    SDL_Rect get_char_src(char c) const;

public:
    ResultDisplay();
    ~ResultDisplay();
    void handle_events(SDL_Event &e);
    void render();
    Uint32 get_high_score() const { return highScore_; }
};
