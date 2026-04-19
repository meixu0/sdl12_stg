#include "player.h"
const int Player::PLAYER_HEIGHT = 47;
const int Player::PLAYER_WIDTH = 31;
const int Player::PLAY_HEIGHT = 600;
const int Player::PLAY_WIDTH = 544;
int Player::playerPowerData = 0;
int Player::simpleShootTimer = 0;
int Player::currentPowerLevel = 0;
bool Player::bombInUse = false;
SDL_Surface* Player::player = NULL;
//SDL_Surface* Player::simplePlayerBulletImage = NULL;
Player::Player(){
	x = 272;
	y = 553;
	xVel = 0;
	yVel = 0;
	//init_playerSimpleBulletData();
	player = load_image("res/player1.png", double(PLAYER_HEIGHT), double(PLAYER_WIDTH));
	//simplePlayerBulletImage = load_image("res/playersimplebullet1.png", 10.0, 10.0);
	PlayerBulletPool playerBulletPool;
}
void Player::simple_shoot_pressed(){
	if(!bombInUse){
		/*
		simpleShootTimer++;
		if(playerPowerData <= 8)	currentPowerLevel = 0;
		else if (playerPowerData > 8 && playerPowerData <= 16)	currentPowerLevel = 1;
		else if (playerPowerData > 16 && playerPowerData <= 32)	currentPowerLevel = 2;
		else if (playerPowerData > 32 && playerPowerData <= 64)	currentPowerLevel = 3;
		else if (playerPowerData > 64 && playerPowerData <= 128)	currentPowerLevel = 4;
		else	currentPowerLevel = 8;//todo:more levels data set
		if(simpleShootTimer % playerSimpleBulletData[currentPowerLevel].fireInterval == 0){
			//compute bullet absolute coordinate
			float spawnX = x + PLAYER_WIDTH + playerSimpleBulletData[currentPowerLevel].xOffset;
			float spawnY = y + PLAYER_HEIGHT + playerSimpleBulletData[currentPowerLevel].yOffset;
			//todo:play shoot sound
		}
		*/
		simpleShootTimer++;
		playerBulletPool.create()
	}	
}
void Player::simple_shoot_released(){
	simpleShootTimer = 0;
}
void Player::handle_input(SDL_Event &e){
	if(e.type == SDL_KEYDOWN){//when a key pressed
		switch(e.key.keysym.sym){//adjust velocity
			case SDLK_UP: yVel -= PLAYER_HEIGHT / 2; break;
			case SDLK_DOWN: yVel += PLAYER_HEIGHT / 2; break;
			case SDLK_LEFT: xVel -= PLAYER_WIDTH / 2; break;
			case SDLK_RIGHT: xVel += PLAYER_WIDTH / 2; break;
			//shoot key pressed
			case SDLK_z: simple_shoot_pressed(); break;
		}
	}else if(e.type == SDL_KEYUP){//when a key released
		    switch( e.key.keysym.sym ){
				case SDLK_UP: yVel += PLAYER_HEIGHT / 2; break;
				case SDLK_DOWN: yVel -= PLAYER_HEIGHT / 2; break;
				case SDLK_LEFT: xVel += PLAYER_WIDTH / 2; break;
				case SDLK_RIGHT: xVel -= PLAYER_WIDTH / 2; break;  
				case SDLK_z: simple_shoot_released(); break;
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