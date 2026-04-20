#include <vector>
#include <assert.h>
#include "UI.h"
struct PlayerBulletConfig{
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
	float x;
	float y;
};
class PlayerBullet{
private:
	bool isActive;
	int fireInterval_;
	float x_;
	float y_;
	float xOffset_;
	float yOffset_;
	float wHitbox_;
	float hHitbox_;
	float angle_;
	float speed_;//pixels per frame
	int damage_;
	int bulletType_;
	int imageId_;
	int soundId_;
	int pPointRequire_;
	int framesLeft;
	PlayerBullet* next_;
	bool isHit;
public:
	PlayerBullet();
    static SDL_Surface* simplePlayerBulletImage;
	void init(PlayerBulletConfig& config);
	bool bullet_move();
	bool inUse() const;
	PlayerBullet* gen_next() const;
	void set_next(PlayerBullet* next);
	void render();
};
class PlayerBulletPool{
private:
	PlayerBullet* firstAvailable;
	static const int POOL_SIZE = 1000;
	PlayerBullet bullet[POOL_SIZE];
public:
	PlayerBulletPool();
	PlayerBullet* create(PlayerBulletConfig& config_);
	void update();
	void render();
};