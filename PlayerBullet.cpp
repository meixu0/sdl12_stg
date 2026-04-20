#include "PlayerBullet.h"
SDL_Surface* PlayerBullet::simplePlayerBulletImage = NULL;
PlayerBullet::PlayerBullet(){
	isHit = false;
	isActive = false;
	framesLeft = 0;
	x_ = 0;
	y_ = 0;
}
void PlayerBullet::init(PlayerBulletConfig& config){
	isActive = true;
	fireInterval_ = config.fireInterval;
	xOffset_ = config.xOffset;
	yOffset_ = config.yOffset;
	wHitbox_ = config.wHitbox;
	hHitbox_ = config.hHitbox;
	angle_ = config.angle;
	speed_ = config.speed;//pixels per frame
	damage_ = config.damage;
	bulletType_ = config.bulletType;
	imageId_ = config.imageId;
	soundId_ = config.soundId;
	pPointRequire_ = config.pPointRequire;
	isHit = false;
	x_ = config.x;
	y_ = config.y;
	framesLeft = y_ / speed_;
}
bool PlayerBullet::bullet_move(){
	if(!isActive)	return false;
	framesLeft--;
	y_ -= speed_;
	if(framesLeft <= 0 || isHit){//death condition
		isActive = false;
		return true;
	}
	return false;
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
	if(simplePlayerBulletImage == NULL)	simplePlayerBulletImage = load_image("res/playersimplebullet1.png", 10.0, 10.0);
	apply_surface(x_, y_, simplePlayerBulletImage, screen);
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