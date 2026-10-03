#ifndef ASSETS_H
#define ASSETS_H
#define IMG_SOUND_AND_SCORE "images/soundANDscore.bmp"
#define IMG_SCORE "images/score.bmp"
// Audio Assets
#define BGM_MENU        "Audios//backgroundmusic.mp3" // plays continuously across Main Menu, Load Game, Settings, Sound/Score and Credits
#define BGM_LEVEL       "Audios//levelsbgm.mp3" // plays for every level (1, 2, 3, and so on)
#define SFX_SHOOT       "Audios//shooting.mp3"  // plays every time the player fires
#define SFX_JUMP        "Audios//jump.mp3"      // plays when the player jumps (unless they're shooting)
#define SFX_HIT         "Audios//hitEffect.mp3"  // plays the instant a laser lands on the dragon/boss

// soundOn is defined in iMain.cpp; declared here so shared headers
// (e.g. laser_fight.h) can check it before playing any sound effect.
extern bool soundOn;

// Image Assets
#define IMG_MAIN_MENU   "Images/mainmenu.bmp"
#define IMG_SOUND_OPT   "Images/sound_options.bmp"
#define IMG_CREDITS     "Images/credits_menu.bmp"
#define IMG_LOAD_GAME   "Images/load_game_options.bmp"
#define IMG_LV1_BG      "Images/lv1_bg.bmp"

// Player Sprites
#define SPRITE_JUMP1    "Images/player/0jump.png"
#define SPRITE_JUMP2    "Images/player/1.png"

#endif