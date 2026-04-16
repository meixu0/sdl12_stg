#include "UI.h"
#include "PlayerBullet.h"
#include <vector>
class Player{
private:
	int x, y;//player's coordinate
	int xVel, yVel;//player's velocity in x and y direction
	static const int PLAYER_HEIGHT;
	static const int PLAYER_WIDTH;
	static const int PLAY_HEIGHT;
	static const int PLAY_WIDTH;
	static int simpleShootTimer;
	static int currentPowerLevel;
	static bool bombInUse;
	static int playerPowerData;
	static SDL_Surface* player;
public:
	Player();
	void simple_shoot_pressed();
	void simple_shoot_released();
	void handle_input(SDL_Event &e);//handle keyboard data and change player's velocity
	void player_move();
	void show();
};
