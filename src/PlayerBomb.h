#ifndef PLAYERBOMB_H
#define PLAYERBOMB_H
#include "UI.h"
enum BombType {
    BOMB_REIMU_A = 0,
    BOMB_REIMU_B = 1,
    BOMB_MARISA_A = 2,
    BOMB_MARISA_B = 3,
};
struct BombParams {
    int duration;
    int invincibility;
    int shakeIntensity;
};
static const BombParams bombParams[4] = {
    {360, 360, 8},
    {200, 200, 6},
    {300, 300, 7},
    {360, 360, 6},
};

class LevelManager;
class ItemManager;
class GameBackground;
class PlayerBomb {
private:
    int type_;
    int playerType_;
    int duration_;
    int invincibility_;
    int timer_;
    int invTimer_;
    bool active_;
    int shakeX_, shakeY_;

    LevelManager* levelMgr_;
    ItemManager* itemMgr_;

    void clear_zako();
    void convert_bullets();

    void draw_portrait(const char* sheetFile);
    void reimuA_bomb_animation(GameBackground* bg);
    void reimuB_bomb_animation(GameBackground* bg);
    void marisaA_bomb_animation(GameBackground* bg);
    void marisaB_bomb_animation(GameBackground* bg);

public:
    PlayerBomb();
    ~PlayerBomb();
    void trigger(int type, int playerType, float x, float y, LevelManager* lm, ItemManager* im);
    void update(float dt);
    void render_portrait();
    void render_shake();
    bool isExpired() const;
    void on_expire();
    bool isActive()  const { return active_; }
    bool isInvincible() const { return invTimer_ > 0; }
    int  invincibility() const { return invincibility_; }
    int  shakeX()    const { return shakeX_; }
    int  shakeY()    const { return shakeY_; }
};
#endif
