#include "ResultDisplay.h"
#include <cstdio>
#include <cstring>

SDL_Surface* ResultDisplay::bgImage      = NULL;
SDL_Surface* ResultDisplay::asciiSheet   = NULL;
SDL_Surface* ResultDisplay::asciiSelSheet = NULL;
Mix_Chunk*   ResultDisplay::select00     = NULL;
Mix_Chunk*   ResultDisplay::ok00         = NULL;
Mix_Chunk*   ResultDisplay::cancel00     = NULL;

ResultDisplay::ResultDisplay() : records_(NULL), recordCount_(0), currentPage_(0), selCol_(0), highScore_(0) {
    if(bgImage == NULL)
        bgImage = load_image("res/title/select00.jpg", 800, 600);
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
    load_records();
}

ResultDisplay::~ResultDisplay() {
    if(records_) { cJSON_Delete(records_); records_ = NULL; }
}

SDL_Rect ResultDisplay::get_char_src(char c) const {
    SDL_Rect r = {0, 0, 0, 0}; /* 默认不绘制 */
    if(c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    if(c >= '0' && c <= '9'){ r.x = (c - '0') * 16; r.y = 48; r.w = 16; r.h = 16; }
    else if(c >= 'A' && c <= 'O'){ r.x = 16 + (c - 'A') * 16; r.y = 64; r.w = 16; r.h = 16; }
    else if(c >= 'P' && c <= 'Z'){ r.x = (c - 'P') * 16; r.y = 80; r.w = 16; r.h = 16; }
    return r;
}

void ResultDisplay::draw_string(const char* s, int x, int y, bool selected){
    SDL_Surface* sheet = selected ? asciiSelSheet : asciiSheet;
    if(!sheet) return;
    for(int i = 0; s[i]; i++){
        SDL_Rect src = get_char_src(s[i]);
        if(src.w == 0) continue;
        SDL_Rect dst = {(Sint16)(x + i * 16), (Sint16)y, 0, 0};
        SDL_BlitSurface(sheet, &src, screen, &dst);
    }
}

void ResultDisplay::draw_number(int n, int x, int y, bool selected, int minW){
    char buf[16];
    snprintf(buf, sizeof(buf), "%0*d", minW, n);
    draw_string(buf, x, y, selected);
}

void ResultDisplay::load_records(){
    if(records_) { cJSON_Delete(records_); records_ = NULL; }
    recordCount_ = 0;
    highScore_ = 0;

    FILE* f = fopen("record.json", "r");
    if(!f) return;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = new char[sz + 1];
    fread(buf, 1, sz, f); buf[sz] = '\0';
    fclose(f);

    records_ = cJSON_Parse(buf);
    delete[] buf;
    if(!records_ || !cJSON_IsArray(records_)) return;

    recordCount_ = cJSON_GetArraySize(records_);

/* 找最高分 */
    for(int i = 0; i < recordCount_; i++){
        cJSON* r = cJSON_GetArrayItem(records_, i);
        if(!r) continue;
        cJSON* sc = cJSON_GetObjectItem(r, "score");
        if(sc && cJSON_IsNumber(sc)){
            int s = sc->valueint;
            if((Uint32)s > highScore_) highScore_ = (Uint32)s;
        }
    }
}

void ResultDisplay::handle_events(SDL_Event &e){
    if(e.type != SDL_KEYDOWN) return;
    int totalPages = (recordCount_ + 4) / 5; // ceil
    if(totalPages < 1) totalPages = 1;

    switch(e.key.keysym.sym){
        case SDLK_LEFT:
            if(selCol_ > 0){ selCol_--; Mix_PlayChannel(-1, select00, 0); }
            break;
        case SDLK_RIGHT:
            if(selCol_ < 1){ selCol_++; Mix_PlayChannel(-1, select00, 0); }
            break;
        case SDLK_RETURN:
            Mix_PlayChannel(-1, ok00, 0);
            if(selCol_ == 0 && currentPage_ > 0) currentPage_--;
            else if(selCol_ == 1 && currentPage_ < totalPages - 1) currentPage_++;
            break;
        case SDLK_ESCAPE:
            Mix_PlayChannel(-1, cancel00, 0);
/* 设置 highScore 全局 */
            ::highScore = highScore_;
            gameState = STATE_MENU;
            break;
    }
}

void ResultDisplay::render(){
    SDL_BlitSurface(bgImage, NULL, screen, NULL);
    if(!asciiSheet || !asciiSelSheet) return;

    draw_string("RANKING", 64, 40, true);

    if(recordCount_ == 0){
        draw_string("NO RECORDS", 200, 200, false);
    } else {
        int start = currentPage_ * 5;
        int end = start + 5;
        if(end > recordCount_) end = recordCount_;

/* 表头 */
        draw_string("PLAYER",    100, 100, true);
        draw_string("DIFFICULTY",350, 100, true);
        draw_string("SCORE",     550, 100, true);

        for(int i = start; i < end; i++){
            cJSON* r = cJSON_GetArrayItem(records_, i);
            if(!r) continue;
            const char* p = cJSON_GetObjectItem(r, "player") ? cJSON_GetObjectItem(r, "player")->valuestring : "";
            const char* d = cJSON_GetObjectItem(r, "difficulty") ? cJSON_GetObjectItem(r, "difficulty")->valuestring : "";
            int s = cJSON_GetObjectItem(r, "score") ? cJSON_GetObjectItem(r, "score")->valueint : 0;

            int y = 140 + (i - start) * 40;
            draw_string(p, 100, y, false);
            draw_string(d, 350, y, false);
            draw_number(s, 550, y, false, 10);
        }
    }

/* 页码 + 导航 */
    int totalPages = (recordCount_ + 4) / 5;
    if(totalPages < 1) totalPages = 1;
    char pageStr[32];
    snprintf(pageStr, sizeof(pageStr), "PAGE %d/%d", currentPage_ + 1, totalPages);
    draw_string(pageStr, 300, 480, true);

    bool prevSel = (selCol_ == 0);
    bool nextSel = (selCol_ == 1);
    draw_string("P", 200, 530, prevSel);
    draw_string("R", 216, 530, prevSel);
    draw_string("E", 232, 530, prevSel);
    draw_string("V", 248, 530, prevSel);
    draw_string("N", 500, 530, nextSel);
    draw_string("E", 516, 530, nextSel);
    draw_string("X", 532, 530, nextSel);
    draw_string("T", 548, 530, nextSel);
}
