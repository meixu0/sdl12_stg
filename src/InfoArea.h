#include "UI.h"
#include "player.h"
class InfoArea{
private:
    Uint32 currentScore, lives, spell, graze, point, time_;
public:
    InfoArea();
    ~InfoArea();
    void update_high_score(Uint32 highscore);
    void update_score(Uint32 score);
    void update_lives(Uint32 lives);
    void update_spell(Uint32 spell);
    void update_graze(Uint32 graze);
    void update_time(Uint32 time);
    void render();
};
