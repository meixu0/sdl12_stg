#define STATE_MENU 0
#define STATE_GAME 1
#define STATE_EXIT 2
#define STATE_BOSS 3
#define STATE_START_MENU 4
#define STATE_DIFFICULTY_MENU 5

#define SPRITE_COLS 16
#define SPRITE_ROWS 16

#define PLAYER_REIMU 0
#define PLAYER_MARISA 1
#define PLAYER_SAKUYA 2

#define SC_REIMU_TSUITI 0
#define SC_REIMU_SOKUSHA 1
#define SC_MARISA_IRYOKUJUUSHI 2
#define SC_MARISA_KANTSUU 3
#define SC_SAKUYA_KOUHANNI 4
#define SC_SAKUYA_TOKUSHU 5

#define DIFFICULTY_EASY 0
#define DIFFICULTY_NORM 1
#define DIFFICULTY_HARD 2
#define DIFFICULTY_LNTC 3
#define DIFFICULTY_EXTR 4

extern int gameState;
extern int playerType;
extern int gameDifficulty;