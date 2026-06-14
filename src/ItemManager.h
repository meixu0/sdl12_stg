#include "UI.h"
class ItemManager{
private:
    struct Item{
        float x;
        float y;
        int type;
        float speedY;
        bool isActive;
    };
    static const int MAX_ITEMS = 64;
    Item items[MAX_ITEMS];
public:
    ItemManager();
    void update(float dt);
    void render();
    void spawn_item(float x, float y, int itemType);
    void clear_all();
};