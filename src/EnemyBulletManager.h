#pragma once
#include "UI.h"
#include "EnemyBulletState.h"
#include "BulletPatternType.h"
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#define PI 3.14159265
class ItemManager;
struct Bullet{
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
};
struct EnemyBulletPatternDesc{
    int patternType;
    int mainCnt; // 主数量 一圈/一排的子弹数量
    int subCnt;  // 层数 速度分几档
    float angleOffset; // 角度偏移：扇形偏移，圆形起始角度，随机角度上限
    float angleInterval; // 角度间隔：扇形和圆形的子弹间隔，随机角度下限
    float speed1; // 速度1：扇形和圆形的基础速度，随机速度下限
    float speed2; // 速度2：每层递减量，随机速度上限,决定出生速度
    int spriteID;
    float hitboxRadius;
    float lifeTime;
    int spawnEffect;// todo:生成特效ID
    int soundEffect;// todo:生成音效ID
    int reboundEffect; // todo:反弹特效ID
    bool fromBoss;
};
struct EmitterConfig{
    float emitInterval; // 发射间隔
    int burstCount; // 发射子弹数量
    float burstInterval; // 连续发射时每发之间的间隔
    float startDelay; // 开始延迟
    EnemyBulletPatternDesc patternDesc;
};
class EnemyBulletManager{
private:
    static const int POOL_SIZE = 640;
    Bullet bullets[POOL_SIZE];
    int nextBulletIndex;
public:
    EnemyBulletManager();
    ~EnemyBulletManager();
    void spawn_bullet(float x, float y, float angle, float speed, int spriteID, float hitboxRadius, float lifeTime, int spawnEffect, int soundEffect, int reboundEffect, int enemyType, int enemyID, bool fromBoss);
    void spawn_pattern(const EnemyBulletPatternDesc& desc, float originX, float originY, float playerX, float playerY, int enemyType, int enemyID, bool fromBoss);
    void update(float dt);
    void render();
    void clear_all();
    void despawn_all_for_spellcard();
    void convert_all_to_p_items(ItemManager* itemMgr);
};
