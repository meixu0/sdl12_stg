#include "UI.h"
#include "player.h"
class InfoArea{
private:
    static SDL_Surface* infoAreaTH08Logo;// res/title/title02.png
    static SDL_Surface* highScoreImage; // res/front/front.img
    static SDL_Surface* scoreImage;     // res/front/front.img
    static SDL_Surface* livesImage;     // res/front/front.img
    static SDL_Surface* spellImage;     // res/front/front.img
    static SDL_Surface* powerImage;     // res/front/front.img
    static SDL_Surface* grazeImage;     // res/front/front.img
    static SDL_Surface* pointImage;     // res/front/front.img
    static SDL_Surface* timeImage;      // res/front/front.img
    static SDL_Surface* redStarImage;   // res/front/front.img
    static SDL_Surface* blueStarImage;  // /res/front/frong.img
    Uint32 currentScore, lives, spell, graze, point, time_;
public:
    InfoArea();
    ~InfoArea();
    void update_high_score(Uint32 highscore);
    void update_score(Uint32 score);
    void update_lives(Uint32 lives);
    void update_spell(Uint32 spell);
    void update_graze(Uint32 graze);
    void update_point(Uint32 point);
    void update_time(Uint32 time);
    void render();
};
