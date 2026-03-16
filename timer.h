#include "SDL.h"
class Timer{
private:
	int startTicks;
	int pausedTicks;
	bool paused;//not stopped
	bool started;
public:
	Timer();
	void start();
	void stop();
	void pause();
	void unpause();
	int get_ticks();//get time from timer
	//check timer's status
	bool is_started();
	bool is_paused();
};