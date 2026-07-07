#include "StartMenu.h"

ShotRects StartMenu::shotRects[4] = {
    // Reimu
    {{0, 0, 256, 512}, {256, 0, 256, 128}, {256, 128, 256, 128}, {256, 256, 256, 128}, {256, 384, 256, 128}},
    // Marisa
    {{0, 0, 256, 512}, {256, 0, 256, 128}, {256, 128, 256, 128}, {256, 256, 256, 128}, {256, 384, 256, 128}},
    // Sakuya
    {{0, 0, 256, 512}, {256, 0, 256, 128}, {256, 128, 256, 128}, {256, 256, 256, 128}, {256, 384, 256, 128}},
    // Youmu（坐标完全一致）
    {{0, 0, 256, 512}, {256, 0, 256, 128}, {256, 128, 256, 128}, {256, 256, 256, 128}, {256, 384, 256, 128}},
};

SDL_Surface* StartMenu::plSheet[4]     = {NULL, NULL, NULL, NULL};
SDL_Surface* StartMenu::select00img    = NULL;
SDL_Surface* StartMenu::playerSelectImage = NULL;
Mix_Chunk*   StartMenu::select00       = NULL;
Mix_Chunk*   StartMenu::ok00           = NULL;
Mix_Chunk*   StartMenu::cancel00       = NULL;

StartMenu::StartMenu(): currentPlayerIndex(0), currentScIndex(0){
    playerType = PLAYER_REIMU;
    if(plSheet[0] == NULL) plSheet[0] = load_image("res/title/sl_pl00.png", 512, 512);
    if(plSheet[1] == NULL) plSheet[1] = load_image("res/title/sl_pl01.png", 512, 512);
    if(plSheet[2] == NULL) plSheet[2] = load_image("res/title/sl_pl02.png", 512, 512);
    if(plSheet[3] == NULL) plSheet[3] = load_image("res/title/sl_pl03.png", 512, 512);
    if(select00img == NULL)      select00img      = load_image("res/title/select00.jpg", 800, 600);
    if(playerSelectImage == NULL) playerSelectImage = load_image("res/title/playerSelectImage.png", 256, 32);
    if(select00 == NULL) select00 = Mix_LoadWAV("res/sound/se_select00.wav");
    if(cancel00 == NULL) cancel00 = Mix_LoadWAV("res/sound/se_cancel00.wav");
    if(ok00     == NULL) ok00     = Mix_LoadWAV("res/sound/se_ok00.wav");
}

void StartMenu::switch_to_next_player(){
    if(currentPlayerIndex >= 3) return;
    currentPlayerIndex++;
    currentScIndex = currentPlayerIndex * 2;
    playerType = currentPlayerIndex;
    Mix_PlayChannel(-1, select00, 0);
}

void StartMenu::switch_to_previous_player(){
    if(currentPlayerIndex <= 0) return;
    currentPlayerIndex--;
    currentScIndex = currentPlayerIndex * 2;
    playerType = currentPlayerIndex;
    Mix_PlayChannel(-1, select00, 0);
}

void StartMenu::jump_to_game(){
    playerType = currentPlayerIndex;
    Mix_PlayChannel(-1, ok00, 0);
    Mix_HaltMusic();
    static Mix_Music* stage1BGM = Mix_LoadMUS("res/music/th07_02.mid");
    if(stage1BGM != NULL) Mix_PlayMusic(stage1BGM, -1);
    gameState = STATE_GAME;
}

void StartMenu::switch_to_next_sc(){
    if(currentScIndex % 2 == 1) return;
    currentScIndex++;
    Mix_PlayChannel(-1, select00, 0);
}

void StartMenu::switch_to_previous_sc(){
    if(currentScIndex % 2 == 0) return;
    currentScIndex--;
    Mix_PlayChannel(-1, select00, 0);
}

void StartMenu::handle_events(SDL_Event &e){
    if(e.type == SDL_KEYDOWN){
        switch (e.key.keysym.sym){
            case SDLK_LEFT:  switch_to_previous_player(); break;
            case SDLK_RIGHT: switch_to_next_player();     break;
            case SDLK_RETURN: jump_to_game();             break;
            case SDLK_ESCAPE: back_to_difficulty_menu();  break;
            case SDLK_UP:    switch_to_previous_sc();     break;
            case SDLK_DOWN:  switch_to_next_sc();         break;
        }
    }
}

void StartMenu::back_to_difficulty_menu(){
    Mix_PlayChannel(-1, cancel00, 0);
    gameState = STATE_DIFFICULTY_MENU;
}

StartMenu::~StartMenu(){
    if(select00 != NULL){ Mix_FreeChunk(select00); select00 = NULL; }
    if(ok00     != NULL){ Mix_FreeChunk(ok00);     ok00     = NULL; }
}

void StartMenu::render(){
    SDL_BlitSurface(select00img, NULL, screen, NULL);
    SDL_Rect playerSelectDest = {272, 32, 256, 32};
    SDL_BlitSurface(playerSelectImage, NULL, screen, &playerSelectDest);

    int p = currentPlayerIndex; 
    SDL_Rect portDest = {0, 44, 256, 512};
    SDL_BlitSurface(plSheet[p], &shotRects[p].portrait, screen, &portDest);
    // scIndex 偶数 → shotA 选中 / shotB 普通
    // scIndex 奇数 → shotA 普通 / shotB 选中
    bool selA = (currentScIndex % 2 == 0);
    SDL_Rect* srcA = selA ? &shotRects[p].shotA_sel  : &shotRects[p].shotA_norm;
    SDL_Rect* srcB = selA ? &shotRects[p].shotB_norm : &shotRects[p].shotB_sel;

    SDL_Rect destA = {400, 280, 0, 0};
    SDL_Rect destB = {400, 440, 0, 0};
    SDL_BlitSurface(plSheet[p], srcA, screen, &destA);
    SDL_BlitSurface(plSheet[p], srcB, screen, &destB);
}
