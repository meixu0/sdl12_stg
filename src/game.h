#include <iostream>
#include <string>
#include "timer.h"
#include "UI.h"
#include "main_menu.h"
#include "StartMenu.h"
#include "player.h"
#include "GameBackground.h"
#include "LevelManager.h"
#include "ItemManager.h"
#include "InfoArea.h"
#ifndef GAME_H
#define GAME_H
static const int FRAMES_PER_SECOND = 60;
class Game{
private:
	Uint32 frameCounter;//track fps number
	bool fpsLimited;
	Timer menuFps;
	Timer fps;
	Uint32 menuFrameCounter;
	//FPS update
	Timer menuFpsUpdate;
	MainMenu mainMenu;
	LevelManager* levelManager;
	ItemManager* itemManager;
	InfoArea infoArea;
	int currentStage;
	StageState prevStageState;
	Uint32 midbossEnterFrame;

	void rebuild_managers(EnemyBulletManager* bulletMgr);
protected:
	static Uint32 lastUpdate;
	static SDL_Surface* fpsSurface;
public:
	Game();
	~Game();
	PlayerBulletPool playerBulletPool_;
	Player player1;
	void show_fps(Uint32 currentFps);
	void init_info_area(SDL_Surface* dest);
	void init_game(SDL_Surface* dest);
	void run();
};
#endif