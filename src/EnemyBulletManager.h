#pragma once
#include "BulletPatternType.h"
#include "EnemyBulletState.h"
#include "UI.h"
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>
#define PI 3.14159265
class ItemManager;
struct Bullet {
  float x;
  float y;
  float speedX;
  float speedY;
  int state;
  float lifeTime;
  float hitboxRadius;
  int color;
  float acceleration;
  float angularVelocity;
  int enemyType;
  int enemyID;
  bool fromBoss;
  int splitCount;
  float splitTimer;
};
struct EnemyBulletPatternDesc {
  int patternType;
  int mainCnt;
  int subCnt;
  float angleOffset;
  float angleInterval;
  float speed1;
  float speed2;
  int spriteID;
  float hitboxRadius;
  float lifeTime;
  bool fromBoss;
  bool isSplit;
  EnemyBulletPatternDesc() : patternType(0), mainCnt(0), subCnt(0), angleOffset(0), angleInterval(0),
                              speed1(0), speed2(0), spriteID(0), hitboxRadius(0), lifeTime(0),
                              fromBoss(false), isSplit(false) {}
};
struct EmitterConfig {
  float emitInterval;
  int burstCount;
  float burstInterval;
  float startDelay;
  bool isSplit;
  EnemyBulletPatternDesc patternDesc;
};
class EnemyBulletManager {
private:
  enum { POOL_SIZE = 640 };
  Bullet bullets[POOL_SIZE];
  Bullet splitPool[POOL_SIZE];
  int nextBulletIndex;
  bool bombActive_;
  float playerX_, playerY_;
  ItemManager *itemMgr_;

public:
  EnemyBulletManager();
  ~EnemyBulletManager();
  void spawn_bullet(float x, float y, float angle, float speed, int spriteID, float hitboxRadius, float lifeTime, int enemyType, int enemyID, bool fromBoss, bool isSplit = false, int splitGen = 0);
  void spawn_pattern(const EnemyBulletPatternDesc &desc, float originX, float originY, float playerX, float playerY, int enemyType, int enemyID, bool fromBoss);
  void update(float dt);
  void render();
  void clear_all();
  void despawn_all_for_spellcard();
  void convert_all_to_p_items(ItemManager *itemMgr, bool isUsingBomb = false);
  void convert_all_to_score_items(ItemManager *itemMgr, bool isUsingBomb = false);
  void freeze_all_for_bomb(ItemManager *im);
  void convert_frozen_to_player();
  void set_player_pos(float px, float py) {
    playerX_ = px;
    playerY_ = py;
  }
  Bullet *get_bullet(int i) {
    return (i >= 0 && i < POOL_SIZE) ? &bullets[i] : NULL;
  }
  std::vector<int> get_active_bullet_indices() const;
};
