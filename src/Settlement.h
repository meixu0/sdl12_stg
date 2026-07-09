#include "UI.h"

class Settlement {
private:
    static SDL_Surface* bgImage;
    static SDL_Surface* asciiSheet;
    static SDL_Surface* asciiSelSheet;
    static Mix_Music*   bgm;
    static Mix_Chunk*   select00;
    static Mix_Chunk*   ok00;

    char name_[9];        // 玩家名 8字符 + null
    int  nameLen_;        // 当前已输入长度
    int  kbRow_;          // 键盘选中行 0-3
    int  kbCol_;          // 键盘选中列 0-6
    int  kbSection_;      // 0=字母, 1=数字, 2=Save
    bool confirmed_;

    // 获取字母的源矩形（从 ascii 表）
    SDL_Rect get_char_src(char c) const;
    // 绘制字符串
    void draw_string(const char* s, int x, int y, bool selected);
    // 绘制数字
    void draw_number(int n, int x, int y, bool selected, int minW);
    // 添加当前选中字母到名字
    void add_letter();
    // 删除名字最后一个字符
    void del_letter();
    // 保存记录到 record.json
    void save_record();
    // Difficulty → 字符串
    const char* diff_name() const;

public:
    Settlement();
    void handle_events(SDL_Event &e);
    void render();
    bool is_confirmed() const { return confirmed_; }
    void reset();
};
