#include "DifficultyMenu.h"
SDL_Surface* DifficultyMenu::select01 = NULL;
SDL_Surface* DifficultyMenu::select00_ = NULL;
SDL_Rect DifficultyMenu::select01Rect[5] = {NULL};
SDL_Surface* DifficultyMenu::selectLevelImage = NULL;
Mix_Chunk* DifficultyMenu::select00 = NULL;
Mix_Chunk* DifficultyMenu::ok00 = NULL;

DifficultyMenu::DifficultyMenu(): currentSelectedDifficulty(0){
    if(select00_ == NULL)    select00_ = load_image("res/title/select00.jpg", 800.0, 600.0);
    if(select01 == NULL)    select01 = load_image("res/title/select01.png", 512.0, 512.0);
    if(selectLevelImage == NULL)    selectLevelImage = load_sprite("res/title/select01.png", 0, 480, 256, 32, 256, 32);
    if(select00 == NULL)    select00 = Mix_LoadWAV("res/sound/se_select00.wav");
    if(ok00 == NULL)    ok00 = Mix_LoadWAV("res/sound/se_ok00.wav");
    for(short i = 0;i < 5; i++)   select01Rect[i] = {256, (Sint16)(i * 96), 256, 96};
}
void DifficultyMenu::switch_to_next_difficulty(){
    if(currentSelectedDifficulty >= 4) return;
    currentSelectedDifficulty++;
    Mix_PlayChannel(-1, select00, 0);
}
void DifficultyMenu::switch_to_previous_difficulty(){
    if(currentSelectedDifficulty <= 0) return;
    currentSelectedDifficulty--;
    Mix_PlayChannel(-1, select00, 0);
}
void DifficultyMenu::jump_to_submenu(){
   gameDifficulty = currentSelectedDifficulty;
    Mix_PlayChannel(-1, ok00, 0);
    //Mix_HaltMusic();
    gameState = STATE_START_MENU;
}

void DifficultyMenu::handle_events(SDL_Event &e){
    if(e.type == SDL_KEYDOWN){
		switch (e.key.keysym.sym){
			case SDLK_UP: switch_to_previous_difficulty(); break;
			case SDLK_DOWN: switch_to_next_difficulty(); break;
			case SDLK_RETURN: jump_to_submenu(); break;
			case SDLK_ESCAPE: back_to_mainmenu(); break;
		}
	}
}

void DifficultyMenu::back_to_mainmenu(){
    //Mix_HaltMusic();
    Mix_PlayChannel(-1, ok00, 0);
    gameState = STATE_MENU;
}

DifficultyMenu::~DifficultyMenu(){
    if(select00 != NULL){
        Mix_FreeChunk(select00);
        select00 = NULL;
    }
    if(ok00 != NULL){
        Mix_FreeChunk(ok00);
        ok00 = NULL;
    }
}

SDL_Rect DifficultyMenu::get_menu_src_rect(int id, bool isSelected){
    if(isSelected){
        SDL_Rect rect = {0, select01Rect[id].y, select01Rect[id].w, select01Rect[id].h};
        return rect;
    }else{
        SDL_Rect rect = {256, select01Rect[id].y, select01Rect[id].w, select01Rect[id].h};
        return rect;
    }
}

void DifficultyMenu::render(){
    apply_surface(0, 0, select00_, screen);
    apply_surface(272, 0, selectLevelImage, screen);
    int startY = 128;
    int spacingY = 96;
    for(int i = 0;i < 5;i++){
        bool isSelected_ = (currentSelectedDifficulty == i);
        SDL_Rect rect_ = get_menu_src_rect(i, isSelected_);
        SDL_Rect dest = {272, (Sint16)(startY + (i*spacingY)), 256, 96};
        SDL_BlitSurface(select01, &rect_, screen, &dest);
    }
}