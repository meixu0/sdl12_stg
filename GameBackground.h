#include "UI.h"
class GameBackground{
private:
    int bgX;
    int bgY;
    static const int BG_HEIGHT;
    static const int BG_WIDTH;
public:
    static SDL_Surface* backgroundImage;
    GameBackground();
    void background_update();
    void background_show();
};