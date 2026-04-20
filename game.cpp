#include "game.h"
Uint32 Game::lastUpdate = 0;
TTF_Font* Game::fpsFont = NULL;
SDL_Surface* Game::fpsSurface = NULL;
Game::Game(){
	frameCounter = 0;
	fpsLimited = true;
	menuFrameCounter = 0;
	fpsSurface = NULL;
	lastUpdate = 0;
	fpsFont = load_font("res/FSEX300.ttf", 16);
}
void Game::show_fps(Uint32 current_fps){
	/*
	Uint32 now = SDL_GetTicks();
	if(fpsSurface)	SDL_FreeSurface(fpsSurface);
	char fps_text[16];
	sprintf(fps_text, "FPS: %u", current_fps);
	SDL_Color yellow = {255, 255, 0};
	fpsSurface = TTF_RenderText_Blended(fpsFont, fps_text, yellow);
	lastUpdate = now;
	*/
}
void Game::init_info_area(SDL_Surface* dest){
	SDL_Rect infoArea;
	infoArea.x = 544;
	infoArea.y = 0;
	infoArea.w = 256;
	infoArea.h = 600;
	Uint32 white = SDL_MapRGB(dest->format, 255, 255, 255);
	SDL_FillRect(dest, &infoArea, white);
}
void Game::init_game(SDL_Surface* dest){
	SDL_Rect wholeScreen;
	wholeScreen.x = 0;
	wholeScreen.y = 0;
	wholeScreen.w = 800;
	wholeScreen.h = 600;
	Uint32 black = SDL_MapRGB(dest->format, 0, 0, 0);
	SDL_FillRect(dest, &wholeScreen, black);
	player1.init_player_bullet_pool(&playerBulletPool_);
}
void Game::run(){
	lastUpdate = 0;
	std::cout << "game run" << std::endl;
	if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error");
	std::cout << "window init ok" << std::endl;
	while(gameState == STATE_MENU){
		menuFps.start();
		menuFpsUpdate.start();
		while(SDL_PollEvent(&event)){
			mainMenu.handle_events(event);
			if(event.type == SDL_QUIT)	gameState = STATE_EXIT;
		}
		mainMenu.show();
		if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error in main loop");
	    menuFrameCounter++;
		Uint32 currentFps = menuFrameCounter / (menuFps.get_ticks() / 1000);
		//fps display start
		/*update_fps(currentFps);
		if(fpsSurface){
			SDL_Rect dest = {800 - fpsSurface->w - 15, 10, 0, 0};
			SDL_BlitSurface(fpsSurface, NULL, screen, &dest);
		}*/
		//fps display end
		
		if((fpsLimited == true) && (menuFps.get_ticks() < 1000 / FRAMES_PER_SECOND)){
			SDL_Delay((1000 / FRAMES_PER_SECOND) - menuFps.get_ticks());
		}
	}
	lastUpdate = 0;
	init_game(screen);
	init_info_area(screen);
	static const int PLAY_AREA_WIDTH = 544;
	static const int PLAY_AREA_HEIGHT = 600;
	SDL_Rect playArea = {0, 0, PLAY_AREA_WIDTH, PLAY_AREA_HEIGHT};
	while(gameState == STATE_GAME){
		fps.start();
		while(SDL_PollEvent(&event)){
			player1.handle_input(event);
			if(event.type == SDL_QUIT)	gameState = STATE_EXIT;
		}
		player1.player_move();
		player1.update_simple_shoot();
		playerBulletPool_.update();
		SDL_FillRect(screen, &playArea, SDL_MapRGB(screen->format, 0, 0, 0));
		player1.show();
		playerBulletPool_.render();
		if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error in game loop");
		if((fpsLimited == true) && (fps.get_ticks() < 1000 / FRAMES_PER_SECOND)){
			SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
		}
		frameCounter++;
	}
}
