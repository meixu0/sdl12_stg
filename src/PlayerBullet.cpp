#include "PlayerBullet.h"
#include "Enemy.h"
#include <iostream>
// 每个 Rank 定义一组子弹：{间隔, x偏, y偏, 宽, 高, 角度, 速度, 伤害, 弹型, 贴图, 音效, Power要求, 初始x, 初始y}

static PlayerBulletConfig g_Rank1_Bullets[] = {
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 13, 0, 0, 0, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank2_Bullets[] = {
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 13, 0, 0, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 4, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 4, 1, 2, -1, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank3_Bullets[] = {
	{5, -4, 0, 12, 48, -1.5882f, 12.0f, 10, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 4, 0, 12, 48, -1.5533f, 12.0f, 10, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 4, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 4, 1, 2, -1, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank4_Bullets[] = {
	{5, 0, 0, 12, 48, -1.6755f, 12.0f, 8, 0, 0, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 10, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.4661f, 12.0f, 8, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 4, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 4, 1, 2, -1, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank5_Bullets[] = {
	{5, 0, 0, 12, 48, -1.6755f, 12.0f, 8, 0, 0, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 10, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.4661f, 12.0f, 8, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 4, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 4, 1, 2, -1, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank6_Bullets[] = {
	{5, 0, 0, 12, 48, -1.5882f, 12.0f, 8, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.5708f, 12.0f, 9, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -1.5533f, 12.0f, 8, 0, 0, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 3, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 3, 1, 2, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -2.8798f, 10.0f, 4, 1, 1, -1, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 48, -0.2618f, 10.0f, 4, 1, 2, -1, 0, 0, 0, 0, 0},
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
static SDL_Surface* clip_rect(SDL_Surface* sheet, int x, int y, int w, int h){
	SDL_Surface* dst = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
		0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if(dst == NULL) return NULL;
	SDL_Rect src = {(Sint16)x, (Sint16)y, (Uint16)w, (Uint16)h};
	SDL_BlitSurface(sheet, &src, dst, NULL);
	{
		Uint32* p = (Uint32*)dst->pixels;
		for(int i = 0; i < w * h; i++){
			Uint8 r = (p[i] >> 16) & 0xFF;
			Uint8 g = (p[i] >> 8) & 0xFF;
			Uint8 b = p[i] & 0xFF;
			Uint8 a = r > g ? (r > b ? r : b) : (g > b ? g : b);
			p[i] = (p[i] & 0x00FFFFFF) | ((Uint32)a << 24);
		}
	}
	return dst;
}
PlayerBullet::PlayerBullet(){
	isHit = false;
	isActive = false;
	framesLeft = 0;
	x_ = 0; y_ = 0;
	velX_ = 0; velY_ = 0;
	targetX_ = 0; targetY_ = 0;
	homingSpeed_ = 0;

	if(reimuBulletSideRaw == NULL){
		const char* f;
		switch(playerType){
			case 0: f = "res/player00/player00.png"; break;
			case 1: f = "res/player00/player01.png"; break;
			case 2: f = "res/player00/player02.png"; break;
			case 3: f = "res/player00/player03.png"; break;
			default: f = "res/player00/player00.png"; break;
		}
		SDL_Surface* sheet = IMG_Load(f);
		if(sheet != NULL){
			reimuBulletSideRaw = clip_rect(sheet, 0, 169, 75, 19);
			if(reimuBulletSideRaw != NULL){
				SDL_Surface* t1 = rotate_90(reimuBulletSideRaw, 3);
				SDL_FreeSurface(reimuBulletSideRaw); reimuBulletSideRaw = NULL;
				SDL_Surface* t2 = rotate_90(t1, 3);
				SDL_FreeSurface(t1);
				reimuBulletSide = rotate_90(t2, 3);
				SDL_FreeSurface(t2);
				if(reimuBulletSide) SDL_SetAlpha(reimuBulletSide, SDL_SRCALPHA, 255);
			}
			SDL_FreeSurface(sheet);
		}
	}

	sideBulletRotateAngle_ = 0.0f;
	rotatedSurface_ = NULL;
	pivotX_ = 0.0f;
	pivotY_ = 0.0f;
}

PlayerBullet::~PlayerBullet(){
	if (rotatedSurface_ != NULL) {
		SDL_FreeSurface(rotatedSurface_);
		rotatedSurface_ = NULL;
	}
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
	if(isHit)      { isActive = false; return true; }
	framesLeft--;
	if (bulletType_ == 1 && targetY_ > 0) {
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
		float rotRad = atan2f(velY_, velX_) + 1.5707963f;
		double rotDeg = rotRad * 180.0 / 3.14159265;
		SDL_Surface* t = rotate_nearest(reimuBulletSide, rotDeg);
		rotatedSurface_ = rotate_90(t, 2);
		SDL_FreeSurface(t);
		if (rotatedSurface_ != NULL) {
			SDL_SetAlpha(rotatedSurface_, SDL_SRCALPHA, 255);
			pivotX_ = (float)rotatedSurface_->w * 0.5f;
			pivotY_ = (float)rotatedSurface_->h * 0.5f;
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
	isActive = false;
	if (rotatedSurface_ != NULL) { SDL_FreeSurface(rotatedSurface_); rotatedSurface_ = NULL; }
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
		// 主子弹：load_image + clip_rect + SDL_DisplayFormat + rotozoomSurface
		static SDL_Surface* mainBullet = NULL;
		if (mainBullet == NULL) {
			SDL_Surface* sheet = IMG_Load("res/player00/player00.png");
			if(sheet != NULL){
				SDL_Surface* raw = clip_rect(sheet, 0, 188, 75, 19);
				if(raw != NULL){
					mainBullet = rotate_90(raw, 3);
					SDL_FreeSurface(raw);
					if(mainBullet) SDL_SetAlpha(mainBullet, SDL_SRCALPHA, 255);
				}
				SDL_FreeSurface(sheet);
			}
		}
			
			if (mainBullet == NULL) return;
			SDL_Rect dstRect = {(Sint16)((int)x_ - mainBullet->w / 2 - 6),
			                    (Sint16)((int)y_ - mainBullet->h / 2 - 6),
			                    (Uint16)mainBullet->w, (Uint16)mainBullet->h};
			SDL_BlitSurface(mainBullet, NULL, screen, &dstRect);
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
