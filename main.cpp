#include "UI.h"
#include "game.h"
#include <iostream>
int main(int argc, char* args[]){
	if(init() == false)	return -1;
	try{
		Game game;
		game.run();
	}catch(const std::exception& err){
		std::cout << err.what() << std::endl;
	}

	SDL_Quit();
	return 0;
}
