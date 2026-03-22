#include "UI.h"
class Player{
private:
	int x, y;//player's coordinate
	int xVel, yVel;//player's velocity in x and y direction
	static const int PLAYER_HEIGHT;
	static const int PLAYER_WIDTH;
	static SDL_Surface* player;
public:
	Player();
	void handle_input(SDL_Event &e);//handle keyboard data and change player's velocity
	void player_move();
	void show();
};
