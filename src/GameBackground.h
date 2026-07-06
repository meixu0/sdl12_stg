#include "UI.h"
class GameBackground{
private:
    float bgX;
    float bgY;
    float scrollSpeed;
    static const int BG_HEIGHT;
    static const int BG_WIDTH;
    static SDL_Surface* stgbg[2];
public:
    GameBackground();
    void background_update(float dt);
    void background_show();
};