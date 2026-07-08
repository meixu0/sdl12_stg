#include "UI.h"
#include "ItemType.h"

class Player;

class ItemManager{
private:
    static Mix_Chunk* powerup00;
    static Mix_Chunk* item00;

    struct Item{
        float x;
        float y;
        float speedX;
        float speedY;
        float targetX;     // state 2 lerp target
        float targetY;
        float startX;      // state 2 lerp start
        float startY;
        float timer;       // state 2 animation timer (seconds)
        int type;
        bool isActive;
        int state;         // 0: normal physics, 1: magnet to player, 2: spawn lerp
    };

    static const int MAX_ITEMS = 513;
    static const float STATE2_DURATION;  // 1.0 second lerp
    static const float MAGNET_SPEED;     // px/s toward player
    static const float GRAVITY;          // px/s² downward acceleration
    static const float MAX_FALL_SPEED;   // px/s terminal velocity
    static const float UPWARD_DRIFT;     // px/s initial upward drift
    static const float COLLECT_RADIUS;   // player collection hitbox half-size

    Item items[MAX_ITEMS];
    int nextItemIndex;
    Player* playerPtr;

    // Game state
    static int score;
    static int lives;
    static int bombs;
    static int powerItemCountForScore;
    static const int MAX_POWER = 128;
    static const int POWER_UP_THRESHOLDS[11];
    static const int POWER_ITEM_SCORE[31];

public:
    ItemManager();
    ~ItemManager();
    void set_player(Player* p);
    void spawn_item(float x, float y, int itemType, int state = 0);
    void update(float dt);
    void render();
    void clear_all();

    static int get_score() { return score; }
    static void add_score(int val) { score += val; }
    static int get_lives() { return lives; }
    static int get_bombs() { return bombs; }
    static void use_bomb() { if (bombs > 0) bombs--; }
};
