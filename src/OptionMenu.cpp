#include "OptionMenu.h"
SDL_Surface* OptionMenu::select00img = NULL;
SDL_Surface* OptionMenu::titleSheet = NULL;
Mix_Chunk*   OptionMenu::select00   = NULL;
Mix_Chunk*   OptionMenu::cancel00   = NULL;

OptionMenu::OptionMenu() : currentRow(0), playerLives_(3), playerBombs_(3) {
    if(select00img == NULL)      
        select00img = load_image("res/title/select00.jpg", 800, 600);

    if(titleSheet == NULL)
        titleSheet = IMG_Load("res/title/title01.png");
    if(select00 == NULL)
        select00 = Mix_LoadWAV("res/sound/se_select00.wav");
    if(cancel00 == NULL)
        cancel00 = Mix_LoadWAV("res/sound/se_cancel00.wav");
}

OptionMenu::~OptionMenu() {}

void OptionMenu::switch_up(){
    if(currentRow > 0){ currentRow--; Mix_PlayChannel(-1, select00, 0); }
}
void OptionMenu::switch_down(){
    if(currentRow < 1){ currentRow++; Mix_PlayChannel(-1, select00, 0); }
}
void OptionMenu::switch_left(){
    if(currentRow == 0 && playerLives_ > 1){ playerLives_--; playerLives = playerLives_; Mix_PlayChannel(-1, select00, 0); }
    if(currentRow == 1 && playerBombs_ > 1){ playerBombs_--; playerBombs = playerBombs_; Mix_PlayChannel(-1, select00, 0); }
}
void OptionMenu::switch_right(){
    if(currentRow == 0 && playerLives_ < 9){ playerLives_++; playerLives = playerLives_; Mix_PlayChannel(-1, select00, 0); }
    if(currentRow == 1 && playerBombs_ < 9){ playerBombs_++; playerBombs = playerBombs_; Mix_PlayChannel(-1, select00, 0); }
}
void OptionMenu::back_to_main(){
    Mix_PlayChannel(-1, cancel00, 0);
    gameState = STATE_MENU;
}

void OptionMenu::handle_events(SDL_Event &e){
    if(e.type != SDL_KEYDOWN) return;
    switch(e.key.keysym.sym){
        case SDLK_UP:    switch_up();    break;
        case SDLK_DOWN:  switch_down();  break;
        case SDLK_LEFT:  switch_left();  break;
        case SDLK_RIGHT: switch_right(); break;
        case SDLK_ESCAPE: back_to_main(); break;
    }
}

void OptionMenu::render(){
    SDL_BlitSurface(select00img, NULL, screen, NULL);
    if(titleSheet == NULL) return;
    SDL_Rect playerSrc;
    if (currentRow == 0)
        { playerSrc.x = 0; playerSrc.y = 128; playerSrc.w = 96; playerSrc.h = 32; }
    else
        { playerSrc.x = 0; playerSrc.y = 192; playerSrc.w = 96; playerSrc.h = 32; }
    SDL_Rect playerDst = {64, 200, 0, 0};
    SDL_BlitSurface(titleSheet, &playerSrc, screen, &playerDst);
    SDL_Rect bombSrc;
    if (currentRow == 1)
        { bombSrc.x = 80; bombSrc.y = 384; bombSrc.w = 80; bombSrc.h = 32; }
    else
        { bombSrc.x = 80; bombSrc.y = 448; bombSrc.w = 80; bombSrc.h = 32; }
    SDL_Rect bombDst = {64, 300, 0, 0};
    SDL_BlitSurface(titleSheet, &bombSrc, screen, &bombDst);
    int numY_sel = 256, numY_norm = 320;
    int numStartX = 200;

    // Player
    for(int n = 0; n < 9; n++){
        bool sel = (n == playerLives_ - 1);
        SDL_Rect src = {(Sint16)(32 + n * 32), (Sint16)(sel ? numY_sel : numY_norm), 32, 32};
        SDL_Rect dst = {(Sint16)(numStartX + n * 32), 200, 0, 0};
        SDL_BlitSurface(titleSheet, &src, screen, &dst);
    }

    // Bomb
    for(int n2 = 0; n2 < 9; n2++){
        bool sel = (n2 == playerBombs_ - 1);
        SDL_Rect src = {(Sint16)(32 + n2 * 32), (Sint16)(sel ? numY_sel : numY_norm), 32, 32};
        SDL_Rect dst = {(Sint16)(numStartX + n2 * 32), 300, 0, 0};
        SDL_BlitSurface(titleSheet, &src, screen, &dst);
    }
}
