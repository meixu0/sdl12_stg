#ifndef PLAYER_H
#define PLAYER_H

#include "UI.h"
#include "PlayerBullet.h"
#include <vector>

#define MAX_BULLETS_PER_RANK 12

class LevelManager;
class ItemManager;
class PlayerBomb;
class GameBackground;
class EnemyBulletManager;

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
	PlayerBomb* playerBomb_;
	LevelManager* levelMgr_;
	GameBackground* gameBg_;
	bool isInvincible_;
	int invTimer_;
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
	void set_level_manager(LevelManager* lm)  { levelMgr_ = lm; }
	void set_game_bg(GameBackground* bg)  { gameBg_ = bg; }
	void update_bomb(float dt);
	void render_bomb_shake();
	void render_bomb_portrait();
	bool check_hit(float bx, float by, float br);
	void hit();
	void update_invincible();
	void check_collision_with_enemy_bullets(EnemyBulletManager* ebm);
	static int get_power() { return playerPowerData; }
	static void set_power(int p) { playerPowerData = p; }
};

#endif
