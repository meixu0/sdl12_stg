#include <vector>
#include "UI.h"
struct PlayerSimpleBulletData{
	int fireInterval;
	float xOffset;
	float yOffset;
	float wHitbox;
	float hHitbox;
	float angle;
	float speed;
	int damage;
	int bulletType;
	int imageId;
	int soundId;
	int pPointRequire;
	SDL_Surface* image;
};
extern std::vector<PlayerSimpleBulletData> playerSimpleBulletData;
void init_playerSimpleBulletData();