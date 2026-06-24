#pragma once
#include "UI.h"
#define MAIN_MENU_H
#ifdef MAIN_MENU_H
class MainMenu{
private:
	enum MainMenuOptionID{
		MENU_START = 0,//selected
		MENU_RESULT,
		MENU_MUSIC_ROOM,
		MENU_OPTION,
		MENU_QUIT,
		MENU_START_,//unselected
		MENU_RESULT_,
		MENU_MUSIC_ROOM_,
		MENU_OPTION_,
		MENU_QUIT_,
		MENU_ITEM_COUNT
	};
	int currentSelectedIndex;
	static SDL_Surface* title00;
	static SDL_Surface* title01;
	static SDL_Surface* title02;
	static SDL_Rect screenRect;
	static SDL_Rect menuSrcRects[10];

	static Mix_Music* menuBGM;
	static Mix_Chunk* select00;
	static Mix_Chunk* ok00;
		
	void switch_to_previous_option();
	void switch_to_next_option();
	void jump_to_submenu();
	SDL_Rect get_menu_src_rect(int id, bool isSelected);
public:
	MainMenu();
	~MainMenu();
	void handle_events(SDL_Event &e);
	void render();
};
#endif