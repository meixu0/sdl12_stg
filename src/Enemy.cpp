#include "Enemy.h"
#include "EnemyScManager.h"
#include "player.h"
#include <iostream>
#include <algorithm>

Enemy* Enemy::onScreenList[256] = {NULL};
Mix_Chunk* Enemy::tan00 = NULL;
int Enemy::onScreenCount = 0;

namespace {
    inline float easeLinear(float t) { return 1.0f - t; }
    inline float easeDecelerate(float t) { return 1.0f - t * t; }
    inline float easeDecelerateFast(float t) {
        float t2 = t * t;
        return 1.0f - t2 * t2;
    }
    inline float easeAccelerate(float t) {
        float u = 1.0f - t;
        return u * u;
    }
}
Enemy::Enemy() : x(0.0), y(0.0), startX(0.0), startY(0.0), playerX(0.0), playerY(0.0), hp(0), isActive(false),
    speedX(0.0), speedY(0.0), timeAlive(0.0), durationTime(0.0), targetX(0.0), targetY(0.0), type(0),
    bezierP1x(0), bezierP1y(0), bezierP2x(0), bezierP2y(0), bezierEndX(0), bezierEndY(0),
    bezierDuration(0), bezierTime(0), stateStartX(0), stateStartY(0),
    moveAngle(0), angularVelocity(0), accel(0), minPlayerDist(80.0f),
    moveDuration(0), moveElapsed(0), moveEasing(0),
    spriteRow(0), spriteAnimTimer(0.0f),
    isDead(false),
    axisSpeedX(0), axisSpeedY(0), enemyType(0), enemyID(0), isMidboss(false), isEntering(false), entryTargetY(100.0f), playerPtr(NULL),
    isSpellcardBoss(false), scManager(nullptr), hasMoveBounds(false), moveMinX(8), moveMaxX(536), moveMinY(0), moveMaxY(592),
    currentPhase_(-1), phaseTimer_(0.0f), isPhasedBoss_(false), bulletManager(NULL){
        if (tan00 == NULL) {
            tan00 = Mix_LoadWAV("res/sound/se_tan00.wav");
            Mix_VolumeChunk(tan00, 25);
        }
}
Enemy::~Enemy(){
    deactivate();
}
SDL_Surface* Enemy::get_zako_sprite(int col) {
    return zakoSprites[enemyType][col];
}
SDL_Surface* Enemy::get_boss_sprite(int col) {
    return bossSprites[enemyType][col];
}
void Enemy::init(EnemyConfig config, float x_, float y_){
    x = x_;
    y = y_;
    startX = x_;
    startY = y_;
    hp = config.hp;
    movePattern = config.movePattern;
    emergeTime = config.emergeTime;
    durationTime = config.durationTime;
    enemyType = config.enemyType;
    enemyID = config.enemyID;
    isMidboss = config.isMidboss;
    if(isMidboss)   durationTime = 99.0;//将道中boss默认设为99的生存时间
    hitboxHeight = config.hitboxHeight;
    hitboxWidth = config.hitboxWidth;
    speedX = config.speedX;
    speedY = config.speedY;
    emergeSpeedY = 5.0;
    timeAlive = 0.0;
    isActive = false;
    isDead = false;
    targetX = config.targetX;
    targetY = config.targetY;
    vertAmplitude = config.vertAmplitude;
    vertPeriod = config.vertPeriod;
    horizAmplitude = config.horizAmplitude;
    horizPeriod = config.horizPeriod;
    if (config.halfLife > 0.0f)
        homingRate = 0.693f / config.halfLife;
    else
        homingRate = 0.0f;
    bezierP1x = config.bezierP1x;
    bezierP1y = config.bezierP1y;
    bezierP2x = config.bezierP2x;
    bezierP2y = config.bezierP2y;
    bezierEndX = config.bezierEndX;
    bezierEndY = config.bezierEndY;
    bezierDuration = config.bezierDuration;
    bezierTime = 0.0f;
    stateStartX = x_;
    stateStartY = y_;
    moveAngle = config.moveAngle;
    angularVelocity = config.angularVelocity;
    accel = config.acceleration;
    minPlayerDist = (config.minPlayerDist > 0.0f) ? config.minPlayerDist : 80.0f;
    int seed = ((int)(config.emergeTime * 100.0f)) % 100;
    int zakoRows[] = {0, 1, 2, 3, 8, 9, 10, 11};
    spriteRow = zakoRows[seed % 8];
    spriteAnimTimer = 0.0f;
    emitterRuntime.resize(emitterConfig.size());
    for (size_t i = 0; i < emitterRuntime.size(); i++) {
        emitterRuntime[i].timer = 0.0f;
        emitterRuntime[i].burstRemaining = 0;
        emitterRuntime[i].cycleCount = 0;
    }
}
bool Enemy::is_active(){
    return isActive;
}
float Enemy::clamp(float value, float min_, float max_){
    if(value < min_)  return min_;
    else if(value > max_) return max_;
    else return value;
}

float Enemy::cubic_bezier(float t, float p0, float p1, float p2, float p3){
    float u = 1.0f - t;
    return u*u*u * p0 + 3*u*u*t * p1 + 3*u*t*t * p2 + t*t*t * p3;
}
void Enemy::update_player_info(float px, float py, size_t frameCounter__){
    frameCounter_ = frameCounter__;
    playerX = px;
    playerY = py;
}

void Enemy::compute_axis_speed(){
    switch (movePattern){
        case LINER:
            axisSpeedX = 0.0f;
            axisSpeedY = speedY;
            break;

        case SINWAVE:
            axisSpeedX = speedX + cosf(timeAlive * 2.0f * PI / horizPeriod) * horizAmplitude;
            axisSpeedY = sinf(timeAlive * 2.0f * PI / vertPeriod) * vertAmplitude;
            break;

        case HOMING: {
            float dx = playerX - x;
            float dy = playerY - y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist > 0.1f) {
                float targetAngle = atan2f(dy, dx);

                if (dist < minPlayerDist * 1.2f) {
                    float nx = dx / dist;
                    float ny = dy / dist;
                    float aimX = playerX - nx * minPlayerDist;
                    float aimY = playerY - ny * minPlayerDist;
                    targetAngle = atan2f(aimY - y, aimX - x);
                }
                if (homingRate > 0.0f) {
                    float diff = targetAngle - moveAngle;
                    while (diff > PI)  diff -= 2.0f * PI;
                    while (diff < -PI) diff += 2.0f * PI;
                    moveAngle += diff * (1.0f - expf(-homingRate * (1.0f / 60.0f)));
                } else if (angularVelocity > 0.0f && fabsf(targetAngle - moveAngle) > 0.001f) {
                    float diff = targetAngle - moveAngle;
                    while (diff > PI)  diff -= 2.0f * PI;
                    while (diff < -PI) diff += 2.0f * PI;
                    float maxTurn = angularVelocity * (1.0f / 60.0f);
                    diff = std::max(-maxTurn, std::min(maxTurn, diff));
                    moveAngle += diff;
                } else {
                    moveAngle = targetAngle;
                }
                float curSpeed = speedX;
                if (dist < minPlayerDist * 0.4f) {
                    curSpeed = 0.0f;
                } else if (dist < minPlayerDist) {
                    float t = (dist - minPlayerDist * 0.4f) / (minPlayerDist * 0.6f);
                    curSpeed = speedX * easeDecelerate(1.0f - t);
                }

                curSpeed += accel * (1.0f / 60.0f);
                if (curSpeed < 0.0f) curSpeed = 0.0f;

                axisSpeedX = cosf(moveAngle) * curSpeed;
                axisSpeedY = sinf(moveAngle) * curSpeed;
            } else {
                axisSpeedX = 0.0f;
                axisSpeedY = 0.0f;
            }
            break;
        }

        case BEZIER:
            bezierTime += (1.0f / 60.0f);
            if (bezierDuration > 0.0f) {
                float t = clamp(bezierTime / bezierDuration, 0.0f, 1.0f);
                float nextX = cubic_bezier(t, stateStartX, bezierP1x, bezierP2x, bezierEndX);
                float nextY = cubic_bezier(t, stateStartY, bezierP1y, bezierP2y, bezierEndY);
                axisSpeedX = (nextX - x) * 60.0f; 
                axisSpeedY = (nextY - y) * 60.0f;
            }
            break;

        case STOPANDGO:
            {
                float dx = targetX - x;
                float dy = targetY - y;
                float dist = sqrtf(dx*dx + dy*dy);
                float speed = speedY > 0.0f ? speedY : 120.0f;
                if (dist > 1.5f) {
                    axisSpeedX = (dx / dist) * speed;
                    axisSpeedY = (dy / dist) * speed;
                } else {
                    x = targetX;
                    y = targetY;
                    axisSpeedX = 0.0f;
                    axisSpeedY = 0.0f;
                }
            }
            break;

        case INTERCEPTION: {
            float predictTime = 0.5f;
            float predX = playerX;
            float predY = playerY;
            float dx = predX - x;
            float dy = predY - y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist > 0.1f) {
                float targetAngle = atan2f(dy, dx);
                if (dist < minPlayerDist * 1.2f) {
                    float nx = dx / dist;
                    float ny = dy / dist;
                    float aimX = predX - nx * minPlayerDist;
                    float aimY = predY - ny * minPlayerDist;
                    targetAngle = atan2f(aimY - y, aimX - x);
                }
                if (homingRate > 0.0f) {
                    float diff = targetAngle - moveAngle;
                    while (diff > PI)  diff -= 2.0f * PI;
                    while (diff < -PI) diff += 2.0f * PI;
                    moveAngle += diff * (1.0f - expf(-homingRate * (1.0f / 60.0f)));
                } else if (angularVelocity > 0.0f) {
                    float diff = targetAngle - moveAngle;
                    while (diff > PI)  diff -= 2.0f * PI;
                    while (diff < -PI) diff += 2.0f * PI;
                    float maxTurn = angularVelocity * (1.0f / 60.0f);
                    diff = std::max(-maxTurn, std::min(maxTurn, diff));
                    moveAngle += diff;
                } else {
                    moveAngle = targetAngle;
                }

                float curSpeed = speedX;
                if (dist < minPlayerDist * 0.4f) {
                    curSpeed = 0.0f;
                } else if (dist < minPlayerDist) {
                    float t = (dist - minPlayerDist * 0.4f) / (minPlayerDist * 0.6f);
                    curSpeed = speedX * easeDecelerate(1.0f - t);
                }

                curSpeed += accel * (1.0f / 60.0f);
                if (curSpeed < 0.0f) curSpeed = 0.0f;

                axisSpeedX = cosf(moveAngle) * curSpeed;
                axisSpeedY = sinf(moveAngle) * curSpeed;
            } else {
                axisSpeedX = 0.0f;
                axisSpeedY = 0.0f;
            }
            break;
        }

        case POSITION_INTERP:
            {
                if (moveDuration <= 0.0f) { axisSpeedX = 0; axisSpeedY = 0; break; }
                moveElapsed += 1.0f / 60.0f;
                float t = std::min(moveElapsed / moveDuration, 1.0f);
                float ease_t;
                if (moveEasing == 4) {
                    float u = 1.0f - t;
                    ease_t = 1.0f - u * u;
                } else {
                    ease_t = t;
                }
                float curX = stateStartX + (targetX - stateStartX) * ease_t;
                float curY = stateStartY + (targetY - stateStartY) * ease_t;
                x = curX;
                y = curY;
                axisSpeedX = 0.0f;
                axisSpeedY = 0.0f;
            }
            break;

        default:
            axisSpeedX = 0.0f;
            axisSpeedY = 0.0f;
            break;
    }
}

void Enemy::force_retreat(){
    axisSpeedY = -axisSpeedY - 50;
}

void Enemy::enemy_move(float dt){
    bool wasActive = isActive;
    timeAlive += dt;
    if (isDead) { isActive = false; }
    else if(x + 40 < 0 || x >= 544 - 40 || y + 40 < 0 || y >= 800 - 40 ||
       timeAlive < emergeTime || timeAlive >= emergeTime + durationTime) {
        isActive = false;
    } else {
        isActive = true;
    }

    if (!wasActive && isActive) {
        if (onScreenCount < 256) onScreenList[onScreenCount++] = this;
        if (enemyType >= BOSS_RUMIA) boss_entry();
    } else if (wasActive && !isActive) {
        for (int i = 0; i < onScreenCount; i++) {
            if (onScreenList[i] == this) {
                onScreenList[i] = onScreenList[--onScreenCount];
                break;
            }
        }
    }
    if(!isActive) return;

    // boss entry phase: move down from above screen to target Y
    if (isEntering) {
        axisSpeedX = 0.0f;
        axisSpeedY = speedY;
        x += axisSpeedX * dt;
        y += axisSpeedY * dt;
        if (y >= entryTargetY) {
            y = entryTargetY;
            isEntering = false;
            // Reset stop_and_go target to current position so enemy stays put after entry
            targetX = x;
            targetY = y;
        }
        spriteAnimTimer += dt;
        return;
    }

    spriteAnimTimer += dt;

    if(playerPtr != NULL){
        PlayerPosition playerPos = playerPtr->get_player_position();
        playerX = playerPos.x;
        playerY = playerPos.y;
    }

    compute_axis_speed();
    x += axisSpeedX * dt;
    y += axisSpeedY * dt;

    // Skip all boundary clipping during active position_interp (ECL entry phase)
    if (movePattern == POSITION_INTERP && moveElapsed < moveDuration) {
        // allow offscreen entry — no clipping
    } else {
        // Apply moveBounds (ECL move_bounds_set) if set; otherwise default screen bounds
        if (hasMoveBounds) {
            if (x < moveMinX) x = moveMinX;
            if (x > moveMaxX) x = moveMaxX;
            if (y < moveMinY) y = moveMinY;
            if (y > moveMaxY) y = moveMaxY;
        } else {
            if (x < 8.0f)        x = 8.0f;
            if (x > 536.0f)      x = 536.0f;
            if (y < 0.0f)        { /* 允许从上方出场 */ }
            if (y > 592.0f)      y = 592.0f;
        }
    }
}

void Enemy::enemy_attack(float dt){
    if (!isActive || bulletManager == NULL) return;

    // ── Phase sequencer: advance timer, transition phases ──
    if (isPhasedBoss_ && bossPhases_.size() > 0 && currentPhase_ >= 0) {
        phaseTimer_ += dt;
        if (phaseTimer_ >= bossPhases_[currentPhase_].duration) {
            advance_phase();
        }
    }

    if (emitterConfig.size() != emitterRuntime.size()) return;

    for (size_t i = 0; i < emitterConfig.size(); i++) {
        EmitterConfig& ec = emitterConfig[i];
        EmitterRuntime& rt = emitterRuntime[i];

        rt.timer += dt;
        if (rt.timer < ec.startDelay) continue;

        float activeTime = rt.timer - ec.startDelay;
        int  burstMax   = (ec.burstCount > 0) ? ec.burstCount : 1;

        if (burstMax == 1) {
            if (activeTime >= ec.emitInterval) {
                bulletManager->spawn_pattern(ec.patternDesc, x, y, playerX, playerY, enemyType, enemyID, enemyType >= BOSS_RUMIA);
                Mix_PlayChannel(0, tan00, 0);
                rt.timer = ec.startDelay;
            }
            continue;
        }

        if (rt.burstRemaining <= 0) {
            if (activeTime >= ec.emitInterval) {
                rt.burstRemaining = burstMax;
                rt.timer = ec.startDelay;
                activeTime = 0.0f;
            } else {
                continue;
            }
        }

        if (activeTime >= ec.burstInterval) {
            bulletManager->spawn_pattern(ec.patternDesc, x, y, playerX, playerY, enemyType, enemyID, enemyType >= BOSS_RUMIA);
            Mix_PlayChannel(0, tan00, 0);
            rt.burstRemaining--;
            rt.timer = ec.startDelay;
            if (rt.burstRemaining > 0) {
                rt.timer = ec.startDelay;
            }
        }
    }
}

void Enemy::enemy_show(){
    if(!isActive) return;
    int colBase;
    float threshold = 5.0f;
    if (axisSpeedX < -threshold)          colBase = 12;
    else if (axisSpeedX > threshold)      colBase = 4;
    else if (axisSpeedY > threshold)      colBase = 8;
    else                                  colBase = 0;

    int tick = ((int)(spriteAnimTimer * 4.0f)) % 8;
    int frameIndex = (tick > 3) ? (7 - tick) : tick;

    int col = colBase + frameIndex;
    SDL_Surface* sprite = (enemyType >= BOSS_RUMIA)
        ? get_boss_sprite(col)
        : get_zako_sprite(col);
    if (sprite) {
        apply_surface((int)x, (int)y, sprite, screen);
    }
}

void Enemy::deactivate(){
    isDead = true;
    if (isActive) {
        isActive = false;
        for (int i = 0; i < onScreenCount; i++) {
            if (onScreenList[i] == this) {
                onScreenList[i] = onScreenList[--onScreenCount];
                break;
            }
        }
    }
}

bool Enemy::check_bullet_hit(float bx, float by, float bhw, float bhh){
    float eLeft = x;
    float eRight = x + hitboxWidth;
    float eTop = y;
    float eBottom = y + hitboxHeight;
    float bLeft = bx - bhw;
    float bRight = bx + bhw;
    float bTop = by - bhh;
    float bBottom = by + bhh;
    return !(bRight < eLeft || bLeft > eRight || bBottom < eTop || bTop > eBottom);
}

int Enemy::take_damage(int dmg){
    if (scManager && scManager->is_active()) {
        if (dmg > 7) dmg = dmg / 7;
        else dmg = 1;
    }
    hp -= dmg;
    return hp;
}

void Enemy::link_spellcard_manager(EnemyScManager* mgr) {
    scManager = mgr;
    isSpellcardBoss = true;
}

void Enemy::set_spellcard_hp(int newHp) {
    hp = newHp;
    if (hp < 0) hp = 0;
}

void Enemy::start_position_interp(float dur, int easing, float tx, float ty) {
    stateStartX = x;
    stateStartY = y;
    targetX = tx;
    targetY = ty;
    movePattern = POSITION_INTERP;
    moveDuration = dur;
    moveElapsed = 0.0f;
    moveEasing = easing;
}

void Enemy::set_move_bounds(float minX, float minY, float maxX, float maxY) {
    moveMinX = minX;
    moveMaxX = maxX;
    moveMinY = minY;
    moveMaxY = maxY;
    hasMoveBounds = true;
}

void Enemy::set_phases(const std::vector<BossPhaseDef>& phases) {
    bossPhases_ = phases;
}

void Enemy::start_phases() {
    if (bossPhases_.empty()) return;
    currentPhase_ = 0;
    phaseTimer_ = 0.0f;
    isPhasedBoss_ = true;
    BossPhaseDef phase = bossPhases_[0];
    emitterConfig = phase.patterns;
    emitterRuntime.resize(emitterConfig.size());
    std::vector<EmitterRuntime>::iterator it = emitterRuntime.begin();
    for (it = emitterRuntime.begin(); it != emitterRuntime.end(); ++it) {
        it->timer = 0.0f; it->burstRemaining = 0; it->cycleCount = 0;
    }
}

void Enemy::advance_phase() {
    if (!isPhasedBoss_ || bossPhases_.empty()) return;
    currentPhase_ = (currentPhase_ + 1) % bossPhases_.size();
    phaseTimer_ = 0.0f;

    BossPhaseDef phase = bossPhases_[currentPhase_];

    if (phase.moveDuration > 0.0f) {
        start_position_interp(phase.moveDuration, phase.moveEasing, phase.moveToX, phase.moveToY);
    }

    emitterConfig = phase.patterns;
    emitterRuntime.resize(emitterConfig.size());
    std::vector<EmitterRuntime>::iterator it = emitterRuntime.begin();
    for (it = emitterRuntime.begin(); it != emitterRuntime.end(); ++it) {
        it->timer = 0.0f; it->burstRemaining = 0; it->cycleCount = 0;
    }
}

void Enemy::boss_entry() {
    if (enemyType < BOSS_RUMIA || isMidboss) return;
    // position_interp handles its own entry movement — skip boss_entry
    if (movePattern == POSITION_INTERP) return;
    isEntering = true;
    entryTargetY = 100.0f;

}