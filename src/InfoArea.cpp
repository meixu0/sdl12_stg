#include "InfoArea.h"
#include <cstdio>
static SDL_Surface* infoSheet = NULL;
static SDL_Surface* th07logo = NULL;
static SDL_Surface* infoAreaBg = NULL;
static SDL_Surface* PerfectCherryBlossom = NULL;
static SDL_Rect th07logoSrc = {0, 0, 192, 384};
static SDL_Rect th07logoDst = {608, 216, 0, 0};
static SDL_Rect perfectCherryBlossomDst = {544, 400, 0, 0};
static SDL_Rect infoSrc[] = {
    {128,   0, 128, 256},  // 0: TH07 Logo
    {  0,   0, 128,  64},  // 1: Perfect Cherry Blossom
    {  0,  80,  48,  16},  // 2: High Score
    {  0,  96,  48,  16},  // 3: Score
    {  0, 112,  48,  16},  // 4: Lives
    {  0, 128,  48,  16},  // 5: Spell
    {  0, 144,  48,  16},  // 6: Power
    {  0, 160,  48,  16},  // 7: Graze
    {  0, 176,  48,  16},  // 8: Point
    {  0, 192,  48,  16},  // 9: Time
    { 64,  80,  16,  16},  // 10: Red Star
    { 80,  80,  16,  16},  // 11: Blue Star
};

static const int BAR_X  = 544;
static const int BAR_W  = 256;
static const int BAR_H  = 600;
static const int LBL_X  = 548;
static const int VAL_X  = 604;
static const int STAR_X = 604;

static void draw_digit(int x, int y, int d) {
    if (d < 0 || d > 9) return;
    SDL_Rect dst = {(Sint16)x, (Sint16)y, 0, 0};
    SDL_BlitSurface(asciiSheet, &digitRects[d], screen, &dst);
}

static void draw_num(int x, int y, Uint32 v, int minW) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%0*u", minW, v);
    for (int i = 0; buf[i]; i++) {
        draw_digit(x, y, buf[i] - '0');
        x += 16;
    }
}

static void draw_blit(int idx, int x, int y) {
    SDL_Rect dst = {(Sint16)x, (Sint16)y, 0, 0};
    SDL_BlitSurface(infoSheet, &infoSrc[idx], screen, &dst);
}

static void draw_stars(int x, int y, int n, int sprIdx, int wrap) {
    for (int i = 0; i < n; i++) {
        int sx = x + (i % wrap) * 18;
        int sy = y + (i / wrap) * 18;
        draw_blit(sprIdx, sx, sy);
    }
}

static void draw_powers(int x, int y, int pwr){
    int pwr100 = 0, pwr10 = 0, pwr1 = 0;
    if(pwr > 100)       { pwr100 = pwr/100; pwr10 = (pwr/10)%10; pwr1 = pwr%10; }
    else if(pwr > 10)   { pwr10 = pwr/10;   pwr1 = pwr%10; }
    else                { pwr1 = pwr; }
    draw_digit(x, y, pwr100);
    draw_digit(x + 16, y, pwr10);
    draw_digit(x + 32, y, pwr1);
}

InfoArea::InfoArea() : currentScore(0), lives(0), spell(0), graze(0), point(0), time_(0) {
    if (infoSheet == NULL)
        infoSheet = IMG_Load("res/front/front.png");
    if( th07logo == NULL)
        th07logo = IMG_Load("res/front/th07logo.png");
    if( infoAreaBg == NULL)
        infoAreaBg = IMG_Load("res/front/infoAreaBg.png");
    if( PerfectCherryBlossom == NULL)
        PerfectCherryBlossom = IMG_Load("res/front/PerfectCherryBlossom.png");
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
    SDL_BlitSurface(infoAreaBg, NULL, screen, &bar);

    if (infoSheet == NULL) return;

    if (th07logo != NULL) {
        SDL_BlitSurface(th07logo, &th07logoSrc, screen, &th07logoDst);
    }
    if (PerfectCherryBlossom != NULL) {
        SDL_BlitSurface(PerfectCherryBlossom, NULL, screen, &perfectCherryBlossomDst);
    }

    const int ry[8] = {16, 36, 56, 76, 96, 116, 136, 156};

    draw_blit(2, LBL_X, ry[0]);  // High Score
    draw_num(VAL_X, ry[0], highScore, 10);

    draw_blit(3, LBL_X, ry[1]);  // Score
    draw_num(VAL_X, ry[1], currentScore, 10);

    draw_blit(4, LBL_X, ry[2]);  // Lives
    draw_stars(STAR_X, ry[2], lives, 10, 10);

    draw_blit(5, LBL_X, ry[3]);  // Spell
    draw_stars(STAR_X, ry[3], spell, 11, 10);

    draw_blit(6, LBL_X, ry[4]);  // Power
    draw_powers(VAL_X, ry[4], Player::get_power());

    draw_blit(7, LBL_X, ry[5]);  // Graze
    draw_num(VAL_X, ry[5], graze, 5);

    draw_blit(8, LBL_X, ry[6]);  // Point
    draw_num(VAL_X, ry[6], point, 5);

    draw_blit(9, LBL_X, ry[7]);  // Time
    draw_num(VAL_X, ry[7], time_, 5);
}
