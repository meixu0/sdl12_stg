#include "game.h"
Uint32 Game::lastUpdate = 0;
//TTF_Font* Game::fpsFont = NULL;
SDL_Surface* Game::fpsSurface = NULL;
Game::Game() : gameBackground_(nullptr), playerBulletPool_(NULL), player1(NULL) {
	frameCounter = 0;
	fpsLimited = true;
	menuFps = Timer();
	fpsSurface = NULL;
	startMenuFrameCounter = 0;
	difficultyMenuFpsCounter = 0;
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
	delete gameBackground_;
	if(player1 != NULL) {
		delete player1;
		player1 = NULL;
	}
	if(playerBulletPool_ != NULL) {
		delete playerBulletPool_;
		playerBulletPool_ = NULL;
	}
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
	player1->init_player_bullet_pool(playerBulletPool_);
}
void Game::run(){
	while(gameState != STATE_EXIT) {
		Uint32 frameStart = SDL_GetTicks();

		while(SDL_PollEvent(&event)) {
			if(gameState == STATE_MENU)
				mainMenu.handle_events(event);
			else if(gameState == STATE_DIFFICULTY_MENU)
				difficultyMenu.handle_events(event);
			else if(gameState == STATE_START_MENU)
				startMenu.handle_events(event);
			else if(gameState == STATE_GAME || gameState == STATE_BOSS)
				player1->handle_input(event);
			if(event.type == SDL_QUIT) gameState = STATE_EXIT;
			if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11)
				toggle_fullscreen();
		}

		switch(gameState) {
			case STATE_MENU:
				update_menu();
				render_menu();
				break;
			case STATE_DIFFICULTY_MENU:
				update_difficulty_menu();
				render_difficulty_menu();
				break;
			case STATE_START_MENU:
				update_startmenu();
				render_startmenu();
				break;
			case STATE_GAME:
			case STATE_BOSS:
				if(!gameBackground_) {
					start_gameplay();
				}
				{
					Uint32 now = SDL_GetTicks();
					float dt = (now - lastFrameTime_) / 1000.0f;
					if(dt > 1.0f / 15.0f) dt = 1.0f / 30.0f;
					lastFrameTime_ = now;
					update_game(dt);
				}
				render_game();
				break;
		}

		if(fpsLimited) {
			Uint32 elapsed = SDL_GetTicks() - frameStart;
			if(elapsed < 1000 / FRAMES_PER_SECOND)
				SDL_Delay(1000 / FRAMES_PER_SECOND - elapsed);
		}
	}
}

void Game::update_menu(){
	menuFps.start();
	menuFpsUpdate.start();
	menuFrameCounter++;
}
void Game::render_menu(){
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	mainMenu.render();
	if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error in main loop");
}

void Game::update_difficulty_menu(){
	difficultyMenuFps.start();
	difficultyMenuFpsUpdate.start();
	difficultyMenuFpsCounter++;
}
void Game::render_difficulty_menu(){
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	difficultyMenu.render();
	if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error in main loop");
}

void Game::update_startmenu(){
	startMenuFps.start();
	startMenuFpsUpdate.start();
	startMenuFrameCounter++;
}
void Game::render_startmenu(){
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	startMenu.render();
	if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error in main loop");
}

void Game::start_gameplay(){
	if(playerBulletPool_ != NULL) {
		delete playerBulletPool_;
	}
	playerBulletPool_ = new PlayerBulletPool();
	if(player1 != NULL) {
		delete player1;
	}
	player1 = new Player();

	init_game(screen);
	init_info_area(screen);
	gameBackground_ = new GameBackground();
	lastFrameTime_ = SDL_GetTicks();
	gameStartTime_ = SDL_GetTicks();
	frameCounter = 0;
	levelManager->set_bullet_manager(&enemyBulletManager_);
	levelManager->set_bullet_manager_for_all(&enemyBulletManager_);
	levelManager->set_item_manager(itemManager);
	itemManager->set_player(player1);
	player1->set_item_manager(itemManager);
	prevStageState = STAGE_RUNNING;
}

void Game::update_game(float dt){
	gameBackground_->background_update(dt);
	player1->player_move();
	{
		static const float PLAY_CENTER_X = 272.0f;
		static const float PLAY_WIDTH_F  = 544.0f;
		float bestDist = -1.0f;
		float bestEnemyX = 0, bestEnemyY = 0;
		bool found = false;
		int enemyCount = levelManager->get_enemy_count();
		for(int i = 0; i < enemyCount; i++) {
			Enemy* e = levelManager->get_enemy(i);
			if(!e || !e->is_active()) continue;
			float cx = e->get_x() + e->get_hitbox_w() * 0.5f;
			float cy = e->get_y() + e->get_hitbox_h() * 0.5f;
			float dist;
			if(cx < PLAY_CENTER_X)
				dist = cx;
			else
				dist = PLAY_WIDTH_F - cx;
			if(dist > bestDist) {
				bestDist = dist;
				bestEnemyX = cx;
				bestEnemyY = cy;
				found = true;
			}
		}
		if(found)
			player1->set_homing_target(bestEnemyX, bestEnemyY);
		else
			player1->set_homing_target(272, 0);
	}
	player1->update_simple_shoot();
	if(playerBulletPool_ != NULL) playerBulletPool_->update();
	levelManager->update_spellcards(dt);
	PlayerPosition playerPosition = player1->get_player_position();
	StageState currentStageState = levelManager->get_stage_state();
	levelManager->update_all_enemies(playerPosition.x, playerPosition.y, frameCounter, midbossEnterFrame, dt, prevStageState, currentStageState);
	levelManager->move_all_enemies(dt);
	levelManager->attack_all_enemies(dt);
	enemyBulletManager_.update(dt);
	itemManager->update(dt);
	// TH06-style: LevelManager 自动检测并处理关卡过渡
	if (levelManager->auto_transition(frameCounter)) {
		itemManager->clear_all();
		itemManager->update(1.0f / 30.0f);
		StageState st = levelManager->get_stage_state();
		if (st == STAGE_BOSS) {
			gameState = STATE_BOSS;
		} else if (st == STAGE_RUNNING) {
			gameState = STATE_GAME;
		} else if (st == STAGE_ALL_CLEAR) {
			gameState = STATE_GAME;
		}
	}
	prevStageState = levelManager->get_stage_state();
	frameCounter++;
}

void Game::render_game(){
	static const int PLAY_AREA_WIDTH = 544;
	static const int PLAY_AREA_HEIGHT = 600;
	SDL_Rect playArea = {0, 0, PLAY_AREA_WIDTH, PLAY_AREA_HEIGHT};
	SDL_FillRect(screen, &playArea, SDL_MapRGB(screen->format, 0, 0, 0));
	gameBackground_->background_show();
	player1->show();
	if(playerBulletPool_ != NULL) playerBulletPool_->render();
	levelManager->show_all_enemies();
	enemyBulletManager_.render();
	infoArea.update_score(ItemManager::get_score());
	infoArea.update_lives(ItemManager::get_lives());
	infoArea.update_spell(ItemManager::get_bombs());
	infoArea.render();
	itemManager->render();
	if(SDL_Flip(screen) == -1)	throw std::runtime_error("flip error in game loop");
}
