#include "UI.h"
SDL_Surface* screen = NULL;
SDL_Event event;
bool quit = false;
int gameState = STATE_MENU;
bool isFullscreen = false;
int playerType = 0;
int gameDifficulty = 0;
const SDL_Surface* numbersImage[10] = {NULL};
const SDL_Surface* uppercaseImage[26] = {NULL};
const SDL_Surface* lowercaseImage[26] = {NULL};
SDL_Surface* zakoSprites[SPRITE_ROWS][SPRITE_COLS] = {{NULL}};
SDL_Surface* zakoRingSprites[2][2] = {{NULL}};
SDL_Surface* bossSprites[9][12] = {{NULL}};
bool init(){
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1)	return false;
	if(TTF_Init() == -1)	return false;
	// Configure FluidSynth SoundFont for MIDI playback (Linux).
	// Must be called before Mix_OpenAudio so FluidSynth initialises
	// with the correct soundfont. Harmless on macOS/Windows.
	Mix_SetSoundFonts("/usr/share/soundfonts/FluidR3_GM.sf2");
	if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)	return false;
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	if(screen == NULL)	return false;
	SDL_WM_SetCaption("test", NULL);
	return true;
}
void toggle_fullscreen(){
	isFullscreen = !isFullscreen;
	int flags = SDL_SWSURFACE;
	if(isFullscreen)	flags |= SDL_FULLSCREEN;
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, flags);
	if(screen == NULL)	isFullscreen = !isFullscreen;
}
void apply_surface(int x,int y, SDL_Surface *source, SDL_Surface *destination){
	if (source == NULL || destination == NULL) return;
	SDL_Rect offset;
	offset.x = x;
	offset.y = y;
	SDL_BlitSurface(source, NULL, destination, &offset);
}
void apply_surface_mirror(int x, int y, SDL_Surface* source, SDL_Surface* destination){
	if (source == NULL || destination == NULL) return;
	SDL_Surface* flipped = rotozoomSurfaceXY(source, 0.0, -1.0, 1.0, 0);
	if (flipped == NULL) return;
	if (source->flags & SDL_SRCCOLORKEY)
		SDL_SetColorKey(flipped, SDL_SRCCOLORKEY, source->format->colorkey);
	SDL_Rect offset;
	offset.x = (Sint16)(x - flipped->w);
	offset.y = (Sint16)y;
	SDL_BlitSurface(flipped, NULL, destination, &offset);
	SDL_FreeSurface(flipped);
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
	int x = 0, y = 0;
	if(e.type == SDL_MOUSEMOTION){
		x = e.motion.x;
		y = e.motion.y;
	}else{
		x = e.button.x;
		y = e.button.y;
	}
	bool isInside = ((x >= box.x) && (x <= box.x + box.w) && (y >= box.y) && (y <= box.y + box.h));
	if(!isInside){
		state = 0;
	}else{
		if(e.type == SDL_MOUSEMOTION){
			state = 1;
			if(mOnHover != NULL)	mOnHover();
		}else if(e.type == SDL_MOUSEBUTTONDOWN){
			state = 2;
		}else if(e.type == SDL_MOUSEBUTTONUP && state == 2){
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
		optimizedImage = SDL_DisplayFormatAlpha(loadedImage);
		SDL_FreeSurface(loadedImage);
	}
	if(optimizedImage == NULL)	return NULL;
	double scaleW = targetW / optimizedImage->w;
	double scaleH = targetH / optimizedImage->h;
	double scaleRate = (scaleW < scaleH) ? scaleW : scaleH;
	SDL_Surface* zoomedImage = rotozoomSurface(optimizedImage, 0.0, scaleRate, 0);
	SDL_FreeSurface(optimizedImage);
	return zoomedImage;
}
SDL_Surface *load_sprite(std::string filename, int srcX, int srcY, int srcW, int srcH, double targetW, double targetH){
	SDL_Surface* loadedImage = IMG_Load(filename.c_str());
	if(loadedImage == NULL)	return NULL;
	SDL_Surface* clippedImage = SDL_CreateRGBSurface(SDL_SWSURFACE,
		srcW, srcH,
		loadedImage->format->BitsPerPixel,
		loadedImage->format->Rmask,
		loadedImage->format->Gmask,
		loadedImage->format->Bmask,
		loadedImage->format->Amask);
	if(clippedImage == NULL){
		SDL_FreeSurface(loadedImage);
		return NULL;
	}
	SDL_FillRect(clippedImage, NULL, SDL_MapRGB(clippedImage->format, 0, 0, 0));
	SDL_Rect clip = {(Sint16)srcX, (Sint16)srcY, (Uint16)srcW, (Uint16)srcH};
	SDL_BlitSurface(loadedImage, &clip, clippedImage, NULL);
	SDL_FreeSurface(loadedImage);

	// Save the per-pixel alpha from clippedImage *before* SDL_DisplayFormat
	// strips it.  This is the original transparency from the PNG – it tells
	// us exactly which pixels are background vs. sprite, regardless of RGB.
	int clipW = clippedImage->w, clipH = clippedImage->h;
	Uint8 *alpha = new Uint8[clipW * clipH];
	{
		Uint8  ashift = clippedImage->format->Ashift;
		Uint32 amask  = clippedImage->format->Amask;
		if (SDL_MUSTLOCK(clippedImage)) SDL_LockSurface(clippedImage);
		const Uint32 *cp = (const Uint32 *)clippedImage->pixels;
		for (int i = 0; i < clipW * clipH; i++)
			alpha[i] = (Uint8)((cp[i] & amask) >> ashift);
		if (SDL_MUSTLOCK(clippedImage)) SDL_UnlockSurface(clippedImage);
	}

	SDL_Surface* formattedImage = SDL_DisplayFormat(clippedImage);
	SDL_FreeSurface(clippedImage);
	if(formattedImage == NULL)	{ delete[] alpha; return NULL; }

	double scaleW = targetW / formattedImage->w;
	double scaleH = targetH / formattedImage->h;
	double scaleRate = (scaleW < scaleH) ? scaleW : scaleH;
	SDL_Surface* zoomedImage = rotozoomSurface(formattedImage, 0.0, scaleRate, 0);
	SDL_FreeSurface(formattedImage);
	if(zoomedImage == NULL)	{ delete[] alpha; return NULL; }

	// Use the saved alpha mask to set genuinely-transparent pixels to the
	// colour key, then erode one pixel at the sprite boundary to remove
	// the dark-outline halo that binary colour-keying cannot smooth.
	{
		Uint32 used = zoomedImage->format->Rmask
		            | zoomedImage->format->Gmask
		            | zoomedImage->format->Bmask;
		Uint32 ck = SDL_MapRGB(zoomedImage->format, 0, 0, 0) | ~used;

		if (SDL_MUSTLOCK(zoomedImage)) SDL_LockSurface(zoomedImage);
		Uint32 *zp = (Uint32 *)zoomedImage->pixels;
		int zw = zoomedImage->w, zh = zoomedImage->h;

		// Pass 1: apply PNG alpha mask (alpha < 128 → background).
		for (int zy = 0; zy < zh; zy++) {
			int cy = (int)(zy / scaleRate);
			if (cy >= clipH) cy = clipH - 1;
			for (int zx = 0; zx < zw; zx++) {
				int cx = (int)(zx / scaleRate);
				if (cx >= clipW) cx = clipW - 1;
				if (alpha[cy * clipW + cx] < 128)
					zp[zy * zw + zx] = ck;
			}
		}

		// Pass 2: erode dark pixels at sprite boundaries.
		// Run twice with a generous threshold (RGB < 80) to eat
		// through the dark outlines that Touhou-style sprites
		// typically have.  Only pixels bordering a transparent
		// neighbour are affected, so interior details stay intact.
		for (int pass = 0; pass < 2; pass++) {
			for (int zy = 0; zy < zh; zy++) {
				for (int zx = 0; zx < zw; zx++) {
					Uint32 p = zp[zy * zw + zx];
					if (p == ck) continue;
					Uint8 r, g, b;
					SDL_GetRGB(p, zoomedImage->format, &r, &g, &b);
					if (r >= 80 || g >= 80 || b >= 80) continue;
					if ((zx > 0      && zp[zy * zw + (zx-1)] == ck) ||
					    (zx < zw - 1 && zp[zy * zw + (zx+1)] == ck) ||
					    (zy > 0      && zp[(zy-1) * zw + zx] == ck) ||
					    (zy < zh - 1 && zp[(zy+1) * zw + zx] == ck))
						zp[zy * zw + zx] = ck;
				}
			}
		}

		if (SDL_MUSTLOCK(zoomedImage)) SDL_UnlockSurface(zoomedImage);

		zoomedImage->flags &= ~SDL_SRCALPHA;
		SDL_SetColorKey(zoomedImage, SDL_SRCCOLORKEY, ck);
	}
	delete[] alpha;
	return zoomedImage;
}
SDL_Surface* rotate_image(SDL_Surface* src, double degrees){
	if (src == NULL) return NULL;
	SDL_Surface* rotated = rotozoomSurface(src, degrees, 1.0, 0);
	if (rotated != NULL && (src->flags & SDL_SRCCOLORKEY))
		SDL_SetColorKey(rotated, SDL_SRCCOLORKEY, src->format->colorkey);
	return rotated;
}

TTF_Font* load_font(std::string filename, int fontsize){
	TTF_Font* font = NULL;
	font = TTF_OpenFont(filename.c_str(), fontsize);
	return font;
}
