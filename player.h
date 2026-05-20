#include "UI.h"
#include "PlayerBullet.h"
#include <vector>

struct PlayerPosition {
    float x;
    float y;
};
class Player{
private:
	int x, y;//player's coordinate
	int xVel, yVel;//player's velocity in x and y direction
	static const int PLAYER_HEIGHT;
	static const int PLAYER_WIDTH;
	static const int PLAY_HEIGHT;
	static const int PLAY_WIDTH;
	static int currentPowerLevel;
	static bool bombInUse;
	static int playerPowerData;
	static SDL_Surface* player;
	//static SDL_Surface* simplePlayerBulletImage;
	bool isShooting;
	int simpleShootCoolDown;
public:
	Player();
	//void simple_shoot_pressed();
	//void simple_shoot_released();
	PlayerBulletPool* playerBulletPool_;
	void init_player_bullet_pool(PlayerBulletPool *poolPtr);
	void handle_input(SDL_Event &e);//handle keyboard data and change player's velocity
	void player_move();
	void update_simple_shoot();
	void show();
	PlayerPosition get_player_position() const;  // 获取玩家实时位置
};
