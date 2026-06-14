#include "UI.h"
#include "ItemType.h"
class ItemManager{
private:
    static SDL_Surface* itemPowerSmallImage;
    static SDL_Surface* itemPointImage;
    static SDL_Surface* itemPowerBigImage;
    static SDL_Surface* itemBombImage;
    static SDL_Surface* itemFullPowerImage;
    static SDL_Surface* itemLifeImage;
    static SDL_Surface* itemPointBulletImage;

    struct Item{
        float x;
        float y;
        int type;
        float speedX;
        float speedY;
        bool isActive;
    };
    static const int MAX_ITEMS = 513;
    Item items[MAX_ITEMS];
public:
    ItemManager();
    void update(float dt);
    void render();
    void spawn_item(float x, float y, int itemType);
    void clear_all();
};