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
	screenRect = {0, 0, 800, 600};
	menuSrcRects[0] = {8, 0, 64, 32};
	menuSrcRects[1] = {100, 32, 80, 32};
	menuSrcRects[2] = {196, 32, 148, 32};
	menuSrcRects[3] = {360, 32, 84, 32};
	menuSrcRects[4] = {435, 0, 56, 32};
	menuSrcRects[5] = {8, 64, 64, 32};
	menuSrcRects[6] = {100, 96, 80, 32};
	menuSrcRects[7] = {196, 96, 148, 32};
	menuSrcRects[8] = {360, 96, 84, 32};
	menuSrcRects[9] = {435, 64, 56, 32};

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
	if(currentSelectedIndex == 0)	{Mix_PlayChannel(-1, ok00, 0);gameState = STATE_GAME;}
	if(currentSelectedIndex == 4)	{Mix_PlayChannel(-1, ok00, 0);gameState = STATE_EXIT;}
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