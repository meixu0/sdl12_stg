#include "PlayerBullet.h"
#include <iostream>

// ── Power 等级子弹配置 ──────────────────────────────────────────
// 每个 Rank 定义一组子弹：{间隔, x偏, y偏, 宽, 高, 角度, 速度, 伤害, 弹型, 贴图, 音效, Power要求, 初始x, 初始y}

static PlayerBulletConfig g_Rank1_Bullets[] = {
	{5, 0, 0, 12, 12, -1.5708f, 12.0f, 48, 0, 0, 0, 0, 0, 0},
};

static PlayerBulletConfig g_Rank2_Bullets[] = {
	{5, 0, 0, 12, 12, -1.5708f, 12.0f, 48, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 12, -2.0944f, 10.0f, 14, 1, 1, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.0472f, 10.0f, 14, 1, 2, -1, 0, 0, 0},
};

static PlayerBulletConfig g_Rank3_Bullets[] = {
	{5, -4, 0, 12, 12, -1.5882f, 12.0f, 30, 0, 0, -1, 0, 0, 0},
	{5, 4, 0, 12, 12, -1.5533f, 12.0f, 30, 0, 0, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -2.0944f, 10.0f, 14, 1, 1, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.0472f, 10.0f, 14, 1, 2, -1, 0, 0, 0},
};

static PlayerBulletConfig g_Rank4_Bullets[] = {
	{5, 0, 0, 12, 12, -1.6755f, 12.0f, 24, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.5708f, 12.0f, 30, 0, 0, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.4661f, 12.0f, 24, 0, 0, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -2.0944f, 10.0f, 14, 1, 1, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.0472f, 10.0f, 14, 1, 2, -1, 0, 0, 0},
};

static PlayerBulletConfig g_Rank5_Bullets[] = {
	{5, 0, 0, 12, 12, -1.6755f, 12.0f, 24, 0, 0, 0, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.5708f, 12.0f, 30, 0, 0, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.4661f, 12.0f, 24, 0, 0, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -2.0944f, 10.0f, 12, 1, 1, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.0472f, 10.0f, 12, 1, 2, -1, 0, 0, 0},
};

static PlayerBulletConfig g_Rank6_Bullets[] = {
	{5, 0, 0, 12, 12, -1.5882f, 12.0f, 24, 0, 0, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.5708f, 12.0f, 29, 0, 0, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.5533f, 12.0f, 24, 0, 0, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -2.0944f, 10.0f, 9,  1, 1, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.0472f, 10.0f, 9,  1, 2, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -2.0944f, 10.0f, 12, 1, 1, -1, 0, 0, 0},
	{5, 0, 0, 12, 12, -1.0472f, 10.0f, 12, 1, 2, -1, 0, 0, 0},
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

PlayerBullet::PlayerBullet(){
	isHit = false;
	isActive = false;
	framesLeft = 0;
	x_ = 0;
	y_ = 0;
	velX_ = 0;
	velY_ = 0;
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
	framesLeft = (int)(600.0f / speed_);
}
bool PlayerBullet::bullet_move(){
	if(!isActive)	return false;
	framesLeft--;
	x_ += velX_;
	y_ += velY_;
	// 出界回收
	if(x_ < -16 || x_ > 528 || y_ < -16 || y_ > 616 || framesLeft <= 0 || isHit){
		isActive = false;
		return true;
	}
	return false;
}
void PlayerBullet::on_hit(){
	velX_ /= 8.0f;
	velY_ /= 8.0f;
	isHit = true;
	isActive = false;  // 立即从碰撞检测中移除, 避免同一颗子弹多次命中
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
	// 加载整张 etama.png，用 colorkey 做透明（与 zako 同样方式）
	if (etamaSheet == NULL) {
		SDL_Surface* raw = IMG_Load("res/etama/etama.png");
		if (raw != NULL) {
			etamaSheet = SDL_DisplayFormat(raw);
			SDL_FreeSurface(raw);
			SDL_SetColorKey(etamaSheet, SDL_SRCCOLORKEY,
				SDL_MapRGB(etamaSheet->format, 0, 0, 0));
		}
	}
	if (etamaSheet == NULL) return;

	if (bulletType_ == 0) {
		// 主弹: col1, 始终正上, 无需旋转
		SDL_Rect srcRect = {(Sint16)16, 0, 16, 16};
		SDL_Rect dstRect = {(Sint16)((int)x_ - 8), (Sint16)((int)y_ - 8), 16, 16};
		SDL_BlitSurface(etamaSheet, &srcRect, screen, &dstRect);
	} else {
		// 子机弹: col5, 旋转到速度方向
		SDL_Rect srcRect = {(Sint16)(5 * 16), 0, 16, 16};
		SDL_Surface* baseSprite = SDL_CreateRGBSurface(SDL_SWSURFACE,
			16, 16, etamaSheet->format->BitsPerPixel,
			etamaSheet->format->Rmask, etamaSheet->format->Gmask,
			etamaSheet->format->Bmask, 0);
		if (baseSprite != NULL) {
			SDL_BlitSurface(etamaSheet, &srcRect, baseSprite, NULL);
			SDL_SetColorKey(baseSprite, SDL_SRCCOLORKEY,
				SDL_MapRGB(baseSprite->format, 0, 0, 0));
			// 旋转角度: 速度方向 - 原图朝上(-PI/2)
			float rotRad = atan2f(velY_, velX_) + 1.5707964f;
			double rotDeg = rotRad * 180.0 / 3.14159265;
			SDL_Surface* rotated = rotozoomSurface(baseSprite, rotDeg, 1.0, 0);
			SDL_FreeSurface(baseSprite);
			if (rotated != NULL) {
				SDL_SetColorKey(rotated, SDL_SRCCOLORKEY,
					SDL_MapRGB(rotated->format, 0, 0, 0));
				SDL_Rect dstRect = {(Sint16)((int)x_ - rotated->w / 2),
				                    (Sint16)((int)y_ - rotated->h / 2),
				                    (Uint16)rotated->w, (Uint16)rotated->h};
				SDL_BlitSurface(rotated, NULL, screen, &dstRect);
				SDL_FreeSurface(rotated);
				std::cout << "render bullet at (" << x_ << ", " << y_ << ") with angle " << rotDeg << std::endl;
			}
		}
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