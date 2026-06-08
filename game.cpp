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
	fpsFont = load_font("res/menufont.ttf", 16);
	levelManager.read_stage_data("level/level.json");
	levelManager.init_enemy_pool();
}
void Game::show_fps(Uint32 current_fps){
	//todo: fps monitor
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
	//std::cout << "game run" << std::endl;
	if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error");
	//std::cout << "window init ok" << std::endl;
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
	GameBackground gameBackground;
	Uint32 lastTime = SDL_GetTicks();
	Uint32 gameStartTime = SDL_GetTicks();
	frameCounter = 0;
	EnemyBulletManager enemyBulletManager;
	levelManager.set_bullet_manager_for_all(&enemyBulletManager);
	//todo: fps monitor
	while(gameState == STATE_GAME){
		Uint32 frameStart = SDL_GetTicks();
		fps.start();
		while(SDL_PollEvent(&event)){
			player1.handle_input(event);
			if(event.type == SDL_QUIT)	gameState = STATE_EXIT;
		}
		float dt = (frameStart - lastTime) / 1000.0f;
		if (dt > 1.0f / 15.0f) dt = 1.0f / 30.0f;
		gameBackground.background_update(dt);
		player1.player_move();
		// 子机子弹追踪目标: 左移→最左敌机, 右移→最右敌机
		{
			float bestX = -1;
			bool found = false;
			int enemyCount = levelManager.get_enemy_count();
			for (int i = 0; i < enemyCount; i++) {
				Enemy* e = levelManager.get_enemy(i);
				if (!e || !e->is_active()) continue;
				float ex = e->get_x();
				if (!found) { bestX = ex; found = true; continue; }
				if (player1.get_player_x_vel() < 0 && ex < bestX) bestX = ex;       // 左移→最左
				else if (player1.get_player_x_vel() > 0 && ex > bestX) bestX = ex;  // 右移→最右
			}
			if (found) {
				for (int i = 0; i < enemyCount; i++) {
					Enemy* e = levelManager.get_enemy(i);
					if (!e || !e->is_active()) continue;
					if (e->get_x() == bestX) {
						player1.set_homing_target(e->get_x(), e->get_y());
						break;
					}
				}
			} else {
				player1.set_homing_target(272, 0);
			}
		}
		player1.update_simple_shoot();
		playerBulletPool_.update();
		SDL_FillRect(screen, &playArea, SDL_MapRGB(screen->format, 0, 0, 0));
		gameBackground.background_show();
		player1.show();
		playerBulletPool_.render();
		PlayerPosition playerPosition = player1.get_player_position();
		levelManager.update_all_enemies(playerPosition.x, playerPosition.y, frameCounter);
		levelManager.move_all_enemies(dt);
			levelManager.attack_all_enemies(dt);
		enemyBulletManager.update(dt);
		levelManager.show_all_enemies();
		enemyBulletManager.render();
		if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error in game loop");
		if((fpsLimited == true)){
			Uint32 currentFrameTicks = SDL_GetTicks();
			Uint32 targetTicks = gameStartTime + ((frameCounter + 1) * 1000) / FRAMES_PER_SECOND;
			if(currentFrameTicks < targetTicks)	SDL_Delay(targetTicks - currentFrameTicks);
		}
		lastTime = frameStart;
		frameCounter++;
	}
}
