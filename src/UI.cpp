#include "UI.h"
#include <cmath>
SDL_Surface* screen = NULL;
SDL_Event event;
bool quit = false;
int gameState = STATE_MENU;
bool isFullscreen = false;
int playerType = 0;
int gameDifficulty = 0;
bool godMode = false;
int playerLives = 3;
int playerBombs = 3;
Uint32 highScore = 0;
const SDL_Surface* numbersImage[10] = {NULL};
SDL_Surface* asciiSheet = NULL;
SDL_Rect digitRects[10] = {
    {  0, 48, 16, 16},
    { 16, 48, 16, 16},
    { 32, 48, 16, 16},
    { 48, 48, 16, 16},
    { 64, 48, 16, 16},
    { 80, 48, 16, 16},
    { 96, 48, 16, 16},
    {112, 48, 16, 16},
    {128, 48, 16, 16},
    {144, 48, 16, 16},
};
const SDL_Surface* uppercaseImage[26] = {NULL};
const SDL_Surface* lowercaseImage[26] = {NULL};
SDL_Surface* zakoSprites[SPRITE_ROWS][SPRITE_COLS] = {{NULL}};
SDL_Surface* zakoRingSprites[2][2] = {{NULL}};
SDL_Surface* bossSprites[14][12] = {{NULL}};
bool init(){
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1)	return false;
	if(TTF_Init() == -1)	return false;
#ifndef _WIN32
	Mix_SetSoundFonts("/usr/share/soundfonts/FluidR3_GM.sf2");//only on linux
#endif
	if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)	return false;
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_HWSURFACE);
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
SDL_Surface* rotate_image(SDL_Surface* src, double degrees){
	if (src == NULL) return NULL;
	SDL_Surface* rotated = rotozoomSurface(src, degrees, 1.0, 0);
	if (rotated != NULL && (src->flags & SDL_SRCCOLORKEY))
		SDL_SetColorKey(rotated, SDL_SRCCOLORKEY, src->format->colorkey);
	return rotated;
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
/* strips it.  This is the original transparency from the PNG – it tells */
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

/* Pass 1: apply PNG alpha mask (alpha < 128 → background). */
		for (int zy1 = 0; zy1 < zh; zy1++) {
			int cy = (int)(zy1 / scaleRate);
			if (cy >= clipH) cy = clipH - 1;
			for (int zx = 0; zx < zw; zx++) {
				int cx = (int)(zx / scaleRate);
				if (cx >= clipW) cx = clipW - 1;
				if (alpha[cy * clipW + cx] < 128)
					zp[zy1 * zw + zx] = ck;
			}
		}

		// Pass 2: erode dark pixels at sprite boundaries.
		// Run twice with a generous threshold (RGB < 80) to eat
		// through the dark outlines that Touhou-style sprites
		// typically have.  Only pixels bordering a transparent
		// neighbour are affected, so interior details stay intact.
		for (int pass = 0; pass < 2; pass++) {
			for (int zy2 = 0; zy2 < zh; zy2++) {
				for (int zx = 0; zx < zw; zx++) {
					Uint32 p = zp[zy2 * zw + zx];
					if (p == ck) continue;
					Uint8 r, g, b;
					SDL_GetRGB(p, zoomedImage->format, &r, &g, &b);
					if (r >= 80 || g >= 80 || b >= 80) continue;
					if ((zx > 0      && zp[zy2 * zw + (zx-1)] == ck) ||
					    (zx < zw - 1 && zp[zy2 * zw + (zx+1)] == ck) ||
					    (zy2 > 0      && zp[(zy2-1) * zw + zx] == ck) ||
					    (zy2 < zh - 1 && zp[(zy2+1) * zw + zx] == ck))
						zp[zy2 * zw + zx] = ck;
				}
			}
		}

/* Pass 3: boundary erosion – bottom row and right column have */
		// no neighbour below/right to trigger normal erosion, so dark
		// pixels on those edges would survive as visible lines.
		// Sprites don't typically extend to the exact edge, so treat
		// dark edge pixels as background.
		for (int zx = 0; zx < zw; zx++) {
			Uint32 p = zp[(zh-1) * zw + zx];       // bottom row
			if (p == ck) continue;
			Uint8 r, g, b;
			SDL_GetRGB(p, zoomedImage->format, &r, &g, &b);
			if (r < 80 && g < 80 && b < 80)
				zp[(zh-1) * zw + zx] = ck;
		}
		for (int zy3 = 0; zy3 < zh; zy3++) {
			Uint32 p = zp[zy3 * zw + (zw-1)];       // right column
			if (p == ck) continue;
			Uint8 r, g, b;
			SDL_GetRGB(p, zoomedImage->format, &r, &g, &b);
			if (r < 80 && g < 80 && b < 80)
				zp[zy3 * zw + (zw-1)] = ck;
		}

		if (SDL_MUSTLOCK(zoomedImage)) SDL_UnlockSurface(zoomedImage);

		zoomedImage->flags &= ~SDL_SRCALPHA;
		SDL_SetColorKey(zoomedImage, SDL_SRCCOLORKEY, ck);
	}
	delete[] alpha;
	return zoomedImage;
}

/* 逐像素旋转 90° 整数倍。turns: 1=90°CW, 2=180°, 3=270°CW(-90°) */
SDL_Surface* rotate_90(SDL_Surface* src, int turns){
	turns = turns % 4;
	if(turns == 0) return src;
	int dstW = (turns % 2) ? src->h : src->w;
	int dstH = (turns % 2) ? src->w : src->h;
	SDL_Surface* dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstW, dstH, 32,
		0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if(dst == NULL) return NULL;
	int bpp = src->format->BytesPerPixel;
	if(SDL_MUSTLOCK(src)) SDL_LockSurface(src);
	if(SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);
	for(int sy = 0; sy < src->h; sy++){
		Uint8* sr = (Uint8*)src->pixels + sy * src->pitch;
		for(int sx = 0; sx < src->w; sx++){
			int dx, dy;
			switch(turns){
				case 1: dx = src->h - 1 - sy; dy = sx; break;
				case 2: dx = src->w - 1 - sx; dy = src->h - 1 - sy; break;
				default: dx = sy; dy = src->w - 1 - sx; break;
			}
			Uint8* dr = (Uint8*)dst->pixels + dy * dst->pitch;
			for(int b = 0; b < bpp; b++)
				dr[dx * bpp + b] = sr[sx * bpp + b];
		}
	}
	if(SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);
	if(SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);
	return dst;
}

/* 逐像素旋转任意角度（nearest-neighbor），无插值无黑边 */
SDL_Surface* rotate_nearest(SDL_Surface* src, double degrees){
	double rad = degrees * 3.14159265 / 180.0;
	double ca = cos(rad), sa = sin(rad);
	double aca = fabs(ca), asa = fabs(sa);
	int dstW = (int)(src->w * aca + src->h * asa + 1);
	int dstH = (int)(src->w * asa + src->h * aca + 1);
	SDL_Surface* dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstW, dstH, 32,
		0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if(dst == NULL) return NULL;
	if(SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);
	Uint32* dp = (Uint32*)dst->pixels;
	for(int i = 0; i < dstW * dstH; i++) dp[i] = 0;
	if(SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);

	int bpp = src->format->BytesPerPixel;
	double scx = src->w * 0.5, scy = src->h * 0.5;
	double dcx = dstW * 0.5, dcy = dstH * 0.5;
	if(SDL_MUSTLOCK(src)) SDL_LockSurface(src);
	if(SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);
	for(int dy = 0; dy < dstH; dy++){
		Uint8* dr = (Uint8*)dst->pixels + dy * dst->pitch;
		for(int dx = 0; dx < dstW; dx++){
			double fx = (dx - dcx) * ca + (dy - dcy) * sa + scx;
			double fy = -(dx - dcx) * sa + (dy - dcy) * ca + scy;
			int sx = (int)(fx + 0.5);
			int sy = (int)(fy + 0.5);
			if(sx >= 0 && sx < src->w && sy >= 0 && sy < src->h){
				Uint8* sr = (Uint8*)src->pixels + sy * src->pitch;
				for(int b = 0; b < bpp; b++)
					dr[dx * bpp + b] = sr[sx * bpp + b];
			}
		}
	}
	if(SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);
	if(SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);
	return dst;
}

/* Mirror surface horizontally */
SDL_Surface* mirror_surface(SDL_Surface* source){
	SDL_Surface* dst = SDL_CreateRGBSurface(SDL_SWSURFACE,
		source->w, source->h,
		source->format->BitsPerPixel,
		source->format->Rmask, source->format->Gmask,
		source->format->Bmask, source->format->Amask);
	if(dst == NULL) return NULL;
	int bpp = source->format->BytesPerPixel;
	if(SDL_MUSTLOCK(source)) SDL_LockSurface(source);
	if(SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);
	for(int y = 0; y < source->h; y++){
		Uint8* sr = (Uint8*)source->pixels + y * source->pitch;
		Uint8* dr = (Uint8*)dst->pixels + y * dst->pitch;
		for(int x = 0; x < source->w; x++)
			for(int b = 0; b < bpp; b++)
				dr[(source->w - 1 - x) * bpp + b] = sr[x * bpp + b];
	}
	if(SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);
	if(SDL_MUSTLOCK(source)) SDL_UnlockSurface(source);
	return dst;
}

/* 用未选中 ASCII 样式绘制数字 */
void draw_ascii_number(int n, int x, int y, int minW){
	if(!asciiSheet) return;
	char buf[16];
	snprintf(buf, sizeof(buf), "%0*d", minW, n);
	for(int i = 0; buf[i]; i++){
		int d = buf[i] - '0';
		if(d < 0 || d > 9) continue;
		SDL_Rect dst = {(Sint16)(x + i * 16), (Sint16)y, 0, 0};
		SDL_BlitSurface(asciiSheet, &digitRects[d], screen, &dst);
	}
}

TTF_Font* load_font(std::string filename, int fontsize){
	TTF_Font* font = NULL;
	font = TTF_OpenFont(filename.c_str(), fontsize);
	return font;
}
