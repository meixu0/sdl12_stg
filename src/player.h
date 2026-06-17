#include "UI.h"
#include "PlayerBullet.h"
#include <vector>

#define MAX_BULLETS_PER_RANK 12

class LevelManager;
class ItemManager;

struct PlayerPosition {
    float x;
    float y;
};
class Player{
private:
	int x, y;
	int xVel, yVel;
	static const int PLAYER_HEIGHT;
	static const int PLAYER_WIDTH;
	static const int PLAY_HEIGHT;
	static const int PLAY_WIDTH;
	static int currentPowerLevel;
	static bool bombInUse;
	static int playerPowerData;
	static SDL_Surface* player;
	static SDL_Surface* reimuImageMiddle[4];
	static SDL_Surface* reimuImageSide[7];
	static SDL_Surface* reimuBulletSide;  // 子机子弹贴图 (64×16 → 旋转90° → 16×64)
	bool isShooting;
	int simpleShootCoolDown;
	int bulletFireTimers[MAX_BULLETS_PER_RANK];
	float animTimer;  // 自机动画计时器
	float targetEnemyX, targetEnemyY;
	bool hasTarget;
	float optionStreamAngle_;
	ItemManager* itemMgr;
public:
	Player();
	PlayerBulletPool* playerBulletPool_;
	void init_player_bullet_pool(PlayerBulletPool *poolPtr);
	void handle_input(SDL_Event &e);
	void player_move();
	void update_simple_shoot();
	void set_homing_target(float tx, float ty);
	void update_player_bullet_collision_detection();
	void show();
	int get_player_x_vel() const { return xVel; }
	PlayerPosition get_player_position();
	void set_item_manager(ItemManager* mgr) { itemMgr = mgr; }
	static int get_power() { return playerPowerData; }
	static void set_power(int p) { playerPowerData = p; }
};
