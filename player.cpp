#include "player.h"
const int Player::PLAYER_HEIGHT = 47;
const int Player::PLAYER_WIDTH = 31;
const int Player::PLAY_HEIGHT = 600;
const int Player::PLAY_WIDTH = 544;
int Player::playerPowerData = 0;
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
	isShooting = false;
}
void Player::init_player_bullet_pool(PlayerBulletPool *poolPtr){
	playerBulletPool_ = poolPtr;
}
void Player::handle_input(SDL_Event &e){
	if(e.type == SDL_KEYDOWN){//when a key pressed
		switch(e.key.keysym.sym){//adjust velocity
			case SDLK_UP: yVel -= PLAYER_HEIGHT / 2; break;
			case SDLK_DOWN: yVel += PLAYER_HEIGHT / 2; break;
			case SDLK_LEFT: xVel -= PLAYER_WIDTH / 2; break;
			case SDLK_RIGHT: xVel += PLAYER_WIDTH / 2; break;
			//shoot key pressed
			case SDLK_z: isShooting = true; break;
		}
	}else if(e.type == SDL_KEYUP){//when a key released
		    switch( e.key.keysym.sym ){
				case SDLK_UP: yVel += PLAYER_HEIGHT / 2; break;
				case SDLK_DOWN: yVel -= PLAYER_HEIGHT / 2; break;
				case SDLK_LEFT: xVel += PLAYER_WIDTH / 2; break;
				case SDLK_RIGHT: xVel -= PLAYER_WIDTH / 2; break;  
				case SDLK_z: isShooting = false; break;
        }        
	}
}
void Player::player_move(){
	x += xVel;
	if((x < 0) || (x + PLAYER_WIDTH > PLAY_WIDTH))	x -= xVel;
	y += yVel;
	if((y < 0) || (y + PLAYER_HEIGHT > PLAY_HEIGHT))	y -= yVel;
}
void Player::update_simple_shoot(){
	if(simpleShootCoolDown > 0)	simpleShootCoolDown--;
	if(isShooting && simpleShootCoolDown == 0){
		PlayerBulletConfig currentConfig;
		currentConfig.x = x + (PLAYER_WIDTH / 2);
		currentConfig.y = y;
		currentConfig.speed = 20;
		currentConfig.damage = 1;
		//todo: more image res
		playerBulletPool_->create(currentConfig);
		//todo:more simple power damage value and cd value
		simpleShootCoolDown = 5;//5 bullets per frame
	}
}
void Player::show(){
	apply_surface(x, y, player, screen);
}