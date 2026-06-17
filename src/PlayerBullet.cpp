#include "PlayerBullet.h"
#include "Enemy.h"
#include <iostream>
// 每个 Rank 定义一组子弹：{间隔, x偏, y偏, 宽, 高, 角度, 速度, 伤害, 弹型, 贴图, 音效, Power要求, 初始x, 初始y}

static PlayerBulletConfig g_Rank1_Bullets[] = {
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 48, 0, 0, 0, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank2_Bullets[] = {
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 48, 0, 0, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 14, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 14, 1, 2, -1, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank3_Bullets[] = {
	{5, -4, 0, 12, 48, -1.5882f, 12.0f, 30, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 4, 0, 12, 48, -1.5533f, 12.0f, 30, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 14, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 14, 1, 2, -1, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank4_Bullets[] = {
	{5, 0, 0, 12, 48, -1.6755f, 12.0f, 24, 0, 0, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 30, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.4661f, 12.0f, 24, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 14, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 14, 1, 2, -1, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank5_Bullets[] = {
	{5, 0, 0, 12, 48, -1.6755f, 12.0f, 24, 0, 0, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 30, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.4661f, 12.0f, 24, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 12, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 12, 1, 2, -1, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank6_Bullets[] = {
	{5, 0, 0, 12, 48, -1.5882f, 12.0f, 24, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 29, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.5533f, 12.0f, 24, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 9,  1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 9,  1, 2, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 12, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 12, 1, 2, -1, 0, 0, 0, 0, 0},
};

PlayerPowerRank g_PlayerPowerRanks[6] = {
	{1, 8,  g_Rank1_Bullets},
	{3, 16, g_Rank2_Bullets},
	{4, 32, g_Rank3_Bullets},
	{5, 48, g_Rank4_Bullets},
	{5, 64, g_Rank5_Bullets},
	{7, 80, g_Rank6_Bullets},
};

SDL_Surface* PlayerBullet::bulletSprites[16] = {NULL};
static SDL_Surface* etamaSheet = NULL;
SDL_Surface* PlayerBullet::reimuBulletSideRaw = NULL;
SDL_Surface* PlayerBullet::reimuBulletSide = NULL;

PlayerBullet::PlayerBullet(){
	isHit = false;
	isActive = false;
	framesLeft = 0;
	x_ = 0;
	y_ = 0;
	velX_ = 0;
	velY_ = 0;
	targetX_ = 0;
	targetY_ = 0;
	homingSpeed_ = 0;
	if(reimuBulletSideRaw == NULL){
		reimuBulletSideRaw = load_sprite("res/player00/player00.png", 0, 144, 64, 16, 64.0, 16.0);
		reimuBulletSide = rotate_image(reimuBulletSideRaw, -90.0);
	}
	sideBulletRotateAngle_ = 0.0f;
	rotatedSurface_ = NULL;
	pivotX_ = 0.0f;
	pivotY_ = 0.0f;
}
void PlayerBullet::init(PlayerBulletConfig& config){
	isActive = true;
	fireInterval_ = config.fireInterval;
	xOffset_ = config.xOffset;
	yOffset_ = config.yOffset;
	wHitbox_ = config.wHitbox;
	hHitbox_ = config.hHitbox;
	angle_ = config.angle;
	speed_ = config.speed;
	damage_ = config.damage;
	bulletType_ = config.bulletType;
	imageId_ = config.imageId;
	soundId_ = config.soundId;
	pPointRequire_ = config.pPointRequire;
	isHit = false;
	x_ = config.x;
	y_ = config.y;
	velX_ = cosf(angle_) * speed_;
	velY_ = sinf(angle_) * speed_;
	targetX_ = config.targetX;
	targetY_ = config.targetY;
	homingSpeed_ = speed_;
	framesLeft = (int)(600.0f / speed_);
}
bool PlayerBullet::bullet_move(){
	if(!isActive)	return false;
	framesLeft--;
	if (bulletType_ == 1 && targetY_ > 0) {
		// check if tracked enemy is still alive
		bool targetAlive = false;
		for (int ei = 0; ei < Enemy::onScreenCount; ei++) {
			Enemy* e = Enemy::onScreenList[ei];
			if (!e || !e->is_active()) continue;
			float edx = targetX_ - (e->get_x() + e->get_hitbox_w() * 0.5f);
			float edy = targetY_ - (e->get_y() + e->get_hitbox_h() * 0.5f);
			if (fabsf(edx) < 48.0f && fabsf(edy) < 48.0f) {
				targetAlive = true;
				break;
			}
		}
		if (!targetAlive) {
			targetY_ = 0;
		} else {
			float dx = targetX_ - x_;
			float dy = targetY_ - y_;
			float dist = sqrtf(dx*dx + dy*dy);
			if (dist < 8.0f) {
				targetY_ = 0;
			} else {
			float tx = dx / dist;
			float ty = dy / dist;
			float curSpeed = sqrtf(velX_*velX_ + velY_*velY_);
			float blend = dist / (homingSpeed_ / 4.0f);
			if (blend < 1.0f) blend = 1.0f;
			float nvx = velX_ + tx * homingSpeed_ / blend;
			float nvy = velY_ + ty * homingSpeed_ / blend;
			float nsp = sqrtf(nvx*nvx + nvy*nvy);
			if (nsp > homingSpeed_) {
				nvx = nvx / nsp * homingSpeed_;
				nvy = nvy / nsp * homingSpeed_;
			}
			velX_ = nvx;
			velY_ = nvy;
			}
		}
	}

	x_ += velX_;
	y_ += velY_;

	if (bulletType_ == 1 && reimuBulletSide != NULL) {
		if (rotatedSurface_ != NULL) SDL_FreeSurface(rotatedSurface_);
		float bw = (float)reimuBulletSide->w;
		float bh = (float)reimuBulletSide->h;
		float rotRad = atan2f(velY_, velX_) + 1.5707963f;
		double rotDeg = rotRad * 180.0 / 3.14159265;
		SDL_Surface* rotated = rotozoomSurface(reimuBulletSide, -rotDeg, 1.0, 0);
		if (rotated != NULL) {
			if (reimuBulletSide->flags & SDL_SRCCOLORKEY)
				SDL_SetColorKey(rotated, SDL_SRCCOLORKEY, reimuBulletSide->format->colorkey);
			rotatedSurface_ = rotate_image(rotated, 180.0);
			SDL_FreeSurface(rotated);
			if (rotatedSurface_ != NULL) {
				float c = cosf(rotRad);
				float s = sinf(rotRad);
				float dx = -bw * 0.5f * c - bh * 0.5f * s;
				float dy = -bw * 0.5f * s + bh * 0.5f * c;
				pivotX_ = (float)rotatedSurface_->w * 0.5f + dx;
				pivotY_ = (float)rotatedSurface_->h * 0.5f + dy;
			}
		}
	}
	int sprLeft = (int)x_ - (int)pivotX_;
	int sprTop  = (int)y_ - (int)pivotY_;
	int sprRight = sprLeft + (rotatedSurface_ ? rotatedSurface_->w : 16);
	int sprBottom = sprTop  + (rotatedSurface_ ? rotatedSurface_->h : 64);
	if(x_ < -16 || x_ > 528 || y_ < -16 || y_ > 616 || framesLeft <= 0 || isHit ||
	   sprRight < -32 || sprRight > 576 || sprBottom < -32 || sprBottom > 632){
		if (rotatedSurface_ != NULL)	SDL_FreeSurface(rotatedSurface_); rotatedSurface_ = NULL; 
		isActive = false;
		return true;
	}
	
	return false;
}
void PlayerBullet::on_hit(){
	velX_ /= 8.0f;
	velY_ /= 8.0f;
	isHit = true;
	if (rotatedSurface_ != NULL) { SDL_FreeSurface(rotatedSurface_); rotatedSurface_ = NULL; }
	isActive = false;
}
bool PlayerBullet::inUse() const{
	return isActive;
}
PlayerBullet* PlayerBullet::gen_next() const{
	return next_;
}
void PlayerBullet::set_next(PlayerBullet* next){
	next_ = next;
}
void PlayerBullet::render(){
	if(!inUse())	return;

	if (bulletType_ == 0) {
		static SDL_Surface* mainBulletBase = NULL;
		if (mainBulletBase == NULL) {
			SDL_Surface* raw = load_sprite("res/player00/player00.png", 0, 160, 64, 16, 64.0, 16.0);
			mainBulletBase = rotate_image(raw, -90.0);
			if (raw) SDL_FreeSurface(raw);
		}
		if (mainBulletBase == NULL) return;
		SDL_Rect dstRect = {(Sint16)((int)x_ - mainBulletBase->w / 2 - 6),
		                    (Sint16)((int)y_ - mainBulletBase->h / 2 - 6),
		                    (Uint16)mainBulletBase->w, (Uint16)mainBulletBase->h};
		SDL_BlitSurface(mainBulletBase, NULL, screen, &dstRect);
	} else {
		if (rotatedSurface_ == NULL) return;
		SDL_Rect dstRect = {(Sint16)((int)x_ - pivotX_),
		                    (Sint16)((int)y_ - pivotY_),
		                    (Uint16)rotatedSurface_->w, (Uint16)rotatedSurface_->h};
		SDL_BlitSurface(rotatedSurface_, NULL, screen, &dstRect);
	}
}

PlayerBulletPool::PlayerBulletPool(){
	firstAvailable = &bullet[0];
	for(int i = 0; i < POOL_SIZE - 1; i++){
		bullet[i].set_next(&bullet[i+1]);
	}
	bullet[POOL_SIZE - 1].set_next(nullptr);
}
PlayerBullet *PlayerBulletPool::create(PlayerBulletConfig& config_){
	assert(firstAvailable != nullptr);
	PlayerBullet* newBullet = firstAvailable;
	firstAvailable = newBullet->gen_next();
	newBullet->init(config_);
	return newBullet;
}
void PlayerBulletPool::update(){
	for(int i = 0; i < POOL_SIZE; i++){
		if(bullet[i].bullet_move()){
			bullet[i].set_next(firstAvailable);
			firstAvailable = &bullet[i];
		}
	}
}
void PlayerBulletPool::render(){
	for(int i = 0; i < POOL_SIZE; i++){
		bullet[i].render();
	}
}