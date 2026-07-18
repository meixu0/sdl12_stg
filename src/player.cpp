#include "player.h"
#include "LevelManager.h"
#include "ItemManager.h"
#include "PlayerBomb.h"
#include "EnemyBulletManager.h"
const float Player::PLAYER_HEIGHT = 48*1.5625f;
const float Player::PLAYER_WIDTH  = 32*1.5625f;
const int Player::PLAY_HEIGHT = 600;
const int Player::PLAY_WIDTH  = 544;
int Player::playerPowerData = 0;
int Player::currentPowerLevel = 0;
bool Player::bombInUse = false;

static const int FRAME_W   = 38;
static const int FRAME_H   = 56;
static const int SHEET_SZ  = 300;

SDL_Surface* Player::playerSheet       = NULL;
SDL_Surface* Player::playerSheetMirror = NULL;

SDL_Rect Player::middleSrcRects[3] = {
    {  0,  0, FRAME_W, FRAME_H},
    { 38,  0, FRAME_W, FRAME_H},
    { 76,  0, FRAME_W, FRAME_H},
};
SDL_Rect Player::sideSrcRects[7] = {
    {  0, 56, FRAME_W, FRAME_H},
    { 38, 56, FRAME_W, FRAME_H},
    { 76, 56, FRAME_W, FRAME_H},
    {114, 56, FRAME_W, FRAME_H},
    {152, 56, FRAME_W, FRAME_H},
    {190, 56, FRAME_W, FRAME_H},
    {228, 56, FRAME_W, FRAME_H},
};

Player::Player(){
    x = 272;
    y = 600 - (int)PLAYER_WIDTH - 10;
    xVel = 0; yVel = 0;

    const char* sheetFile;
    switch(playerType){
        case 0: sheetFile = "res/player00/player00.png"; break;
        case 1: sheetFile = "res/player00/player01.png"; break;
        case 2: sheetFile = "res/player00/player02.png"; break;
        case 3: sheetFile = "res/player00/player03.png"; break;
        default: sheetFile = "res/player00/player00.png"; break;
    }

    if(playerSheet != NULL)       SDL_FreeSurface(playerSheet);
    if(playerSheetMirror != NULL) SDL_FreeSurface(playerSheetMirror);
    playerSheet       = load_image(sheetFile, SHEET_SZ, SHEET_SZ);
    playerSheetMirror = mirror_surface(playerSheet);

    isShooting = false;
    simpleShootCoolDown = 0;
    animTimer = 0.0f;
    targetEnemyX = 272; targetEnemyY = 0;
    hasTarget = false;
    optionStreamAngle_ = -1.5707963f;
    itemMgr = NULL;
    for(int i = 0; i < MAX_BULLETS_PER_RANK; i++)
        bulletFireTimers[i] = 0;
}

void Player::init_player_bullet_pool(PlayerBulletPool *poolPtr){
    playerBulletPool_ = poolPtr;
}

void Player::handle_input(SDL_Event &e){
    if(e.type == SDL_KEYDOWN){
        switch(e.key.keysym.sym){
            case SDLK_UP:    yVel -= PLAYER_HEIGHT / 4; break;
            case SDLK_DOWN:  yVel += PLAYER_HEIGHT / 4; break;
            case SDLK_LEFT:  xVel -= PLAYER_WIDTH / 4;  break;
            case SDLK_RIGHT: xVel += PLAYER_WIDTH / 4;  break;
            case SDLK_z:     isShooting = true;          break;
            case SDLK_x:
                if(!playerBomb_ && ItemManager::get_bombs() > 0){
                    ItemManager::use_bomb();
                    playerBomb_ = new PlayerBomb();
                    playerBomb_->trigger(playerType, playerType, (float)x, (float)y, levelMgr_, itemMgr);
                    isInvincible_ = true;
                    invTimer_ = playerBomb_->invincibility();
                }
                break;
        }
    }else if(e.type == SDL_KEYUP){
        switch(e.key.keysym.sym){
            case SDLK_UP:    yVel += PLAYER_HEIGHT / 4; break;
            case SDLK_DOWN:  yVel -= PLAYER_HEIGHT / 4; break;
            case SDLK_LEFT:  xVel += PLAYER_WIDTH / 4;  break;
            case SDLK_RIGHT: xVel -= PLAYER_WIDTH / 4;  break;
            case SDLK_z:     isShooting = false;         break;
        }
    }
}

void Player::player_move(){
    x += xVel;
    if((x < 0) || (x + PLAYER_HEIGHT > PLAY_WIDTH)) x -= xVel;
    y += yVel;
    if((y < 0) || (y + PLAYER_WIDTH > PLAY_HEIGHT)) y -= yVel;
    animTimer += 1.0f / 60.0f;
}

void Player::check_collision_with_enemy_bullets(EnemyBulletManager* ebm){
    if(isInvincible_ || godMode) return;
    float px = (float)(x + PLAYER_WIDTH / 2);
    float py = (float)(y + PLAYER_HEIGHT / 2);
    if(ebm){
        for(int i = 0; i < 640; i++){
            Bullet* b = ebm->get_bullet(i);
            if(!b || b->state != ALIVE) continue;
            float dx = b->x - px;
            float dy = b->y - py;
            float r = b->hitboxRadius > 0 ? b->hitboxRadius : 8.0f;
            if((dx*dx + dy*dy) < (r + 5.0f) * (r + 5.0f)){
                hit();
                return;
            }
        }
    }
    for(int ei = 0; ei < Enemy::onScreenCount; ei++){
        Enemy* e = Enemy::onScreenList[ei];
        if(!e || !e->is_active()) continue;
        float ex = e->get_x() + e->get_hitbox_w() * 0.5f;
        float ey = e->get_y() + e->get_hitbox_h() * 0.5f;
        float er = (e->get_hitbox_w() + e->get_hitbox_h()) * 0.25f;
        float dx = ex - px;
        float dy = ey - py;
        if((dx*dx + dy*dy) < (er + 5.0f) * (er + 5.0f)){
            hit();
            return;
        }
    }
}

void Player::update_bomb(float dt){
	if(playerBomb_){
		playerBomb_->update(dt);
		if(playerBomb_->isExpired()){
			playerBomb_->on_expire();
			delete playerBomb_;
			playerBomb_ = NULL;
		}
	}
}

void Player::render_bomb_shake(){
	if(playerBomb_) playerBomb_->render_shake();
}

void Player::render_bomb_portrait(){
	if(playerBomb_) playerBomb_->render_portrait();
}

void Player::update_simple_shoot(){
    PlayerPowerRank* rank = &g_PlayerPowerRanks[0];
    for(int i1 = 0; i1 < 6; i1++)
        if(playerPowerData >= g_PlayerPowerRanks[i1].powerThreshold)
            rank = &g_PlayerPowerRanks[i1];

    if(!isShooting)
        for(int i2 = 0; i2 < MAX_BULLETS_PER_RANK; i2++)
            bulletFireTimers[i2] = 0;

    float spawnX = (float)(x + PLAYER_WIDTH / 2);
    float spawnY = (float)y;

    for(int i3 = 0; i3 < rank->numBullets; i3++){
        update_player_bullet_collision_detection();
        if(!isShooting) continue;
        if(bulletFireTimers[i3] > 0){ bulletFireTimers[i3]--; continue; }

        PlayerBulletConfig cfg = rank->bullets[i3];
        cfg.x = spawnX + cfg.xOffset;
        cfg.y = spawnY + cfg.yOffset;

        if(cfg.bulletType == 1){
            if(hasTarget){
                float dx = targetEnemyX - spawnX;
                float dy = targetEnemyY - spawnY;
                float desiredAngle = atan2f(dy, dx);
                float diff = desiredAngle - optionStreamAngle_;
                while(diff >  3.14159265f) diff -= 6.2831853f;
                while(diff < -3.14159265f) diff += 6.2831853f;
                optionStreamAngle_ += diff * 0.08f;
                cfg.targetX = targetEnemyX;
                cfg.targetY = targetEnemyY;
            }else{
                float diff = -1.5707963f - optionStreamAngle_;
                while(diff >  3.14159265f) diff -= 6.2831853f;
                while(diff < -3.14159265f) diff += 6.2831853f;
                optionStreamAngle_ += diff * 0.05f;
            }
            cfg.angle = optionStreamAngle_;
        }
        playerBulletPool_->create_bullet(cfg);
        bulletFireTimers[i3] = cfg.fireInterval;
    }
}

void Player::set_homing_target(float tx, float ty){
    targetEnemyX = tx; targetEnemyY = ty;
    hasTarget = (ty > 0);
}

void Player::update_player_bullet_collision_detection(){
    if(playerBulletPool_ == NULL) return;
    PlayerBullet* bullets = playerBulletPool_->bullets();
    int poolSize = playerBulletPool_->pool_size();

    for(int ei = 0; ei < Enemy::onScreenCount; ei++){
        Enemy* e = Enemy::onScreenList[ei];
        if(!e) continue;
        for(int bi = 0; bi < poolSize; bi++){
            PlayerBullet& b = bullets[bi];
            if(!b.inUse()) continue;
            if(e->check_bullet_hit(b.get_x(), b.get_y(), b.get_hw(), b.get_hh())){
                int dmg = b.get_damage();
                b.on_hit();
                int remainHp = e->take_damage(dmg);
                if(remainHp <= 0 && !e->is_spellcard_boss()){
                    if(itemMgr != NULL){
                        float ex = e->get_x() + e->get_hitbox_w() * 0.5f;
                        float ey = e->get_y() + e->get_hitbox_h() * 0.5f;
                        itemMgr->spawn_item(ex, ey, ITEM_POWER_SMALL, 2);
                        if(e->bulletManager != NULL)
                            e->bulletManager->convert_all_to_p_items(itemMgr);
                    }
                    e->deactivate();
                }
                break;
            }
        }
    }
}

bool Player::check_hit(float bx, float by, float br){
    float px = (float)(x + PLAYER_WIDTH / 2);
    float py = (float)(y + PLAYER_HEIGHT / 2);
    float dx = bx - px;
    float dy = by - py;
    return (dx*dx + dy*dy) < (br + 5.0f) * (br + 5.0f);
}

void Player::hit(){
    if(isInvincible_ || godMode) return;
    ItemManager::use_life();
    Player::set_power(0);
    isInvincible_ = true;
    invTimer_ = 180;
    EnemyBulletManager* ebm = levelMgr_ ? levelMgr_->get_bullet_manager() : NULL;
    if(ebm){
        ebm->freeze_all_for_bomb(itemMgr);
        ebm->convert_frozen_to_player();
    }
}

void Player::update_invincible(){
    if(invTimer_ > 0){
        invTimer_--;
        if(invTimer_ <= 0) isInvincible_ = false;
    }
}

void Player::show(){
    if(playerSheet == NULL) return;
    if(isInvincible_ && (invTimer_ / 4) % 2) return; /* иак╦ */

    int   frameCount;
    SDL_Rect* srcRects;
    SDL_Surface* sheet;
    int   destX = x;
    if(xVel > 0){
        frameCount = 7; srcRects = sideSrcRects; sheet = playerSheet;
    }else if(xVel < 0){
        frameCount = 7; srcRects = sideSrcRects; sheet = playerSheetMirror;
        destX = x - FRAME_W;
    }else{
        frameCount = 3; srcRects = middleSrcRects; sheet = playerSheet;
    }
    int tick = ((int)(animTimer * 4.0f)) % (frameCount * 2 - 2);
    int idx = (tick > frameCount - 1) ? (frameCount * 2 - 2 - tick) : tick;
    SDL_Rect dest = {(Sint16)destX, (Sint16)y, 0, 0};
    SDL_BlitSurface(sheet, &srcRects[idx], screen, &dest);
}

PlayerPosition Player::get_player_position(){
    PlayerPosition pos;
    pos.x = (float)(x + PLAYER_WIDTH / 2);
    pos.y = (float)(y + PLAYER_HEIGHT / 2);
    return pos;
}
