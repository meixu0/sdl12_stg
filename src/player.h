#ifndef PLAYER_H
#define PLAYER_H

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
	static const float PLAYER_HEIGHT;
	static const float PLAYER_WIDTH;
	static const int PLAY_HEIGHT;
	static const int PLAY_WIDTH;
	static int currentPowerLevel;
	static bool bombInUse;
	static int playerPowerData;
	static SDL_Surface* playerSheet;
	static SDL_Surface* playerSheetMirror;
	static SDL_Rect middleSrcRects[3];
	static SDL_Rect sideSrcRects[7];
	bool isShooting;
	int simpleShootCoolDown;
	int bulletFireTimers[MAX_BULLETS_PER_RANK];
	float animTimer;
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

#endif
