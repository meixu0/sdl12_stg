#pragma once
#include "UI.h"
#define MAIN_MENU_H
#ifdef MAIN_MENU_H
void jump_to_game();
extern TTF_Font* mainMenuFont;
class MainMenu{
private:
	Button gameStartButton;
	SDL_Surface* background;
public:
	MainMenu();
	void handle_events(SDL_Event &e);
	void show();
};
#endif