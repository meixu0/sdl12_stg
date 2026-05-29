#include "EnemyBulletManager.h"
static SDL_Surface* simpleBulletImage = NULL;
static float randf() {
    return (float)rand() / (float)RAND_MAX;
}

EnemyBulletManager::EnemyBulletManager(){
    nextBulletIndex = 0;
    for(int i=0; i<POOL_SIZE; i++){
        bullets[i].state = SLEEPING;
    }
}

EnemyBulletManager::~EnemyBulletManager(){
}

void EnemyBulletManager::spawn_bullet(float originX, float originY, float angle, float speed,
    int spriteID, float hitboxRadius, float lifeTime, int spawnEffect, int soundEffect, int reboundEffect) {
    // 从 nextBulletIndex 开始找空闲槽位
    for (int i = 0; i < POOL_SIZE; i++) {
        int idx = (nextBulletIndex + i) % POOL_SIZE;
        if (bullets[idx].state == SLEEPING) {
            Bullet& b = bullets[idx];
            b.x = originX;
            b.y = originY;
            b.speedX = cosf(angle) * speed;
            b.speedY = sinf(angle) * speed;
            b.state = ALIVE;
            b.lifeTime = lifeTime;
            b.hitboxRadius = hitboxRadius;
            b.color = spriteID;
            b.acceleration = 0.0f;
            b.angularVelocity = 0.0f;
            nextBulletIndex = (idx + 1) % POOL_SIZE;
            return;
        }
    }
}

void EnemyBulletManager::spawn_pattern(const EnemyBulletPatternDesc& desc, float originX, float originY, float playerX, float playerY){
    float aimAngle = atan2f(playerY - originY, playerX - originX);
    for(int layer=0; layer < desc.subCnt; layer++){
        float layerSpeed = desc.speed1;
        if(desc.subCnt > 1) layerSpeed = desc.speed1 - (desc.speed1 - desc.speed2) * layer / (desc.subCnt - 1);
        for(int i = 0; i < desc.mainCnt; i++){
            float angle, speed;
            switch(desc.patternType){
                case PAT_FAN_AIMED:
                    angle = desc.angleOffset;
                    angle += (i % 2 == 0 ? 1.0f : -1.0f) * (i / 2 + i % 2) * desc.angleInterval;
                    angle += aimAngle;
                    speed = layerSpeed;
                    break;
                case PAT_FAN:
                    angle = desc.angleOffset;
                    angle += (i % 2 == 0 ? 1.0f : -1.0f) * (i / 2 + i % 2) * desc.angleInterval;
                    speed = layerSpeed;
                    break;
                case PAT_CIRCLE_AIMED:
                    angle = aimAngle + desc.angleOffset + i * 2 * PI / desc.mainCnt + layer * desc.angleInterval;
                    speed = layerSpeed;
                    break;
                case PAT_CIRCLE:
                    angle = desc.angleOffset + i * 2*PI / desc.mainCnt + layer * desc.angleInterval;
                    speed = layerSpeed;
                    break;
                case PAT_RANDOM_ANGLE:
                    angle = desc.angleOffset + randf() * (desc.angleInterval - desc.angleOffset);
                    speed = layerSpeed;
                    break;
                case PAT_RANDOM_SPEED:
                    angle = desc.angleOffset + i * 2*PI / desc.mainCnt;
                    speed = desc.speed2 + randf() * (desc.speed2 - desc.speed1);
                    break;
                case PAT_RANDOM_BOTH:
                    angle = desc.angleOffset + randf() * (desc.angleInterval - desc.angleOffset);
                    speed = desc.speed2 + randf() * (desc.speed2 - desc.speed1);
                    break;
                case PAT_RING_AIMED:
                    angle = aimAngle + desc.angleOffset + (i + 0.5f) * 2*PI / desc.mainCnt + layer * desc.angleInterval;
                    speed = layerSpeed;
                    break;
            }
            spawn_bullet(originX, originY, angle, speed, desc.spriteID, desc.hitboxRadius, desc.lifeTime, desc.spawnEffect, desc.soundEffect, desc.reboundEffect);
        }
    }
}

void EnemyBulletManager::update(float dt){
    for (int i = 0; i < POOL_SIZE; i++) {
        if (bullets[i].state != ALIVE) continue;
        Bullet& b = bullets[i];

        if (b.acceleration != 0.0f || b.angularVelocity != 0.0f) {
            float curSpeed = sqrtf(b.speedX * b.speedX + b.speedY * b.speedY);
            float curAngle = atan2f(b.speedY, b.speedX);
            float newAngle = curAngle + b.angularVelocity * dt;
            float newSpeed = curSpeed + b.acceleration * dt;
            if (newSpeed < 0.0f) newSpeed = 0.0f;
            b.speedX = cosf(newAngle) * newSpeed;
            b.speedY = sinf(newAngle) * newSpeed;
        }

        b.x += b.speedX * dt;
        b.y += b.speedY * dt;
        b.lifeTime -= dt;

        // 子弹贴图 4x4，半个边长 = 2
        // 只检查下边界和左右边界：子弹可以向上飞出屏幕外不回收
        // 敌机经常在屏幕上方（y < 0）生成子弹，不能用上界直接杀弹
        bool outOfBounds = (b.x + 2.0f > 544.0f)   // 右端出界
                        || (b.y + 2.0f > 600.0f)   // 下端出界
                        || (b.x - 2.0f < 0.0f);    // 左端出界

        // 上端：只回收明显飞远的（防内存泄漏）
        if (b.y + 2.0f < -64.0f) outOfBounds = true;

        // 在游戏区域内 → 不根据 lifeTime 销毁，仅边界回收
        // lifeTime 留给符卡回收 (despawn_all_for_spellcard)
        if (outOfBounds) {
            b.state = SLEEPING;
        }
    }
}

void EnemyBulletManager::despawn_all_for_spellcard(){
    for (int i = 0; i < POOL_SIZE; i++) {
        if (bullets[i].state == ALIVE) {
            bullets[i].state = SLEEPING;
        }
    }
}

void EnemyBulletManager::render(){
    if(simpleBulletImage == NULL){
        simpleBulletImage = load_image("res/simple_bullet.png", 4.0, 4.0);
    }
    for (int i = 0; i < POOL_SIZE; i++) {
        if (bullets[i].state != ALIVE) continue;
        Bullet& b = bullets[i];
        apply_surface((int)(b.x - 2), (int)(b.y - 2), simpleBulletImage, screen);
    }
}

void EnemyBulletManager::clear_all(){
    for (int i = 0; i < POOL_SIZE; i++) {
        bullets[i].state = SLEEPING;
    }
    nextBulletIndex = 0;
}