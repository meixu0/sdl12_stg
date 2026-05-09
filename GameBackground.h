#include "UI.h"
class GameBackground{
private:
    float bgX;
    float bgY;
    float scrollSpeed;
    static const int BG_HEIGHT;
    static const int BG_WIDTH;
public:
    static SDL_Surface* backgroundImage;
    GameBackground();
    void background_update(float dt);
    void background_show();
};