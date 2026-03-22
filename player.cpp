#include "player.h"
const int Player::PLAYER_HEIGHT = 47;
const int Player::PLAYER_WIDTH = 31;
const int Player::PLAY_HEIGHT = 600;
const int Player::PLAY_WIDTH = 544;
SDL_Surface* Player::player = NULL;
Player::Player(){
	x = 272;
	y = 553;
	xVel = 0;
	yVel = 0;
	player = load_image("res/player1.png", double(PLAYER_HEIGHT), double(PLAYER_WIDTH));
}
void Player::handle_input(SDL_Event &e){
	if(e.type == SDL_KEYDOWN){//when a key pressed
		switch(e.key.keysym.sym){//adjust velocity
			case SDLK_UP: yVel -= PLAYER_HEIGHT / 2; break;
			case SDLK_DOWN: yVel += PLAYER_HEIGHT / 2; break;
			case SDLK_LEFT: xVel -= PLAYER_WIDTH / 2; break;
			case SDLK_RIGHT: xVel += PLAYER_WIDTH / 2; break;
		}
	}else if(e.type == SDL_KEYUP){//when a key released
		    switch( e.key.keysym.sym ){
				case SDLK_UP: yVel += PLAYER_HEIGHT / 2; break;
				case SDLK_DOWN: yVel -= PLAYER_HEIGHT / 2; break;
				case SDLK_LEFT: xVel += PLAYER_WIDTH / 2; break;
				case SDLK_RIGHT: xVel -= PLAYER_WIDTH / 2; break;   
        }        
	}
}
void Player::player_move(){
	x += xVel;
	if((x < 0) || (x + PLAYER_WIDTH > PLAY_WIDTH))	x -= xVel;
	y += yVel;
	if((y < 0) || (y + PLAYER_HEIGHT > PLAY_HEIGHT))	y -= yVel;
}
void Player::show(){
	apply_surface(x, y, player, screen);
}