#include "PlayerBullet.h"
int fireInterval;
float xOffset;
float yOffset;
float wHitbox;
float hHitbox;
float angle;
float speed = 5;
int damage = 1;
int bulletType;
int imageId;
int soundId;
int pPointRequire = 8;
std::vector<PlayerSimpleBulletData> playerSimpleBulletData(9);
void init_playerSimpleBulletData(){
	for(int i = 0;i < 9;i++){
		playerSimpleBulletData[i].fireInterval = 5;
		playerSimpleBulletData[i].xOffset = 0;
		playerSimpleBulletData[i].yOffset = 0;
		playerSimpleBulletData[i].wHitbox = 10;
		playerSimpleBulletData[i].hHitbox = 10;
		playerSimpleBulletData[i].angle = 0;
		playerSimpleBulletData[i].speed = speed;
		//todo:set more simple player bullet speed
		//speed *= 1.2;
		playerSimpleBulletData[i].damage = damage;
		damage++;
		playerSimpleBulletData[i].bulletType = 1;
		playerSimpleBulletData[i].imageId = 1;
		playerSimpleBulletData[i].soundId = 1;
		playerSimpleBulletData[i].pPointRequire = pPointRequire;
		pPointRequire *= 2;
		playerSimpleBulletData[i].image = load_image("res/playersimplebullet1.png", double(wHitbox), double(hHitbox));
	}
}