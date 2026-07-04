#include "InfoArea.h"
#include <cstdio>

SDL_Surface* InfoArea::infoAreaTH07Logo = NULL;
SDL_Surface* InfoArea::perfectCherryBlossomImage = NULL;
SDL_Surface* InfoArea::highScoreImage = NULL;
SDL_Surface* InfoArea::scoreImage = NULL;
SDL_Surface* InfoArea::livesImage = NULL;
SDL_Surface* InfoArea::spellImage = NULL;
SDL_Surface* InfoArea::powerImage = NULL;
SDL_Surface* InfoArea::grazeImage = NULL;
SDL_Surface* InfoArea::pointImage = NULL;
SDL_Surface* InfoArea::timeImage = NULL;
SDL_Surface* InfoArea::redStarImage = NULL;
SDL_Surface* InfoArea::blueStarImage = NULL;

static bool assetsLoaded = false;

static const int BAR_X  = 544;
static const int BAR_W  = 256;
static const int BAR_H  = 600;
static const int LBL_X  = 548;
static const int VAL_X  = 604;
static const int STAR_X = 604;

static void draw_digit(int x, int y, int d) {
    if (d < 0 || d > 9) return;
    apply_surface(x, y, (SDL_Surface*)numbersImage[d], screen);
}

static void draw_num(int x, int y, Uint32 v, int minW) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%0*u", minW, v);
    for (int i = 0; buf[i]; i++) {
        draw_digit(x, y, buf[i] - '0');
        x += 16;
    }
}

static void draw_stars(int x, int y, int n, SDL_Surface* spr, int wrap) {
    for (int i = 0; i < n; i++) {
        int sx = x + (i % wrap) * 18;
        int sy = y + (i / wrap) * 18;
        if (spr) apply_surface(sx, sy, spr, screen);
    }
}

static void draw_powers(int x, int y, int pwr){
    int pwr100 = 0;
    int pwr10 = 0;
    int pwr1 = 0;
    if(pwr > 100) {
        pwr100 = pwr / 100;
        pwr10 = (pwr / 10) % 10;
        pwr1 = pwr % 10;
    }else if(pwr > 10) {
        pwr10 = pwr / 10;
        pwr1 = pwr % 10;
    }else{
        pwr1 = pwr;
    }
    draw_digit(x, y, pwr100);
    draw_digit(x + 16, y, pwr10);
    draw_digit(x + 32, y, pwr1);
}

InfoArea::InfoArea() : currentScore(0), lives(0), spell(0), graze(0), point(0), time_(0) {
    if (!assetsLoaded) {
        infoAreaTH07Logo = load_sprite("res/front/front.png", 128, 0, 128, 256, 192.0, 384.0);
        perfectCherryBlossomImage = load_sprite("res/front/front.png", 0, 0, 128, 64, 192.0, 96.0);
        highScoreImage   = load_sprite("res/front/front.png", 0,  80, 48, 16, 48.0, 16.0);
        scoreImage       = load_sprite("res/front/front.png", 0,  96, 48, 16, 48.0, 16.0);
        livesImage       = load_sprite("res/front/front.png", 0, 112, 48, 16, 48.0, 16.0);
        spellImage       = load_sprite("res/front/front.png", 0, 128, 48, 16, 48.0, 16.0);
        powerImage       = load_sprite("res/front/front.png", 0, 144, 48, 16, 48.0, 16.0);
        grazeImage       = load_sprite("res/front/front.png", 0, 160, 48, 16, 48.0, 16.0);
        pointImage       = load_sprite("res/front/front.png", 0, 176, 48, 16, 48.0, 16.0);
        timeImage        = load_sprite("res/front/front.png", 0, 192, 48, 16, 48.0, 16.0);
        redStarImage     = load_sprite("res/front/front.png", 64, 80, 16, 16, 16.0, 16.0);
        blueStarImage    = load_sprite("res/front/front.png", 80, 80, 16, 16, 16.0, 16.0);
        assetsLoaded = true;
    }
}

InfoArea::~InfoArea() {}

void InfoArea::update_high_score(Uint32 hs) { /* reserved */ }
void InfoArea::update_score(Uint32 sc)      { currentScore = sc; }
void InfoArea::update_lives(Uint32 l)       { lives = l; }
void InfoArea::update_spell(Uint32 s)       { spell = s; }
void InfoArea::update_graze(Uint32 g)       { graze = g; }
void InfoArea::update_point(Uint32 p)       { point = p; }
void InfoArea::update_time(Uint32 t)        { time_ = t; }

void InfoArea::render() {
    SDL_Rect bar = {BAR_X, 0, BAR_W, BAR_H};
    SDL_FillRect(screen, &bar, SDL_MapRGB(screen->format, 0, 0, 64));

    if (infoAreaTH07Logo)
        apply_surface(608, 216, infoAreaTH07Logo, screen);
    if(perfectCherryBlossomImage)
        apply_surface(544, 400, perfectCherryBlossomImage, screen);

    const int ry[8] = {16, 36, 56, 76, 96, 116, 136, 156};

    if (highScoreImage) apply_surface(LBL_X, ry[0], highScoreImage, screen);
    draw_num(VAL_X, ry[0], currentScore, 10);

    if (scoreImage) apply_surface(LBL_X, ry[1], scoreImage, screen);
    draw_num(VAL_X, ry[1], currentScore, 10);

    if (livesImage) apply_surface(LBL_X, ry[2], livesImage, screen);
    draw_stars(STAR_X, ry[2], lives, redStarImage, 10);

    if (spellImage) apply_surface(LBL_X, ry[3], spellImage, screen);
    draw_stars(STAR_X, ry[3], spell, blueStarImage, 10);

    if (powerImage) apply_surface(LBL_X, ry[4], powerImage, screen);
    draw_powers(VAL_X, ry[4], Player::get_power());

    if (grazeImage) apply_surface(LBL_X, ry[5], grazeImage, screen);
    draw_num(VAL_X, ry[5], graze, 5);

    if (pointImage) apply_surface(LBL_X, ry[6], pointImage, screen);
    draw_num(VAL_X, ry[6], point, 5);

    if (timeImage) apply_surface(LBL_X, ry[7], timeImage, screen);
    draw_num(VAL_X, ry[7], time_, 5);
}
