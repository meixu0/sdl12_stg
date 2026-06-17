#pragma once
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_rotozoom.h"
#include <string>
#include <stdexcept>
#include <cstdlib>
#define UI_H
#ifdef UI_H
#define STATE_MENU 0
#define STATE_GAME 1
#define STATE_EXIT 2
static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;
static const int SCREEN_BPP = 32;
extern SDL_Surface *screen;
extern const SDL_Surface* numbersImage[10];
extern const SDL_Surface* uppercaseImage[26];
extern const SDL_Surface* lowercaseImage[26];
extern SDL_Event event;
extern int gameState;
typedef void (*ButtonActionCallback)();
typedef void (*RenderCallback)(SDL_Rect* box, SDL_Surface* dest, int state, std::string title, TTF_Font* messageFont);
bool init();
void apply_surface(int x,int y,SDL_Surface *source,SDL_Surface *destination);
void apply_surface_mirror(int x, int y, SDL_Surface* source, SDL_Surface* destination);
//button class
class Button{
private:
	SDL_Rect box;//button profile;
	int state;//0default,1hover,2pressed
	ButtonActionCallback mOnHover;
	ButtonActionCallback mOnClick;
	RenderCallback mOnRender;
	std::string title;
public:
	TTF_Font* messageFont;
	Button(int x, int y, int w, int h, RenderCallback render, ButtonActionCallback hover, ButtonActionCallback click, std::string buttonTitle, TTF_Font* font);
	void handle_events(SDL_Event &e);
	void show(); //show button in window
};
void draw_line(SDL_Surface* dest, int x1,int y1, int x2, int y2, Uint32 color);
void render_win98(SDL_Rect* box, SDL_Surface* dest, int state, std::string buttonTitle, TTF_Font* font);
SDL_Surface *load_image(std::string filename, double targetW, double targetH);
SDL_Surface *load_sprite(std::string filename, int srcX, int srcY, int srcW, int srcH, double targetW, double targetH);
SDL_Surface *rotate_image(SDL_Surface* src, double degrees);
bool load_font(std::string fontfile);
TTF_Font* load_font(std::string filename, int fontsize);
#endif