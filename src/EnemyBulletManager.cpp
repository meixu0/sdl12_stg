#include "EnemyBulletManager.h"
#include "ItemManager.h"
#include "ItemType.h"
#include <iostream>
static SDL_Surface *bulletSheet = NULL;
static SDL_Rect whiteEnemyBullet = {0, 57, 16, 16};
static SDL_Surface *etamaHalfSheet = NULL;
static SDL_Rect bulletSrc[4] = {
    {128, 64, 16, 16},  // 红
    {128, 80, 16, 16},  // 绿
    {128, 96, 16, 16},  // 蓝
    {128, 112, 16, 16}, // 黄
};
static float randf() { return (float)rand() / (float)RAND_MAX; }
EnemyBulletManager::EnemyBulletManager() {
  nextBulletIndex = 0;
  bombActive_ = false;
  playerX_ = 272.0f;
  playerY_ = 0.0f;
  itemMgr_ = NULL;
  for (int i = 0; i < POOL_SIZE; i++) {
    bullets[i].fromBoss = false;
    bullets[i].state = SLEEPING;
    splitPool[i].fromBoss = false;
    splitPool[i].state = SLEEPING;
  }
  if (etamaHalfSheet == NULL)
    etamaHalfSheet = IMG_Load("res/etama/etamahalf.png");
}

EnemyBulletManager::~EnemyBulletManager() {}

void EnemyBulletManager::spawn_bullet(float originX, float originY, float angle, float speed, int spriteID, float hitboxRadius, float lifeTime, int enemyType, int enemyID, bool fromBoss, bool isSplit, int splitGen) {
  for (int i = 0; i < POOL_SIZE; i++) {
    int idx = (nextBulletIndex + i) % POOL_SIZE;
    if (bullets[idx].state == SLEEPING) {
      Bullet &b = bullets[idx];
      b.x = originX;
      b.y = originY;
      b.speedX = cosf(angle) * speed;
      b.speedY = sinf(angle) * speed;
      b.state = bombActive_ ? FROZEN : ALIVE;
      if (bombActive_) {
        b.speedX = 0.0f;
        b.speedY = 0.0f;
      }
      b.lifeTime = lifeTime;
      b.hitboxRadius = hitboxRadius;
      b.color = spriteID;
      b.acceleration = 0.0f;
      b.angularVelocity = 0.0f;
      b.enemyType = enemyType;
      b.enemyID = enemyID;
      b.fromBoss = fromBoss;
      b.splitCount = splitGen;
      b.splitTimer = isSplit ? 1.5f : 0.0f;
      nextBulletIndex = (idx + 1) % POOL_SIZE;
      return;
    }
  }
}

void EnemyBulletManager::spawn_pattern(const EnemyBulletPatternDesc &desc, float originX, float originY, float playerX, float playerY, int enemyType, int enemyID, bool fromBoss) {
  float aimAngle = atan2f(playerY - originY, playerX - originX);
  for (int layer = 0; layer < desc.subCnt; layer++) {
    float layerSpeed = desc.speed1;
    if (desc.subCnt > 1)
      layerSpeed = desc.speed1 - (desc.speed1 - desc.speed2) * layer / (desc.subCnt - 1);
    for (int i = 0; i < desc.mainCnt; i++) {
      float angle, speed;
      switch (desc.patternType) {
      case PAT_FAN_AIMED:
        angle = desc.angleOffset;
        angle +=
            (i % 2 == 0 ? 1.0f : -1.0f) * (i / 2 + i % 2) * desc.angleInterval;
        angle += aimAngle;
        speed = layerSpeed;
        break;
      case PAT_FAN:
        angle = desc.angleOffset;
        angle +=
            (i % 2 == 0 ? 1.0f : -1.0f) * (i / 2 + i % 2) * desc.angleInterval;
        speed = layerSpeed;
        break;
      case PAT_CIRCLE_AIMED:
        angle = aimAngle + desc.angleOffset + i * 2 * PI / desc.mainCnt + layer * desc.angleInterval;
        speed = layerSpeed;
        break;
      case PAT_CIRCLE:
        angle = desc.angleOffset + i * 2 * PI / desc.mainCnt + layer * desc.angleInterval;
        speed = layerSpeed;
        break;
      case PAT_RANDOM_ANGLE:
        angle = desc.angleOffset + randf() * (desc.angleInterval - desc.angleOffset);
        speed = layerSpeed;
        break;
      case PAT_RANDOM_SPEED:
        angle = desc.angleOffset + i * 2 * PI / desc.mainCnt;
        speed = desc.speed2 + randf() * (desc.speed2 - desc.speed1);
        break;
      case PAT_RANDOM_BOTH:
        angle = desc.angleOffset + randf() * (desc.angleInterval - desc.angleOffset);
        speed = desc.speed2 + randf() * (desc.speed2 - desc.speed1);
        break;
      case PAT_RING_AIMED:
        angle = aimAngle + desc.angleOffset + (i + 0.5f) * 2 * PI / desc.mainCnt + layer * desc.angleInterval;
        speed = layerSpeed;
        break;
      }
      spawn_bullet(originX, originY, angle, speed, desc.spriteID,desc.hitboxRadius, desc.lifeTime, enemyType, enemyID,fromBoss, desc.isSplit);
    }
  }
}
// deferred split spawn request (used in update)
struct SplitSpawnRequest {
  float x, y;
  float tangentX, tangentY;
  int spriteID;
  float hitboxRadius;
  float lifeTime;
  int enemyType;
  int enemyID;
  bool fromBoss;
  int splitGen;
};

void EnemyBulletManager::update(float dt) {
  std::vector<SplitSpawnRequest> deferredSplits;

  for (int i = 0; i < POOL_SIZE; i++) {
    if (bullets[i].state == SLEEPING)
      continue;
    Bullet &b = bullets[i];
    if (b.state == FROZEN)
      continue;
    if (b.state == TO_PLAYER) {
      float dx = playerX_ - b.x;
      float dy = playerY_ - b.y;
      float dist = sqrtf(dx * dx + dy * dy);
      if (dist < 16.0f) {
        if (itemMgr_)
          itemMgr_->spawn_item(b.x, b.y, ITEM_SCORE_SMALL);
        b.state = SLEEPING;
        continue;
      }
      float speed = 480.0f;
      b.x += (dx / dist) * speed * dt;
      b.y += (dy / dist) * speed * dt;
      continue;
    }
    if (b.splitTimer > 0.0f) {
      b.splitTimer -= dt;
      if (b.splitTimer <= 0.0f) {
        // 最深5代
        if (b.splitCount >= 5) {
          b.splitTimer = 0.0f;
        } else {
        b.splitTimer = 1.5f;
        b.splitCount++;
        SplitSpawnRequest req;
        req.x = b.x;
        req.y = b.y;
        // 交替逆时针/顺时针切线方向
        if (b.splitCount % 2 == 1) {
          req.tangentX = -b.speedY;
          req.tangentY = b.speedX;
        } else {
          req.tangentX = b.speedY;
          req.tangentY = -b.speedX;
        }
        req.spriteID = b.color;
        req.hitboxRadius = b.hitboxRadius;
        req.lifeTime = b.lifeTime;
        req.enemyType = b.enemyType;
        req.enemyID = b.enemyID;
        req.fromBoss = b.fromBoss;
        req.splitGen = b.splitCount;
        deferredSplits.push_back(req);
        }
      }
    }
    if (b.acceleration != 0.0f || b.angularVelocity != 0.0f) {
      float curSpeed = sqrtf(b.speedX * b.speedX + b.speedY * b.speedY);
      float curAngle = atan2f(b.speedY, b.speedX);
      float newAngle = curAngle + b.angularVelocity * dt;
      float newSpeed = curSpeed + b.acceleration * dt;
      if (newSpeed < 0.0f)
        newSpeed = 0.0f;
      b.speedX = cosf(newAngle) * newSpeed;
      b.speedY = sinf(newAngle) * newSpeed;
    }
    b.x += b.speedX * dt;
    b.y += b.speedY * dt;
    b.lifeTime -= dt;
    bool outOfBounds = (b.x > 544.0f + 16.0f) || (b.y > 600.0f + 16.0f) || (b.x < -16.0f);
    if (b.y + 16.0f < -64.0f)
      outOfBounds = true;
    if (outOfBounds) {
      b.state = SLEEPING;
    }
  }
  for (size_t j = 0; j < deferredSplits.size(); j++) {
    SplitSpawnRequest &req = deferredSplits[j];
    float angle = atan2f(req.tangentY, req.tangentX);
    float speed = sqrtf(req.tangentX * req.tangentX + req.tangentY * req.tangentY);
    spawn_bullet(req.x, req.y, angle, speed, req.spriteID, req.hitboxRadius, req.lifeTime, req.enemyType, req.enemyID, req.fromBoss, true, req.splitGen);
  }
}
void EnemyBulletManager::despawn_all_for_spellcard() {
  for (int i = 0; i < POOL_SIZE; i++) {
    if (bullets[i].state == ALIVE) {
      bullets[i].state = SLEEPING;
    }
  }
}
void EnemyBulletManager::convert_all_to_p_items(ItemManager *itemMgr, bool isUsingBomb) {
  if (!itemMgr)
    return;
  int converted = 0;
  for (int i = 0; i < POOL_SIZE; i++) {
    if (bullets[i].state != ALIVE || (!bullets[i].fromBoss && !isUsingBomb))
      continue;
    if (bullets[i].state == ALIVE) {
      itemMgr->spawn_item(bullets[i].x, bullets[i].y, ITEM_POWER_SMALL);
      bullets[i].state = SLEEPING;
      converted++;
    }
  }
  //if (converted > 0) std::cout << "Converted " << converted << " bullets to P items" << std::endl;
}
void EnemyBulletManager::convert_all_to_score_items(ItemManager *itemMgr, bool isUsingBomb) {
  if (!itemMgr)
    return;
  int converted = 0;
  for (int i = 0; i < POOL_SIZE; i++) {
    if (bullets[i].state != ALIVE || (!bullets[i].fromBoss && !isUsingBomb))
      continue;
    if (bullets[i].state == ALIVE) {
      itemMgr->spawn_item(bullets[i].x, bullets[i].y, ITEM_SCORE_SMALL);
      bullets[i].state = SLEEPING;
      converted++;
    }
  }
  //if (converted > 0) std::cout << "Converted " << converted << " bullets to score items" << std::endl;
}
void EnemyBulletManager::render() {
  if (bulletSheet == NULL)
    bulletSheet = IMG_Load("res/stgenm/enemyhalf.png");
  if (bulletSheet == NULL)
    return;
  static SDL_Surface *etama2Sheet = NULL;
  static SDL_Rect whiteRect = {0, 57, 16, 16};
  static SDL_Rect pointRect = {96, 64, 16, 16};
  if (etama2Sheet == NULL)
    etama2Sheet = IMG_Load("res/etama/etama2.png");

  for (int i = 0; i < POOL_SIZE; i++) {
    int st = bullets[i].state;
    if (st != ALIVE && st != FROZEN && st != TO_PLAYER)
      continue;
    Bullet &b = bullets[i];

    if (st == FROZEN && etamaHalfSheet) {
      SDL_Rect dst = {(Sint16)(b.x - 8), (Sint16)(b.y - 8), 0, 0};
      SDL_BlitSurface(etamaHalfSheet, &whiteRect, screen, &dst);
    } else if (st == TO_PLAYER && etama2Sheet) {
      SDL_Rect dst = {(Sint16)(b.x - 8), (Sint16)(b.y - 8), 0, 0};
      SDL_BlitSurface(etama2Sheet, &pointRect, screen, &dst);
    } else {
      int idx = (b.enemyType >= 0 && b.enemyType < 4) ? b.enemyType : 0;
      SDL_Rect dst = {(Sint16)(b.x - 8), (Sint16)(b.y - 8), 0, 0};
      SDL_BlitSurface(bulletSheet, &bulletSrc[idx], screen, &dst);
    }
  }
}

void EnemyBulletManager::freeze_all_for_bomb(ItemManager *im) {
  itemMgr_ = im;
  bombActive_ = true;
  for (int i = 0; i < POOL_SIZE; i++) {
    if (bullets[i].state == ALIVE) {
      bullets[i].state = FROZEN;
      bullets[i].speedX = 0.0f;
      bullets[i].speedY = 0.0f;
    }
  }
}

void EnemyBulletManager::convert_frozen_to_player() {
  bombActive_ = false;
  for (int i = 0; i < POOL_SIZE; i++) {
    if (bullets[i].state == FROZEN) {
      bullets[i].state = TO_PLAYER;
    }
  }
}

void EnemyBulletManager::clear_all() {
  for (int i = 0; i < POOL_SIZE; i++) {
    bullets[i].state = SLEEPING;
  }
  nextBulletIndex = 0;
}

std::vector<int> EnemyBulletManager::get_active_bullet_indices() const {
  std::vector<int> indices;
  for (int i = 0; i < POOL_SIZE; i++) {
    if (bullets[i].state == ALIVE) {
      indices.push_back(i);
    }
  }
  return indices;
}
