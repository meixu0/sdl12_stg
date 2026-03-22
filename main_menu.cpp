#include "main_menu.h"
#include <iostream>
TTF_Font* mainMenuFont = NULL;
void jump_to_game(){
	std::cout << "jump to game" << std::endl;
	gameState = STATE_GAME;
	static SDL_Surface* gameBackground = NULL;
	/*
	gameBackground = load_image("res/game_background.png", 800, 600);
	if(gameBackground == NULL)	std::cout << "load game background error" << std::endl;
	if(gameBackground != NULL)	apply_surface(0, 0, gameBackground, screen);
	*/
	if(SDL_Flip(screen) == -1)	std::cout << "flip error in jump_to_game" << std::endl;
	//SDL_Delay(10000);
}
MainMenu::MainMenu()
:gameStartButton(500, 200, 200, 100, render_win98, NULL, jump_to_game, "game start", NULL)
{
	if(mainMenuFont == NULL){
		mainMenuFont = load_font("res/main_menu_font.ttf", 20);
	}
	gameStartButton.messageFont = mainMenuFont;
	background = load_image("res/menu_background.png", 800, 600);
}
void MainMenu::handle_events(SDL_Event &e){
	gameStartButton.handle_events(e);
}
void MainMenu::show(){
	apply_surface(0, 0, background, screen);
	gameStartButton.show();
}