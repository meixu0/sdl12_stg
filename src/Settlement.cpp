#include "Settlement.h"
#include "ItemManager.h"
#include "cJSON.h"
#include <cstdio>
#include <cstring>

SDL_Surface* Settlement::bgImage       = NULL;
SDL_Surface* Settlement::asciiSheet    = NULL;
SDL_Surface* Settlement::asciiSelSheet = NULL;
Mix_Music*   Settlement::bgm           = NULL;
Mix_Chunk*   Settlement::select00      = NULL;
Mix_Chunk*   Settlement::ok00          = NULL;

Settlement::Settlement() : nameLen_(0), kbRow_(0), kbCol_(0), kbSection_(0), confirmed_(false) {
    memset(name_, 0, sizeof(name_));
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
}

void Settlement::reset(){
    nameLen_ = 0; kbRow_ = 0; kbCol_ = 0; kbSection_ = 0; confirmed_ = false;
    memset(name_, 0, sizeof(name_));
    // 停止当前 BGM，播放结算 BGM
    Mix_HaltMusic();
    if(bgm == NULL)
        bgm = Mix_LoadMUS("res/music/th07_19.mid");
    if(bgm) Mix_PlayMusic(bgm, -1);
}

SDL_Rect Settlement::get_char_src(char c) const {
    SDL_Rect r = {0, 0, 16, 16};
    if(c >= '0' && c <= '9'){ r.x = (c - '0') * 16; r.y = 48; }
    else if(c >= 'A' && c <= 'O'){ r.x = 16 + (c - 'A') * 16; r.y = 64; }
    else if(c >= 'P' && c <= 'Z'){ r.x = (c - 'P') * 16; r.y = 80; }
    else if(c >= 'a' && c <= 'o'){ r.x = 16 + (c - 'a') * 16; r.y = 128; }
    else if(c >= 'p' && c <= 'z'){ r.x = (c - 'p') * 16; r.y = 144; }
    return r;
}

const char* Settlement::diff_name() const {
    switch(gameDifficulty){
        case 0: return "EASY";     case 1: return "NORMAL";
        case 2: return "HARD";     case 3: return "LUNATIC";
        case 4: return "EXTRA";    default: return "NORMAL";
    }
}

void Settlement::add_letter(){
    if(kbSection_ == 2){
        Mix_PlayChannel(-1, ok00, 0);
        confirmed_ = true;
        save_record();
        Mix_HaltMusic();
        gameState = STATE_EXIT;
        return;
    }
    if(kbSection_ == 1){
        if(nameLen_ < 8) name_[nameLen_++] = '0' + kbCol_;
        return;
    }
    if(kbRow_ == 3 && kbCol_ == 5) return; // DEL
    if(nameLen_ >= 8) return;
    int idx = kbRow_ * 7 + kbCol_;
    if(idx < 26) name_[nameLen_++] = 'A' + idx;
}

void Settlement::del_letter(){
    if(nameLen_ > 0) name_[--nameLen_] = '\0';
}

void Settlement::save_record(){
    cJSON* arr = NULL;
    FILE* f = fopen("record.json", "r");
    if(f){
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* buf = new char[sz + 1];
        fread(buf, 1, sz, f); buf[sz] = '\0';
        fclose(f);
        arr = cJSON_Parse(buf);
        delete[] buf;
    }
    if(!arr) arr = cJSON_CreateArray();

    cJSON* rec = cJSON_CreateObject();
    cJSON_AddStringToObject(rec, "player", name_);
    cJSON_AddStringToObject(rec, "difficulty", diff_name());
    cJSON_AddNumberToObject(rec, "score", ItemManager::get_score());
    cJSON_AddItemToArray(arr, rec);

    char* out = cJSON_Print(arr);
    f = fopen("record.json", "w");
    if(f){ fputs(out, f); fclose(f); }
    free(out);
    cJSON_Delete(arr);
}

void Settlement::draw_string(const char* s, int x, int y, bool selected){
    SDL_Surface* sheet = selected ? asciiSelSheet : asciiSheet;
    if(!sheet) return;
    for(int i = 0; s[i]; i++){
        SDL_Rect src = get_char_src(s[i]);
        SDL_Rect dst = {(Sint16)(x + i * 16), (Sint16)y, 0, 0};
        SDL_BlitSurface(sheet, &src, screen, &dst);
    }
}

void Settlement::draw_number(int n, int x, int y, bool selected, int minW){
    char buf[16];
    snprintf(buf, sizeof(buf), "%0*d", minW, n);
    draw_string(buf, x, y, selected);
}

void Settlement::handle_events(SDL_Event &e){
    if(e.type != SDL_KEYDOWN) return;
    if(confirmed_) return;
    switch(e.key.keysym.sym){
        case SDLK_UP: Mix_PlayChannel(-1, select00, 0);
            if(kbSection_ == 2)      { kbSection_ = 1; kbCol_ = 5; }
            else if(kbSection_ == 1) {
                kbSection_ = 0;
                if(kbCol_ >= 5) kbRow_ = 2;  // T=col5, U=col6
                else kbRow_ = 3;              // V-Z
            }
            else if(kbRow_ > 0) kbRow_--;
            break;
        case SDLK_DOWN: Mix_PlayChannel(-1, select00, 0);
            if(kbSection_ == 0 && (kbRow_ == 2 || kbRow_ == 3)) {
                kbSection_ = 1;  // col stays same, maps letter col→digit col
            }
            else if(kbSection_ == 1) { kbSection_ = 2; }
            else if(kbRow_ < 3) kbRow_++;
            break;
        case SDLK_LEFT: Mix_PlayChannel(-1, select00, 0);
            if(kbSection_ != 2 && kbCol_ > 0) kbCol_--;
            break;
        case SDLK_RIGHT: Mix_PlayChannel(-1, select00, 0);
            if(kbSection_ == 0 && kbCol_ < 6) kbCol_++;
            else if(kbSection_ == 1 && kbCol_ < 9) kbCol_++;
            break;
        case SDLK_RETURN: add_letter(); break;
        case SDLK_BACKSPACE: del_letter(); break;
        case SDLK_ESCAPE: del_letter(); break;
    }
}

void Settlement::render(){
    SDL_BlitSurface(bgImage, NULL, screen, NULL);
    if(!asciiSheet || !asciiSelSheet) return;

    static const int LABEL_X = 64;
    static const int VALUE_X = 280;

    draw_string("PLAYER",     LABEL_X, 80,  true);
    draw_string("DIFFICULTY", LABEL_X, 180, true);
    draw_string("SCORE",      LABEL_X, 260, true);

    draw_string(name_, VALUE_X, 80, true);
    draw_string(diff_name(), VALUE_X, 180, true);
    draw_number(ItemManager::get_score(), VALUE_X, 260, true, 10);

    // 字母键盘 A-Z (7x4)
    int kbX = 100, kbY = 320;
    for(int r = 0; r < 4; r++){
        for(int c = 0; c < 7; c++){
            if(r == 3 && (c >= 5))  continue;
            int idx = r * 7 + c;
            if(idx >= 27) continue;

            bool sel = (kbSection_ == 0 && r == kbRow_ && c == kbCol_);
            SDL_Surface* sheet = sel ? asciiSelSheet : asciiSheet;
            char ch;
            SDL_Rect src;
            if(idx == 26) { ch = '<'; src = (SDL_Rect){0, 48, 16, 16}; }
            else { ch = 'A' + idx; src = get_char_src(ch); }
            SDL_Rect dst = {(Sint16)(kbX + c * 32), (Sint16)(kbY + r * 24), 0, 0};
            SDL_BlitSurface(sheet, &src, screen, &dst);
        }
    }

    // 数字行 0-9
    int digitY = kbY + 4 * 24 + 8;
    for(int d = 0; d < 10; d++){
        bool sel = (kbSection_ == 1 && kbCol_ == d);
        SDL_Surface* sheet = sel ? asciiSelSheet : asciiSheet;
        SDL_Rect src = get_char_src('0' + d);
        SDL_Rect dst = {(Sint16)(kbX + d * 32), (Sint16)digitY, 0, 0};
        SDL_BlitSurface(sheet, &src, screen, &dst);
    }

    // Save 按钮
    int saveX = 340, saveY = digitY + 28;
    bool sav = (kbSection_ == 2);
    draw_string("S", saveX,       saveY, sav);
    draw_string("A", saveX + 16,  saveY, sav);
    draw_string("V", saveX + 32,  saveY, sav);
    draw_string("E", saveX + 48,  saveY, sav);
}
