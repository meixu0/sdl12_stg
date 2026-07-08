#include "PlayerBomb.h"
#include "LevelManager.h"
#include "ItemManager.h"
#include "GameBackground.h"
#include "EnemyBulletManager.h"
#include <cstdlib>
static SDL_Rect portraitRects[4] = {
    {  0, 0, 128, 512},
    {128, 0, 128, 512},
    {256, 0, 128, 512},
    {384, 0, 128, 512},
};
static SDL_Rect whiteEnemyBullet = {0, 64, 16, 16};
static SDL_Surface* etamaHalfSheet = NULL;
PlayerBomb::PlayerBomb(): type_(0), duration_(0), invincibility_(0),
    timer_(0), invTimer_(0), active_(false),
    shakeX_(0), shakeY_(0),
    levelMgr_(NULL), itemMgr_(NULL) {}

PlayerBomb::~PlayerBomb() {}

void PlayerBomb::trigger(int type, int playerType, float x, float y, LevelManager* lm, ItemManager* im) {
    type_   = type;
    playerType_ = playerType;
    active_ = true;
    levelMgr_ = lm;
    itemMgr_  = im;
    (void)x; (void)y;

    const BombParams& p = bombParams[type];
    duration_      = p.duration;
    invincibility_ = p.invincibility;
    timer_   = duration_;
    invTimer_ = invincibility_;
    if(etamaHalfSheet == NULL)
        etamaHalfSheet = IMG_Load("res/etama/etamahalf.png");
    convert_bullets();
    clear_zako();
}

void PlayerBomb::update(float dt) {
    if (!active_) return;
    (void)dt;
    timer_--;
    if (invTimer_ > 0) invTimer_--;
    float progress = (float)timer_ / (float)duration_;
    int intensity = (int)(bombParams[type_].shakeIntensity * progress);
    shakeX_ = (rand() % (intensity * 2 + 1)) - intensity;
    shakeY_ = (rand() % (intensity * 2 + 1)) - intensity;
}

void PlayerBomb::render_portrait() {
    if (!active_) return;
    switch (type_) {
        case BOMB_REIMU_A:  reimuA_bomb_animation(NULL);  break;
        case BOMB_REIMU_B:  reimuB_bomb_animation(NULL);  break;
        case BOMB_MARISA_A: marisaA_bomb_animation(NULL); break;
        case BOMB_MARISA_B: marisaB_bomb_animation(NULL); break;
    }
}

void PlayerBomb::render_shake() {
    if (!active_) return;

    static SDL_Surface* shakeBuf = NULL;
    if(shakeBuf == NULL)
        shakeBuf = SDL_CreateRGBSurface(SDL_SWSURFACE,
            screen->w, screen->h,
            screen->format->BitsPerPixel,
            screen->format->Rmask, screen->format->Gmask,
            screen->format->Bmask, screen->format->Amask);
    if(shakeBuf == NULL) return;

    SDL_BlitSurface(screen, NULL, shakeBuf, NULL);
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Rect dst = {(Sint16)shakeX_, (Sint16)shakeY_, 0, 0};
    SDL_BlitSurface(shakeBuf, NULL, screen, &dst);
}

bool PlayerBomb::isExpired() const {
    return active_ && timer_ <= 0;
}

void PlayerBomb::on_expire() {
    active_ = false;
    EnemyBulletManager* bulletMgr = levelMgr_ ? levelMgr_->get_bullet_manager() : NULL;
    if (bulletMgr) bulletMgr->convert_frozen_to_player();
}

void PlayerBomb::convert_bullets() {
    EnemyBulletManager* bulletMgr = levelMgr_ ? levelMgr_->get_bullet_manager() : NULL;
    if (!bulletMgr) return;
    bulletMgr->freeze_all_for_bomb(itemMgr_);
}

void PlayerBomb::clear_zako() {
    if (!levelMgr_) return;
    int count = levelMgr_->get_enemy_count();
    for (int i = 0; i < count; i++) {
        Enemy* e = levelMgr_->get_enemy(i);
        if (!e || !e->is_active()) continue;
        if (e->get_enemy_type() < BOSS_RUMIA)
            e->deactivate();
    }
}
void PlayerBomb::draw_portrait(const char* sheetFile) {
    static SDL_Surface* reimuSheet  = NULL;
    static SDL_Surface* marisaSheet = NULL;
    static SDL_Surface* sakuyaSheet = NULL;
    SDL_Surface* sheet;
    if(playerType_ <= 1) {
        if(!reimuSheet) reimuSheet = IMG_Load(sheetFile);
        sheet = reimuSheet;
    } else if(playerType_ == 2) {
        if(!sakuyaSheet) sakuyaSheet = IMG_Load(sheetFile);
        sheet = sakuyaSheet;
    } else {
        if(!marisaSheet) marisaSheet = IMG_Load(sheetFile);
        sheet = marisaSheet;
    }
    if(sheet == NULL) return;

    float progress = 1.0f - (float)timer_ / (float)duration_;

    int drawY = (int)(600.0f - progress * (600.0f + 512.0f));
    SDL_Rect dst = {0, (Sint16)drawY, 0, 0};
    SDL_BlitSurface(sheet, &portraitRects[3], screen, &dst);
}

void PlayerBomb::reimuA_bomb_animation(GameBackground*) {
    draw_portrait("res/face/reimu/face_rm00.png");
}
void PlayerBomb::reimuB_bomb_animation(GameBackground*) {
    draw_portrait("res/face/reimu/face_rm00.png");
}
void PlayerBomb::marisaA_bomb_animation(GameBackground*) {
    draw_portrait(playerType_ == 2 ? "res/face/sakuya/face_sk00.png" : "res/face/marisa/face_mr00.png");
}
void PlayerBomb::marisaB_bomb_animation(GameBackground*) {
    draw_portrait(playerType_ == 2 ? "res/face/sakuya/face_sk00.png" : "res/face/marisa/face_mr00.png");
}
