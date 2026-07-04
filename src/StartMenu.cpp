#include "StartMenu.h"
#include <iostream>

static SDL_Surface* load_sprite_section(const std::string& filename, int srcX, int srcY, int srcW, int srcH, int targetW, int targetH){
	SDL_Surface* sheet = load_image(filename, srcW, srcH);
	if(sheet == NULL) return NULL;
	SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA,
		targetW, targetH,
		sheet->format->BitsPerPixel,
		sheet->format->Rmask,
		sheet->format->Gmask,
		sheet->format->Bmask,
		sheet->format->Amask);
	if(result == NULL){
		SDL_FreeSurface(sheet);
		return NULL;
	}
	SDL_FillRect(result, NULL, SDL_MapRGBA(result->format, 0, 0, 0, 0));
	SDL_Rect srcRect = {(Sint16)srcX, (Sint16)srcY, (Uint16)srcW, (Uint16)srcH};
	SDL_BlitSurface(sheet, &srcRect, result, NULL);
	SDL_FreeSurface(sheet);
	return result;
}

SDL_Surface* StartMenu::pl00 = NULL;
SDL_Surface* StartMenu::pl01 = NULL;
SDL_Surface* StartMenu::pl02 = NULL;
SDL_Surface* StartMenu::select00img = NULL;
Mix_Chunk* StartMenu::select00 = NULL;
Mix_Chunk* StartMenu::ok00 = NULL;
Mix_Chunk* StartMenu::cancel00 = NULL;
SDL_Surface* StartMenu::reimuTsuiti[2] = {NULL, NULL};
SDL_Surface* StartMenu::reimuSokusha[2] = {NULL, NULL};
SDL_Surface* StartMenu::marisaIryokujuushi[2] = {NULL, NULL};
SDL_Surface* StartMenu::marisaKantsuu[2] = {NULL, NULL};
SDL_Surface* StartMenu::sakuyaKouhanni[2] = {NULL, NULL};
SDL_Surface* StartMenu::sakuyaTokushu[2] = {NULL, NULL};
StartMenu::StartMenu(): currentPlayerIndex(0), currentScIndex(0){
    playerType = PLAYER_REIMU;
    if(pl00 == NULL)    pl00 = load_sprite("res/title/sl_pl00.png", 0, 0, 256, 512, 400, 600);
    if(pl01 == NULL)    pl01 = load_sprite("res/title/sl_pl01.png", 0, 0, 256, 512, 400, 600);
    if(pl02 == NULL)    pl02 = load_sprite("res/title/sl_pl02.png", 0, 0, 256, 512, 400, 600);
    if(select00img == NULL) select00img = load_image("res/title/select00.jpg", 800, 600);
    if(select00 == NULL)	select00 = Mix_LoadWAV("res/sound/se_select00.wav");
    if(cancel00 == NULL)	cancel00 = Mix_LoadWAV("res/sound/se_cancel00.wav");
	if(ok00 == NULL)	ok00 = Mix_LoadWAV("res/sound/se_ok00.wav");
    if(reimuTsuiti[0] == NULL){
        reimuTsuiti[0] = load_sprite("res/title/sl_pl00.png", 256, 128, 256, 128, 320, 160);
        reimuTsuiti[1] = load_sprite("res/title/sl_pl00.png", 256, 0, 256, 128, 320, 160);
    }
    if(reimuSokusha[0] == NULL){
        reimuSokusha[0] = load_sprite("res/title/sl_pl00.png", 256, 384, 256, 128, 320, 160);
        reimuSokusha[1] = load_sprite("res/title/sl_pl00.png", 256, 256, 256, 128, 320, 160);
    }
    if(marisaIryokujuushi[0] == NULL){
        marisaIryokujuushi[0] = load_sprite("res/title/sl_pl01.png", 256, 128, 256, 128, 320, 160);
        marisaIryokujuushi[1] = load_sprite("res/title/sl_pl01.png", 256, 0, 256, 128, 320, 160);
    }
    if(marisaKantsuu[0] == NULL){
        marisaKantsuu[0] = load_sprite("res/title/sl_pl01.png", 256, 384, 256, 128, 320, 160);
        marisaKantsuu[1] = load_sprite("res/title/sl_pl01.png", 256, 256, 256, 128, 320, 160);
    }
    if(sakuyaKouhanni[0] == NULL){
        sakuyaKouhanni[0] = load_sprite("res/title/sl_pl02.png", 256, 128, 256, 128, 320, 160);
        sakuyaKouhanni[1] = load_sprite("res/title/sl_pl02.png", 256, 0, 256, 128, 320, 160);
    }
    if(sakuyaTokushu[0] == NULL){
        sakuyaTokushu[0] = load_sprite("res/title/sl_pl02.png", 256, 384, 256, 128, 320, 160);
        sakuyaTokushu[1] = load_sprite("res/title/sl_pl02.png", 256, 256, 256, 128, 320, 160);
    }
}

void StartMenu::switch_to_next_player(){
    if(currentPlayerIndex >= 2) return;
    switch (currentPlayerIndex){
    case 0:
        currentScIndex = 2;
        break;
    case 1:
        currentScIndex = 4;
        break;
    }
    currentPlayerIndex++;
    playerType = currentPlayerIndex;
     std::cout<<"current player type is"<<playerType<<std::endl;
    Mix_PlayChannel(-1, select00, 0);
}
void StartMenu::switch_to_previous_player(){
    if(currentPlayerIndex <= 0) return;
    switch (currentPlayerIndex){
    case 1:
        currentScIndex = 0;
        break;
    case 2:
        currentScIndex = 2;
        break;
    }
    currentPlayerIndex--;
    playerType = currentPlayerIndex;
    std::cout<<"current player type is"<<playerType<<std::endl;
    Mix_PlayChannel(-1, select00, 0);
}
void StartMenu::jump_to_game(){
    playerType = currentPlayerIndex;
    std::cout<<"current player type is"<<playerType<<std::endl;
    Mix_PlayChannel(-1, ok00, 0);
    Mix_HaltMusic();
    static Mix_Music* stage1BGM = Mix_LoadMUS("res/music/th07_02.mid");
    if(stage1BGM != NULL) Mix_PlayMusic(stage1BGM, -1);
    gameState = STATE_GAME;
}
void StartMenu::switch_to_next_sc(){
    if(currentScIndex == 1 || currentScIndex == 3 || currentScIndex == 5)   return;
    currentScIndex++;
    Mix_PlayChannel(-1, select00, 0);
}
void StartMenu::switch_to_previous_sc(){
    if(currentScIndex == 0 || currentScIndex == 2 || currentScIndex == 4)   return;
    currentScIndex--;
    Mix_PlayChannel(-1, select00, 0);
}

void StartMenu::handle_events(SDL_Event &e){
    if(e.type == SDL_KEYDOWN){
		switch (e.key.keysym.sym){
			case SDLK_LEFT: switch_to_previous_player(); break;
			case SDLK_RIGHT: switch_to_next_player(); break;
			case SDLK_RETURN: jump_to_game(); break;
			case SDLK_ESCAPE: back_to_difficulty_menu(); break;
            case SDLK_UP: switch_to_previous_sc(); break;
            case SDLK_DOWN: switch_to_next_sc(); break;
		}
	}
}

void StartMenu::back_to_difficulty_menu(){
    //Mix_HaltMusic();
    Mix_PlayChannel(-1, cancel00, 0);
    gameState = STATE_DIFFICULTY_MENU;
}

StartMenu::~StartMenu(){
    if(select00 != NULL){
        Mix_FreeChunk(select00);
        select00 = NULL;
    }
    if(ok00 != NULL){
        Mix_FreeChunk(ok00);
        ok00 = NULL;
    }
}

void StartMenu::render(){
    apply_surface(0, 0, select00img, screen);
    switch(currentPlayerIndex){
        case 0: 
            apply_surface(0, 0, pl00, screen); 
            switch(currentScIndex){
                case 0:
                    apply_surface(400, 280, reimuTsuiti[1], screen);
                    apply_surface(400, 440, reimuSokusha[0], screen);
                    break;
                case 1:
                    apply_surface(400, 280, reimuTsuiti[0], screen);
                    apply_surface(400, 440, reimuSokusha[1], screen);
                    break;
            }
            break;
        case 1: 
            apply_surface(0, 0, pl01, screen); 
            switch(currentScIndex){
                case 2:
                    apply_surface(400, 280, marisaIryokujuushi[1], screen);
                    apply_surface(400, 440, marisaKantsuu[0], screen);
                    break;
                case 3:
                    apply_surface(400, 280, marisaIryokujuushi[0], screen);
                    apply_surface(400, 440, marisaKantsuu[1], screen);
                    break;
            }
            break;
        case 2: 
            apply_surface(0, 0, pl02, screen); 
            switch(currentScIndex){
                case 4:
                    apply_surface(400, 280, sakuyaKouhanni[1], screen);
                    apply_surface(400, 440, sakuyaTokushu[0], screen);
                    break;
                case 5:
                    apply_surface(400, 280, sakuyaKouhanni[0], screen);
                    apply_surface(400, 440, sakuyaTokushu[1], screen);
                    break;
            }
            break;
    }
}