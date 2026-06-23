#include "StartMenu.h"
SDL_Surface* StartMenu::pl00 = NULL;
SDL_Surface* StartMenu::pl01 = NULL;
SDL_Surface* StartMenu::pl02 = NULL;
Mix_Chunk* StartMenu::select00 = NULL;
Mix_Chunk* StartMenu::ok00 = NULL;
StartMenu::StartMenu(): currentPlayerIndex(0){
    if(pl00 == NULL)    pl00 = load_image("res/title/sl_pl00.png", 800.0, 600.0);
    if(pl01 == NULL)    pl00 = load_image("res/title/sl_pl01.png", 800.0, 600.0);
    if(pl02 == NULL)    pl00 = load_image("res/title/sl_pl02.png", 800.0, 600.0);
    if(select00 == NULL)	select00 = Mix_LoadWAV("res/sound/se_select00.wav");
	if(ok00 == NULL)	ok00 = Mix_LoadWAV("res/sound/se_ok00.wav");
}