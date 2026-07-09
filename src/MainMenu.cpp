#include "main_menu.h"
#include <iostream>
TTF_Font* mainMenuFont = NULL;
SDL_Surface* MainMenu::title00 = NULL;
SDL_Surface* MainMenu::title01 = NULL;
SDL_Surface* MainMenu::title02 = NULL;
Mix_Chunk* MainMenu::select00 = NULL;
Mix_Chunk* MainMenu::ok00 = NULL;
Mix_Music* MainMenu::menuBGM = NULL;
SDL_Rect MainMenu::screenRect;
SDL_Rect MainMenu::menuSrcRects[10] = {NULL};
void jump_to_game(){
	std::cout << "jump to game" << std::endl;
	gameState = STATE_GAME;
	static SDL_Surface* gameBackground = NULL;
	if(SDL_Flip(screen) == -1)	std::cout << "flip error in jump_to_game" << std::endl;
}
MainMenu::MainMenu(): currentSelectedIndex(0){
	if(title00 == NULL)	title00 = load_image("res/title/title00.png", 800.0, 600.0);
	if(title01 == NULL)	title01 = load_image("res/title/title01.png", 512, 512);
	if(title02 == NULL)	title02 = load_image("res/title/title02.png", 512, 256);
	screenRect.x = 0; screenRect.y = 0; screenRect.w = 800; screenRect.h = 600;
	menuSrcRects[0].x = 8; menuSrcRects[0].y = 0; menuSrcRects[0].w = 64; menuSrcRects[0].h = 32;
	menuSrcRects[1].x = 100; menuSrcRects[1].y = 32; menuSrcRects[1].w = 80; menuSrcRects[1].h = 32;
	menuSrcRects[2].x = 196; menuSrcRects[2].y = 32; menuSrcRects[2].w = 148; menuSrcRects[2].h = 32;
	menuSrcRects[3].x = 360; menuSrcRects[3].y = 32; menuSrcRects[3].w = 84; menuSrcRects[3].h = 32;
	menuSrcRects[4].x = 435; menuSrcRects[4].y = 0; menuSrcRects[4].w = 56; menuSrcRects[4].h = 32;
	menuSrcRects[5].x = 8; menuSrcRects[5].y = 64; menuSrcRects[5].w = 64; menuSrcRects[5].h = 32;
	menuSrcRects[6].x = 100; menuSrcRects[6].y = 96; menuSrcRects[6].w = 80; menuSrcRects[6].h = 32;
	menuSrcRects[7].x = 196; menuSrcRects[7].y = 96; menuSrcRects[7].w = 148; menuSrcRects[7].h = 32;
	menuSrcRects[8].x = 360; menuSrcRects[8].y = 96; menuSrcRects[8].w = 84; menuSrcRects[8].h = 32;
	menuSrcRects[9].x = 435; menuSrcRects[9].y = 64; menuSrcRects[9].w = 56; menuSrcRects[9].h = 32;

	if(select00 == NULL)	select00 = Mix_LoadWAV("res/sound/se_select00.wav");
	if(ok00 == NULL)	ok00 = Mix_LoadWAV("res/sound/se_ok00.wav");
	if(menuBGM == NULL)	menuBGM = Mix_LoadMUS("res/music/th07_01.mid");
	if(menuBGM != NULL)	Mix_PlayMusic(menuBGM, -1);
}
MainMenu::~MainMenu(){
	if(menuBGM != NULL){
		Mix_HaltMusic();
		Mix_FreeMusic(menuBGM);
		menuBGM = NULL;
	}
}
void MainMenu::switch_to_previous_option(){
	if(currentSelectedIndex <= 0)	return;
	currentSelectedIndex--;
	Mix_PlayChannel(-1, select00, 0);
}
void MainMenu::switch_to_next_option(){
	if(currentSelectedIndex >= 4)	return;
	currentSelectedIndex++;
	Mix_PlayChannel(-1, select00, 0);
}
void MainMenu::jump_to_submenu(){
	if(currentSelectedIndex == 0)	{Mix_PlayChannel(-1, ok00, 0);gameState = STATE_DIFFICULTY_MENU;}
	if(currentSelectedIndex == 1)	{Mix_PlayChannel(-1, ok00, 0);gameState = STATE_RESULT;}
	if(currentSelectedIndex == 2)	{Mix_PlayChannel(-1, ok00, 0);gameState = STATE_MUSICROOM;}
	if(currentSelectedIndex == 3)	{Mix_PlayChannel(-1, ok00, 0);gameState = STATE_OPTION_MENU;}
	if(currentSelectedIndex == 4)	{Mix_PlayChannel(-1, ok00, 0);gameState = STATE_EXIT;}
}

void MainMenu::jump_to_option_menu(){
	Mix_PlayChannel(-1, ok00, 0);
	gameState = STATE_OPTION_MENU;
}
SDL_Rect MainMenu::get_menu_src_rect(int id, bool isSelected){
	if(isSelected)	id -= MENU_START_;
	SDL_Rect src = menuSrcRects[id];
	return src;
}
void MainMenu::handle_events(SDL_Event &e){
	if(e.type == SDL_KEYDOWN){
		switch (e.key.keysym.sym){
			case SDLK_UP: switch_to_previous_option(); break;
			case SDLK_DOWN: switch_to_next_option(); break;
			case SDLK_RETURN: jump_to_submenu(); break;
		}
	}
}
void MainMenu::render(){
	SDL_BlitSurface(title00, &screenRect, screen, &screenRect);
	apply_surface(0, 0, title02, screen);
	int startY = 384;
	int spacingY = 32;
	for(int i = 5;i < MENU_ITEM_COUNT; i++){
		bool isSelected = (i-5 == currentSelectedIndex);
		SDL_Rect srcRect = get_menu_src_rect(i, isSelected);
		SDL_Rect destRect;
		int xOffset[] = {256, 280, 300, 280, 256};
		destRect.x = 256+xOffset[i-5];
		destRect.y = startY + (i-5)*spacingY;
		SDL_BlitSurface(title01, &srcRect, screen, &destRect);
	}
}