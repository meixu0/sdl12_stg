#include "PlayerBullet.h"
SDL_Surface* PlayerBullet::simplePlayerBulletImage = load_image("res/playersimplebullet1.png", 10.0, 10.0);
PlayerBullet::PlayerBullet(){
	isHit = false;
}
void PlayerBullet::init(PlayerBulletConfig& config){
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
	if((!inUse()))	return false;
	framesLeft--;
	y_ -= speed_;
	apply_surface(x_, y_, simplePlayerBulletImage);
	return framesLeft == 0;
}
bool PlayerBullet::inUse() const{
	return ((!isHit) || (framesLeft > 0));
}
PlayerBullet* PlayerBullet::gen_next() const{
	return next_;
}
void PlayerBullet::set_next(PlayerBullet* next){
	next_ = next;
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
void PlayerBulletPool::render(){
	for(int i = 0; i < POOL_SIZE; i++){
		if(bullet[i].bullet_move()){
			bullet[i].set_next(firstAvailable);
			firstAvailable = &bullet[i];
		}
	}
}