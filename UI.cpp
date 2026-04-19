#include "UI.h"
SDL_Surface* screen = NULL;
SDL_Event event;
bool quit = false;
int gameState = STATE_MENU;
bool init(){
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1)	return false;
	if(TTF_Init() == -1)	return false;
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	if(screen == NULL)	return false;
	SDL_WM_SetCaption("test", NULL);
	return true;
}
void apply_surface(int x,int y, SDL_Surface *source, SDL_Surface *destination){
	SDL_Rect offset;
	offset.x = x;
	offset.y = y;
	SDL_BlitSurface(source, NULL, destination, &offset);
}
Button::Button(int x, int y, int w, int h, RenderCallback render, ButtonActionCallback hover, ButtonActionCallback click, std::string buttonTitle, TTF_Font* font){
	title = buttonTitle;
	messageFont = font;
	box.x = x;
	box.y = y;
	box.w = w;
	box.h = h;
	mOnHover = hover;
	mOnClick = click;
	mOnRender = render;
}
void Button::handle_events(SDL_Event &e){
	int x = 0, y = 0;//default mouse coordinate
	//if mouse move occurs
	if(e.type == SDL_MOUSEMOTION){
		x = e.motion.x;
		y = e.motion.y;
	}else{
		//get the coordinate been pressed
		x = e.button.x;
		y = e.button.y;
	}
	bool isInside = ((x >= box.x) && (x <= box.x + box.w) && (y >= box.y) && (y <= box.y + box.h));
	if(!isInside){
		state = 0;
	}else{
		if(e.type == SDL_MOUSEMOTION){//if a mouse button is pressed
			state = 1;
			if(mOnHover != NULL)	mOnHover();//triggle hover behavior
		}else if(e.type == SDL_MOUSEBUTTONDOWN){//mouse is been pressing
			state = 2;
		}else if(e.type == SDL_MOUSEBUTTONUP && state == 2){//mouse pops up
			state = 1;
			if(mOnClick != NULL)	mOnClick();
		}
	}
}
void Button::show(){
	if(mOnRender != NULL)	mOnRender(&box, screen, state, title, messageFont);
}
void draw_line(SDL_Surface* dest, int x1,int y1, int x2, int y2, Uint32 color){
	SDL_Rect line;
	line.x = x1;
	line.y = y1;
	line.w = x2 - x1 + 1;
	line.h = y2 - y1 + 1;
	SDL_FillRect(dest, &line, color);
}
void render_win98(SDL_Rect* box, SDL_Surface* dest, int state, std::string title, TTF_Font* messageFont){
	Uint32 silver = SDL_MapRGB(dest->format, 192, 192, 192);
	Uint32 white = SDL_MapRGB(dest->format, 255, 255, 255);
	Uint32 gray = SDL_MapRGB(dest->format, 128, 128, 128);
	Uint32 white_ = SDL_MapRGB(dest->format, 0, 0, 0);
	SDL_FillRect(dest, box, silver);
	if(state != 2){
		draw_line(dest, box->x, box->y,box->x + box->w - 1, box->y, white_);
		draw_line(dest, box->x, box->y, box->x, box->y + box->h - 1, white_);
		draw_line(dest, box->x, box->y + box->h - 1, box->x + box->w - 1, box->y + box->h - 1, gray);
		draw_line(dest, box->x + box->w - 1, box->y, box->x + box->w - 1, box->y + box->h - 1, gray);
	}else{
		draw_line(dest, box->x, box->y, box->x + box->w - 1, box->y, gray);
        draw_line(dest, box->x, box->y, box->x, box->y + box->h - 1, gray);
		draw_line(dest, box->x, box->y + box->h - 1, box->x + box->w - 1, box->y + box->h - 1, white_);
        draw_line(dest, box->x + box->w - 1, box->y, box->x + box->w - 1, box->y + box->h - 1, white_);
	}
	if(messageFont != NULL){
		SDL_Surface *message = NULL;
		SDL_Color textColor = {0, 0, 0};
		message = TTF_RenderText_Solid(messageFont, title.c_str(), textColor);
		if(message != NULL){
			//centered display text
			int w, h;
			TTF_SizeText(messageFont, title.c_str(), &w, &h);
			int centeredX = box->x + (box->w - w) / 2;
			int centeredY = box->y + (box->h - h) / 2;
			if(state == 2){
				centeredX += 1;
				centeredY += 1;
			}
			apply_surface(centeredX, centeredY, message, dest);
			SDL_FreeSurface(message);
		}
		
	}
}
SDL_Surface *load_image(std::string filename, double targetW, double targetH){
	SDL_Surface* loadedImage = NULL;
	SDL_Surface* optimizedImage = NULL;
	loadedImage = IMG_Load(filename.c_str());
	if(loadedImage != NULL){
		optimizedImage = SDL_DisplayFormat(loadedImage);
		SDL_FreeSurface(loadedImage);
	}
	if(optimizedImage == NULL)	return NULL;
	double scaleW = targetW / optimizedImage->w;
	double scaleH = targetH / optimizedImage->h;
	double scaleRate = (scaleW < scaleH) ? scaleW : scaleH;
	SDL_Surface* zoomedImage = rotozoomSurface(optimizedImage, 0.0, scaleRate, 1);
	return zoomedImage;
}
TTF_Font* load_font(std::string filename, int fontsize){
	TTF_Font* font = NULL;
	font = TTF_OpenFont(filename.c_str(), fontsize);
	return font;
}