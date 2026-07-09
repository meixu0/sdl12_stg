#include "MusicRoom.h"
#include <cstdio>

SDL_Surface* MusicRoom::bgImage      = NULL;
SDL_Surface* MusicRoom::asciiSheet   = NULL;
SDL_Surface* MusicRoom::asciiSelSheet = NULL;
Mix_Music*   MusicRoom::currentBgm   = NULL;
Mix_Chunk*   MusicRoom::select00     = NULL;
Mix_Chunk*   MusicRoom::ok00         = NULL;
Mix_Chunk*   MusicRoom::cancel00     = NULL;

MusicRoom::MusicRoom() : selRow_(0), selCol_(0) {
    if(bgImage == NULL)
        bgImage = load_image("res/result.jpg", 800, 600);
    if(asciiSheet == NULL)
        asciiSheet = IMG_Load("res/ascii/ascii.png");
    if(asciiSelSheet == NULL)
        asciiSelSheet = IMG_Load("res/ascii/asciis.png");
    if(select00 == NULL)
        select00 = Mix_LoadWAV("res/sound/se_select00.wav");
    if(ok00 == NULL)
        ok00 = Mix_LoadWAV("res/sound/se_ok00.wav");
    if(cancel00 == NULL)
        cancel00 = Mix_LoadWAV("res/sound/se_cancel00.wav");
}

SDL_Rect MusicRoom::get_char_src(char c) const {
    SDL_Rect r = {0, 0, 0, 0};
    if(c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    if(c >= '0' && c <= '9')      { r.x = (c - '0') * 16; r.y = 48; r.w = 16; r.h = 16; }
    else if(c >= 'A' && c <= 'O') { r.x = 16 + (c - 'A') * 16; r.y = 64; r.w = 16; r.h = 16; }
    else if(c >= 'P' && c <= 'Z') { r.x = (c - 'P') * 16; r.y = 80; r.w = 16; r.h = 16; }
    return r;
}

void MusicRoom::draw_string(const char* s, int x, int y, bool selected){
    SDL_Surface* sheet = selected ? asciiSelSheet : asciiSheet;
    if(!sheet) return;
    for(int i = 0; s[i]; i++){
        SDL_Rect src = get_char_src(s[i]);
        if(src.w == 0) continue;
        SDL_Rect dst = {(Sint16)(x + i * 16), (Sint16)y, 0, 0};
        SDL_BlitSurface(sheet, &src, screen, &dst);
    }
}

void MusicRoom::draw_number(int n, int x, int y, bool selected, int minW){
    char buf[16];
    snprintf(buf, sizeof(buf), "%0*d", minW, n);
    draw_string(buf, x, y, selected);
}

void MusicRoom::play_track(int n){
    Mix_HaltMusic();
    if(currentBgm){ Mix_FreeMusic(currentBgm); currentBgm = NULL; }
    char path[64];
    snprintf(path, sizeof(path), "res/music/th07_%02d.mid", n);
    currentBgm = Mix_LoadMUS(path);
    if(currentBgm) Mix_PlayMusic(currentBgm, -1);
}

void MusicRoom::handle_events(SDL_Event &e){
    if(e.type != SDL_KEYDOWN) return;
    switch(e.key.keysym.sym){
        case SDLK_UP:
            if(selRow_ > 0) { selRow_--; Mix_PlayChannel(-1, select00, 0); }
            break;
        case SDLK_DOWN:
            if(selRow_ < 4) { selRow_++; Mix_PlayChannel(-1, select00, 0); }
            break;
        case SDLK_LEFT:
            if(selCol_ > 0) { selCol_--; Mix_PlayChannel(-1, select00, 0); }
            break;
        case SDLK_RIGHT:
            if(selCol_ < 3) { selCol_++; Mix_PlayChannel(-1, select00, 0); }
            break;
        case SDLK_RETURN:
            Mix_PlayChannel(-1, ok00, 0);
            play_track(selRow_ * 4 + selCol_ + 1);
            break;
        case SDLK_ESCAPE:
            Mix_PlayChannel(-1, cancel00, 0);
            Mix_HaltMusic();
            if(currentBgm){ Mix_FreeMusic(currentBgm); currentBgm = NULL; }
            currentBgm = Mix_LoadMUS("res/music/th07_01.mid");
            if(currentBgm) Mix_PlayMusic(currentBgm, -1);
            gameState = STATE_MENU;
            break;
    }
}

void MusicRoom::render(){
    SDL_BlitSurface(bgImage, NULL, screen, NULL);
    if(!asciiSheet || !asciiSelSheet) return;

    draw_string("MUSIC ROOM", 300, 40, true);

    for(int r = 0; r < 5; r++){
        for(int c = 0; c < 4; c++){
            int idx = r * 4 + c;
            int x = 150 + c * 130;
            int y = 120 + r * 80;
            bool sel = (r == selRow_ && c == selCol_);
            draw_number(idx + 1, x, y, sel, 2);
        }
    }
}
