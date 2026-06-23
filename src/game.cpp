#include "game.h"
Uint32 Game::lastUpdate = 0;
//TTF_Font* Game::fpsFont = NULL;
SDL_Surface* Game::fpsSurface = NULL;
Game::Game(){
	frameCounter = 0;
	fpsLimited = true;
	menuFrameCounter = 0;
	fpsSurface = NULL;
	lastUpdate = 0;
	currentStage = 1;
	prevStageState = STAGE_LOADING;
	midbossEnterFrame = 0;

	// load boss sprites: 9 types × 12 frames, 3×4 grid, 48×64 per cell
	for(int i = 5;i < 13;i++){
		std::string filename = "res/stgenm/stgenm" + std::to_string(i) + ".png";
		for(int j = 0;j < 12;j++){
			bossSprites[i][j] = load_sprite(filename, (j % 4) * 48, (j / 4) * 64, 48, 64, 48.0f, 64.0f);
		}
	}

	levelManager = new LevelManager();
	itemManager = new ItemManager();
	for(int i = 0; i < 10; i++)	numbersImage[i] = load_sprite("res/ascii/ascii.png", i * 16, 48, 16, 16, 16.0f, 16.0f);
	for(int i = 0; i < 15; i++)	uppercaseImage[i] = load_sprite("res/ascii/ascii.png", (i+1) * 16, 64, 16, 16, 16.0f, 16.0f);
	for(int i = 0; i < 11; i++)	uppercaseImage[i+15] = load_sprite("res/ascii/ascii.png", i*16, 80, 16, 16, 16.0f, 16.0f);
	for(int i = 0; i < 15; i++)	lowercaseImage[i] = load_sprite("res/ascii/ascii.png", (i+1) * 16, 96, 16, 16, 16.0f, 16.0f);
	for(int i = 0; i < 11; i++)	lowercaseImage[i+15] = load_sprite("res/ascii/ascii.png", i*16, 112, 16, 16, 16.0f, 16.0f);
	levelManager->load_stage(currentStage);
	levelManager->init_enemy_pool();
	levelManager->start_stage();
    for (int row = 0; row < SPRITE_ROWS; row++) {
        if (row >= 4 && row <= 7) continue;
        if (row >= 12) continue;
        for (int col = 0; col < SPRITE_COLS; col++) {
            zakoSprites[row][col] = load_sprite(
                "res/stgenm/enemy.png",
                col * 32, row * 32, 32, 32,
                32.0, 32.0);
        }
    }
    zakoRingSprites[0][0] = load_sprite("res/stgenm/enemy.png", 32, 64, 16, 16, 16.0, 16.0);//blue ring
    zakoRingSprites[0][1] = load_sprite("res/stgenm/enemy.png", 48, 64, 16, 16, 16.0, 16.0);//red ring
    zakoRingSprites[1][0] = load_sprite("res/stgenm/enemy.png", 32, 80, 16, 16, 16.0, 16.0);//green ring
    zakoRingSprites[1][1] = load_sprite("res/stgenm/enemy.png", 48, 80, 16, 16, 16.0, 16.0);//yellow ring
	}
Game::~Game() {
	delete levelManager;
	delete itemManager;
}

void Game::rebuild_managers(EnemyBulletManager* bulletMgr) {
	itemManager->clear_all();
	itemManager->update(1.0f / 30.0f);
	delete levelManager;
	delete itemManager;
	levelManager = new LevelManager();
	itemManager = new ItemManager();
	itemManager->set_player(&player1);
	player1.set_item_manager(itemManager);
	levelManager->set_bullet_manager_for_all(bulletMgr);
}

void Game::init_info_area(SDL_Surface* dest){
	SDL_Rect infoArea_;
	infoArea_.x = 544;
	infoArea_.y = 0;
	infoArea_.w = 256;
	infoArea_.h = 600;
	Uint32 white = SDL_MapRGB(dest->format, 255, 255, 255);
	SDL_FillRect(dest, &infoArea_, white);
	infoArea.render();
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
	while(gameState != STATE_EXIT){
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
				if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11)	toggle_fullscreen();
			}
			SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
			mainMenu.render();
			if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error in main loop");
			menuFrameCounter++;
			Uint32 currentFps = menuFrameCounter / (menuFps.get_ticks() / 1000);
			if((fpsLimited == true) && (menuFps.get_ticks() < 1000 / FRAMES_PER_SECOND)){
				SDL_Delay((1000 / FRAMES_PER_SECOND) - menuFps.get_ticks());
			}
		}
		
		lastUpdate = 0;
		while(gameState == STATE_START_MENU){

		}
		mainMenu.~MainMenu();
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
		levelManager->set_bullet_manager_for_all(&enemyBulletManager);
		itemManager->set_player(&player1);
		player1.set_item_manager(itemManager);
		//todo: fps monitor
		prevStageState = STAGE_RUNNING; // to trigger boss loading if stage starts with midboss
		while(gameState == STATE_GAME || gameState == STATE_BOSS){
			Uint32 frameStart = SDL_GetTicks();
			fps.start();
			while(SDL_PollEvent(&event)){
				player1.handle_input(event);
				if(event.type == SDL_QUIT)	gameState = STATE_EXIT;
				if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11)	toggle_fullscreen();
			}
			float dt = (frameStart - lastTime) / 1000.0f;
			if (dt > 1.0f / 15.0f) dt = 1.0f / 30.0f;
			gameBackground.background_update(dt);
			player1.player_move();
			{
				static const float PLAY_CENTER_X = 272.0f;
				static const float PLAY_WIDTH_F  = 544.0f;
				float bestDist = -1.0f;
				float bestEnemyX = 0, bestEnemyY = 0;
				bool found = false;
				int enemyCount = levelManager->get_enemy_count();
				for (int i = 0; i < enemyCount; i++) {
					Enemy* e = levelManager->get_enemy(i);
					if (!e || !e->is_active()) continue;
					float cx = e->get_x() + e->get_hitbox_w() * 0.5f;
					float cy = e->get_y() + e->get_hitbox_h() * 0.5f;
					float dist;
					if (cx < PLAY_CENTER_X)
						dist = cx;
					else
						dist = PLAY_WIDTH_F - cx;
					if (dist > bestDist) {
						bestDist = dist;
						bestEnemyX = cx;
						bestEnemyY = cy;
						found = true;
					}
				}
				if (found)
					player1.set_homing_target(bestEnemyX, bestEnemyY);
				else
					player1.set_homing_target(272, 0);
			}
			player1.update_simple_shoot();
			playerBulletPool_.update();
			SDL_FillRect(screen, &playArea, SDL_MapRGB(screen->format, 0, 0, 0));
			gameBackground.background_show();
			player1.show();
			playerBulletPool_.render();
			PlayerPosition playerPosition = player1.get_player_position();
			StageState currentStageState = levelManager->get_stage_state();
			levelManager->update_all_enemies(playerPosition.x, playerPosition.y, frameCounter, midbossEnterFrame, dt, prevStageState, currentStageState);
			levelManager->move_all_enemies(dt);
			levelManager->attack_all_enemies(dt);
			enemyBulletManager.update(dt);
			itemManager->update(dt);
			levelManager->show_all_enemies();
			// stage state transition detection
			{
				StageState cur = levelManager->get_stage_state();
				if (prevStageState == STAGE_RUNNING && cur == STAGE_BOSS) {
					gameState = STATE_BOSS;
					rebuild_managers(&enemyBulletManager);
					levelManager->load_boss_stage(currentStage);
					levelManager->init_enemy_pool();
					levelManager->trigger_boss();
				}
				else if (cur == STAGE_CLEAR) {
					rebuild_managers(&enemyBulletManager);
					currentStage++;
					levelManager->load_stage(currentStage);
					levelManager->init_enemy_pool();
					levelManager->start_stage();
					gameState = STATE_GAME;
				}
				prevStageState = cur;
			}
			enemyBulletManager.render();
			infoArea.update_score(ItemManager::get_score());
			infoArea.update_lives(ItemManager::get_lives());
			infoArea.update_spell(ItemManager::get_bombs());
			infoArea.render();
			itemManager->render();
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
	
}
