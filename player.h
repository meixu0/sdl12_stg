#include "UI.h"
class Player{
private:
	int x, y;//player's coordinate
	int xVel, yVel;//player's velocity in x and y direction
public:
	Player();
	void handle_input();//handle keyboard data and change player's velocity
	void player_move(SDL_Event &e);
	void show();
};
