#include "player.h"
Player::Player(){
	x = 0;
	y = 0;
	xVel = 0;
	yVel = 0;
}
void Player::handle_input(SDL_Event &e){
	if(e.type == SDL_KEYDOWN){
		switch(e.key.keysym.sym){//adjust velocity
			case SDLK_UP: yVel -= PLAYER_HEIGHT / 2; break;
			case SDLK_DOWN: yVel += PLAYER_HEIGHT / 2; break;
			case SDLK_LEFT: xVel -= PLAYER_WIDTH / 2; break;
			case SDLK_RIGHT: xVel += PLAYER_WIDTH / 2; break;
		}
	}
}