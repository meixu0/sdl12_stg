#include "UI.h"
#include "player.h"
class InfoArea{
private:
    static SDL_Surface* infoAreaTH07Logo;
    static SDL_Surface* perfectCherryBlossomImage;
    static SDL_Surface* highScoreImage;
    static SDL_Surface* scoreImage;
    static SDL_Surface* livesImage;
    static SDL_Surface* spellImage;
    static SDL_Surface* powerImage;
    static SDL_Surface* grazeImage;
    static SDL_Surface* pointImage;
    static SDL_Surface* timeImage;
    static SDL_Surface* redStarImage;
    static SDL_Surface* blueStarImage;
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
