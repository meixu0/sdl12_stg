#include "UI.h"
#include "PlayerBullet.h"
#include <vector>

#define MAX_BULLETS_PER_RANK 12

class LevelManager;

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
	bool isShooting;
	int simpleShootCoolDown;
	int bulletFireTimers[MAX_BULLETS_PER_RANK];
	float targetEnemyX, targetEnemyY;  // 子机子弹追踪目标
	bool hasTarget;                    // 是否有有效目标
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
	PlayerPosition get_player_position() const;
};
