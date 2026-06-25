#include "player.h"
#include "LevelManager.h"
#include "ItemManager.h"
const float Player::PLAYER_HEIGHT = 48*1.5625f;  // 48px → 75px
const float Player::PLAYER_WIDTH = 32*1.5625f;  // 32px → 50px
const int Player::PLAY_HEIGHT = 600;
const int Player::PLAY_WIDTH = 544;
int Player::playerPowerData = 0;
int Player::currentPowerLevel = 0;
bool Player::bombInUse = false;
SDL_Surface* Player::player = NULL;
SDL_Surface* Player::reimuImageMiddle[4] = {NULL};
SDL_Surface* Player::reimuImageSide[7] = {NULL};
SDL_Surface* Player::reimuBulletSide = NULL;
Player::Player(){
	x = 272;
	y = 600 - (int)PLAYER_WIDTH - 10;
	xVel = 0;
	yVel = 0;
	SDL_Surface* raw;
	switch (playerType){
		case 0:
			for(int i = 0;i < 3; i++){
				reimuImageMiddle[i] = load_sprite("res/player00/player00.png", i * 32, 0, 32, 48, PLAYER_HEIGHT, PLAYER_WIDTH);
			}
			for(int i = 0;i < 7; i++){
				reimuImageSide[i] = load_sprite("res/player00/player00.png", i * 32, 48, 32, 48, PLAYER_HEIGHT, PLAYER_WIDTH);
			}
			raw = load_sprite("res/player00/player00.png", 0, 144, 64, 16, 64.0, 16.0);
			break;
		case 1:
			for(int i = 0;i < 3; i++){
				reimuImageMiddle[i] = load_sprite("res/player00/player01.png", i * 32, 0, 32, 48, PLAYER_HEIGHT, PLAYER_WIDTH);
			}
			for(int i = 0;i < 7; i++){
				reimuImageSide[i] = load_sprite("res/player00/player01.png", i * 32, 48, 32, 48, PLAYER_HEIGHT, PLAYER_WIDTH);
			}
			raw = load_sprite("res/player00/player01.png", 0, 144, 64, 16, 64.0, 16.0);
			break;
		case 2:
			for(int i = 0;i < 3; i++){
				reimuImageMiddle[i] = load_sprite("res/player00/player02.png", i * 32, 0, 32, 48, PLAYER_HEIGHT, PLAYER_WIDTH);
			}
			for(int i = 0;i < 7; i++){
				reimuImageSide[i] = load_sprite("res/player00/player02.png", i * 32, 48, 32, 48, PLAYER_HEIGHT, PLAYER_WIDTH);
			}
			raw = load_sprite("res/player00/player02.png", 0, 144, 64, 16, 64.0, 16.0);
			break;
	}
	
	reimuBulletSide = rotate_image(raw, 90.0);
	if (raw) SDL_FreeSurface(raw);
	isShooting = false;
	simpleShootCoolDown = 0;
	animTimer = 0.0f;
	targetEnemyX = 272;
	targetEnemyY = 0;
	hasTarget = false;
	optionStreamAngle_ = -1.5707963f;
	itemMgr = NULL;
	for (int i = 0; i < MAX_BULLETS_PER_RANK; i++) {
		bulletFireTimers[i] = 0;
	}
}
void Player::init_player_bullet_pool(PlayerBulletPool *poolPtr){
	playerBulletPool_ = poolPtr;
}
void Player::handle_input(SDL_Event &e){
	if(e.type == SDL_KEYDOWN){
		switch(e.key.keysym.sym){
			case SDLK_UP: yVel -= PLAYER_HEIGHT / 4; break;
			case SDLK_DOWN: yVel += PLAYER_HEIGHT / 4; break;
			case SDLK_LEFT: xVel -= PLAYER_WIDTH / 4; break;
			case SDLK_RIGHT: xVel += PLAYER_WIDTH / 4; break;
			case SDLK_z: isShooting = true; break;
		}
	}else if(e.type == SDL_KEYUP){
		    switch( e.key.keysym.sym ){
				case SDLK_UP: yVel += PLAYER_HEIGHT / 4; break;
				case SDLK_DOWN: yVel -= PLAYER_HEIGHT / 4; break;
				case SDLK_LEFT: xVel += PLAYER_WIDTH / 4; break;
				case SDLK_RIGHT: xVel -= PLAYER_WIDTH / 4; break;  
				case SDLK_z: isShooting = false; break;
        }        
	}
}
void Player::player_move(){
	x += xVel;
	if((x < 0) || (x + PLAYER_HEIGHT > PLAY_WIDTH))	x -= xVel;
	y += yVel;
	if((y < 0) || (y + PLAYER_WIDTH > PLAY_HEIGHT))	y -= yVel;
	animTimer += 1.0f / 60.0f;
}
void Player::update_simple_shoot(){
	PlayerPowerRank* rank = &g_PlayerPowerRanks[0];
	for (int i = 0; i < 6; i++) {
		if (playerPowerData >= g_PlayerPowerRanks[i].powerThreshold)
			rank = &g_PlayerPowerRanks[i];
	}

	if (!isShooting) {
		for (int i = 0; i < MAX_BULLETS_PER_RANK; i++)
			bulletFireTimers[i] = 0;
	}

	float spawnX = (float)(x + PLAYER_WIDTH / 2);
	float spawnY = (float)y;
	float leftSideX = x - 12.0;
	float rightSideX = x + PLAYER_WIDTH + 12.0;

	for (int i = 0; i < rank->numBullets; i++) {
		update_player_bullet_collision_detection();

		if (!isShooting) continue;
		if (bulletFireTimers[i] > 0) {
			bulletFireTimers[i]--;
			continue;
		}

		PlayerBulletConfig cfg = rank->bullets[i];
		cfg.x = spawnX + cfg.xOffset;
		cfg.y = spawnY + cfg.yOffset;

		if(cfg.bulletType == 1){
			if(hasTarget){
				float dx = targetEnemyX - spawnX;
				float dy = targetEnemyY - spawnY;
				float desiredAngle = atan2f(dy, dx);
				float diff = desiredAngle - optionStreamAngle_;
				while(diff > 3.14159265f) diff -= 6.2831853f;
				while(diff < -3.14159265f) diff += 6.2831853f;
				optionStreamAngle_ += diff * 0.08f;
				cfg.targetX = targetEnemyX;
				cfg.targetY = targetEnemyY;
			} else {
				float diff = -1.5707963f - optionStreamAngle_;
				while(diff > 3.14159265f) diff -= 6.2831853f;
				while(diff < -3.14159265f) diff += 6.2831853f;
				optionStreamAngle_ += diff * 0.05f;
			}
			cfg.angle = optionStreamAngle_;
		}
		playerBulletPool_->create(cfg);
		bulletFireTimers[i] = cfg.fireInterval;
	}
}

void Player::set_homing_target(float tx, float ty){
	targetEnemyX = tx;
	targetEnemyY = ty;
	hasTarget = (ty > 0);
}

void Player::update_player_bullet_collision_detection(){
	if (playerBulletPool_ == NULL) return;

	PlayerBullet* bullets = playerBulletPool_->bullets();
	int poolSize = playerBulletPool_->pool_size();

		for (int ei = 0; ei < Enemy::onScreenCount; ei++) {
		Enemy* e = Enemy::onScreenList[ei];
		if (!e) continue;

		for (int bi = 0; bi < poolSize; bi++) {
			PlayerBullet& b = bullets[bi];
			if (!b.inUse()) continue;

			if (e->check_bullet_hit(b.get_x(), b.get_y(), b.get_hw(), b.get_hh())) {
				int dmg = b.get_damage();
				b.on_hit();
				int remainHp = e->take_damage(dmg);
				if (remainHp <= 0 && !e->is_spellcard_boss()) {
					if (itemMgr != NULL) {
						float ex = e->get_x() + e->get_hitbox_w() * 0.5f;
						float ey = e->get_y() + e->get_hitbox_h() * 0.5f;
						itemMgr->spawn_item(ex, ey, ITEM_POWER_SMALL, 2);
					}
					e->deactivate();
				}
				break;
			}
		}
	}
}

void Player::show(){
	int frameCount;
	SDL_Surface** frames;
	bool mirror = false;

	if (xVel > 0) {
		frameCount = 7;
		frames = reimuImageSide;
	} else if (xVel < 0) {
		frameCount = 7;
		frames = reimuImageSide;
		mirror = true;
	} else {
		frameCount = 3;
		frames = reimuImageMiddle;
	}

	int tick = ((int)(animTimer * 4.0f)) % (frameCount * 2 - 2);
	int idx = (tick > frameCount - 1) ? (frameCount * 2 - 2 - tick) : tick;

	SDL_Surface* sprite = frames[idx];
	if (sprite == NULL) return;

	if (mirror)
		apply_surface_mirror(x, y, sprite, screen);
	else
		apply_surface(x, y, sprite, screen);
}

PlayerPosition Player::get_player_position(){
	PlayerPosition pos;
	pos.x = (float)(x + PLAYER_WIDTH / 2);
	pos.y = (float)(y + PLAYER_HEIGHT / 2);
	return pos;
}