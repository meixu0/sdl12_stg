#include "player.h"
#include "LevelManager.h"
const int Player::PLAYER_HEIGHT = 47;
const int Player::PLAYER_WIDTH = 31;
const int Player::PLAY_HEIGHT = 600;
const int Player::PLAY_WIDTH = 544;
int Player::playerPowerData = 32;
int Player::currentPowerLevel = 8;
bool Player::bombInUse = false;
SDL_Surface* Player::player = NULL;
Player::Player(){
	x = 272;
	y = 553;
	xVel = 0;
	yVel = 0;
	player = load_image("res/player1.png", double(PLAYER_HEIGHT), double(PLAYER_WIDTH));
	isShooting = false;
	simpleShootCoolDown = 0;
	targetEnemyX = 272;
	targetEnemyY = 0;
	hasTarget = false;
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
			case SDLK_UP: yVel -= PLAYER_HEIGHT / 2; break;
			case SDLK_DOWN: yVel += PLAYER_HEIGHT / 2; break;
			case SDLK_LEFT: xVel -= PLAYER_WIDTH / 2; break;
			case SDLK_RIGHT: xVel += PLAYER_WIDTH / 2; break;
			case SDLK_z: isShooting = true; break;
		}
	}else if(e.type == SDL_KEYUP){
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

	for (int i = 0; i < rank->numBullets; i++) {
		update_player_bullet_collision_detection();

		if (!isShooting) continue;
		if (bulletFireTimers[i] > 0) {
			bulletFireTimers[i]--;
			continue;
		}

		PlayerBulletConfig cfg = rank->bullets[i];

		// 子机子弹: 保留配置的左右散开角度, 朝目标微调
		if (cfg.bulletType == 1 && hasTarget) {
			float baseAngle = cfg.angle;
			float dx = targetEnemyX - (spawnX + cfg.xOffset);
			float dy = targetEnemyY - (spawnY + cfg.yOffset);
			float targetAngle = atan2f(dy, dx);
			float diff = targetAngle - baseAngle;
			while (diff > 3.14159265f)  diff -= 6.2831853f;
			while (diff < -3.14159265f) diff += 6.2831853f;
			cfg.angle = baseAngle + diff * 0.3f;
		}

		cfg.x = spawnX + cfg.xOffset;
		cfg.y = spawnY + cfg.yOffset;

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

	// 遍历当前屏幕上的全部敌机 (Enemy 静态列表)
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
				if (remainHp <= 0) {
					// TODO: 掉落P点
					// TODO: 消失动画回调
					e->deactivate();
				}
				break;
			}
		}
	}
}

void Player::show(){
	apply_surface(x, y, player, screen);
}

PlayerPosition Player::get_player_position() const {
	PlayerPosition pos;
	pos.x = static_cast<float>(x + PLAYER_WIDTH / 2);
	pos.y = static_cast<float>(y + PLAYER_HEIGHT / 2);
	return pos;
}