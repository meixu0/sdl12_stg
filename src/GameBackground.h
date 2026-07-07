#include "UI.h"

struct DecorPair {
    int yOffset;   // 相对 bgLayerY 的 Y 偏移
    int leftX;     // 左装饰的 X 坐标
    int rightX;    // 右装饰的 X 坐标
};

class GameBackground{
private:
    float bgX;
    float bgY;
    float bgLayerY;
    float scrollSpeed;
    static const int BG_HEIGHT;
    static const int BG_WIDTH;
    static const int BG_IMG_H;
    static SDL_Surface* stgbg[2];
    static SDL_Surface* stgbgLowestLayer[2];
    static const DecorPair decorPairs[4];
public:
    GameBackground();
    void background_update(float dt, int currentStage);
    void background_show(int currentStage);
};
