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
	float targetX;
	float targetY;
	float sideBulletRotateAngle;
};
struct PlayerPowerRank {
	int numBullets;
	int powerThreshold;
	PlayerBulletConfig* bullets;
};
extern PlayerPowerRank g_PlayerPowerRanks[6];
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
	float speed_;
	int damage_;
	int bulletType_;
	int imageId_;
	int soundId_;
	int pPointRequire_;
	int framesLeft;
	PlayerBullet* next_;
	bool isHit;
	float sideBulletRotateAngle_;
	float velX_, velY_;     // 预计算速度分量
	float targetX_, targetY_; // 子机弹追踪目标
	float homingSpeed_;      // 当前追踪速度
	SDL_Surface* rotatedSurface_; // 子机弹预旋转贴图
	float pivotX_, pivotY_;  // 左下角旋转中心在贴图中的偏移
public:
	PlayerBullet();
	~PlayerBullet();
	static SDL_Surface* bulletSprites[16];
	static SDL_Surface* reimuBulletSideRaw;
	static SDL_Surface* reimuBulletSide;
	void init(PlayerBulletConfig& config);
	bool bullet_move();
	bool inUse() const;
	bool is_hit() {return isHit;}
	float get_x() const { return x_; }
	float get_y() const { return y_; }
	float get_hw() const { return wHitbox_ / 2.0f; }
	float get_hh() const { return hHitbox_ / 2.0f; }
	int   get_damage() const { return damage_; }
	void on_hit();  // 击中敌机: 速度÷8, 标记回收
	PlayerBullet* gen_next() const;
	void set_next(PlayerBullet* next);
	void render();
	int  get_frames_left() const { return framesLeft; }
};
class PlayerBulletPool{
private:
	PlayerBullet* firstAvailable;
	static const int POOL_SIZE = 512;
	PlayerBullet bullet[POOL_SIZE];
public:
	PlayerBulletPool();
	void create_bullet(PlayerBulletConfig& config);
	void update();
	void render();
	PlayerBullet* bullets() { return bullet; }
	int pool_size() const { return POOL_SIZE; }
};
