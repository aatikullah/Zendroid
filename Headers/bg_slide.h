#ifndef BG_SLIDE_H
#define BG_SLIDE_H

// ============================================================================
// bg_slide.h  -  "connected background" slide used when entering a sub-level
//
// This is the same idea as the demo project (demo2):
//
//     iShowImage(bgX,                0, W, H, bg1);
//     iShowImage(bgX + SCREEN_WIDTH, 0, W, H, bg2);
//     ...
//     bgX -= SCROLL_SPEED;          // both pictures slide to the left
//
// In the demo the two pictures looped forever. Here the slide runs only ONCE,
// right after the player completes a level and enters a sub-level:
//
//     bg1 = background of the level the player just finished
//     bg2 = background of the sub-level that is starting
//
// bgX goes from 0 down to -700. When bg2 fully covers the screen the slide is
// over and the sub-level starts normally. While sliding, the player runs on
// the spot (walk-right animation), like the character in the demo.
//
// How it is used (see iMain.cpp):
//     startBgSlide(fromLevel, toLevel);  // right after initLevelXxx(false)
//     updateBgSlide();                   // every tick, from fixedUpdate()
//     drawBgSlide();                     // from iDraw() while bgSlideActive
//
// Level numbers are the same ones iMain.cpp uses in currentLevel:
//     1-5   = main levels 1..5
//     6,7   = level 3 sub 1, sub 2        10 = level 1 sub 1
//     8,9   = level 4 sub 1, sub 2        11,12 = level 2 sub 1, sub 2
//     13,14 = level 5 sub 1, sub 2
// ============================================================================

#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "player.h"

#ifndef screenWidth
#define screenWidth 700
#endif

#ifndef screenHeight
#define screenHeight 500
#endif

// How many pixels the pictures move every tick (like SCROLL_SPEED in the demo).
// Pick a number that divides 700 evenly (10, 14, 20, 25, 28, 35) so the slide
// ends exactly on the new picture. 14 gives 50 ticks (a bit under one second).
#define BG_SLIDE_SPEED 14

// ===== Slide state (plain globals, like bgX in the demo) =====
static bool bgSlideActive = false;   // true while the slide is playing
static int bgSlideX = 0;             // same job as bgX in the demo
static int bgSlideFromTex = 0;       // picture of the level we just finished
static int bgSlideToTex = 0;         // picture of the sub-level we are entering
static Player bgSlideSavedPlayer;    // player's real start state, put back at the end

// One loaded texture per level number (0 = not loaded yet).
static int bgSlideTexCache[15] = { 0 };

// Returns the background picture path of a level number.
static const char* getLevelBgPath(int level) {
	switch (level) {
	case 1:  return IMG_LV1_BG;
	case 2:  return IMG_LV2_BG;
	case 3:  return IMG_LV3_BG;
	case 4:  return IMG_LV4_BG;
	case 5:  return IMG_LV5_BG;
	case 6:  return IMG_LV3SUB1_BG;
	case 7:  return IMG_LV3SUB2_BG;
	case 8:  return IMG_LV4SUB1_BG;
	case 9:  return IMG_LV4SUB2_BG;
	case 10: return IMG_LV1_L1S1_BG;
	case 11: return L2S1_IMG_BG;
	case 12: return L2S2_IMG_BG;
	case 13: return IMG_LV5S1_BG;
	case 14: return IMG_LV5S2_BG;
	}
	return "";
}

// Loads the background of a level as a texture (only the first time).
static int getLevelBgTexture(int level) {
	if (level < 0 || level > 14) {
		return 0;
	}

	if (bgSlideTexCache[level] == 0) {
		const char* path = getLevelBgPath(level);

		char fallbackPath[160];
		sprintf(fallbackPath, "../%s", path);

		bgSlideTexCache[level] = loadTex(path, fallbackPath);
	}

	return bgSlideTexCache[level];
}

// Call this right after initLevelXxx() of the sub-level, when the player has
// just completed fromLevel and is entering toLevel.
static void startBgSlide(int fromLevel, int toLevel) {
	bgSlideFromTex = getLevelBgTexture(fromLevel);
	bgSlideToTex = getLevelBgTexture(toLevel);
	bgSlideX = 0;
	bgSlideActive = true;

	// The sub-level is already initialised, so "player" is standing at its
	// start point. Keep a copy and put it back when the slide finishes.
	bgSlideSavedPlayer = player;

	// Make the player run on the spot while the pictures slide.
	player.isMoving = true;
	player.facingRight = true;
	player.onGround = true;
}

// Called every tick while bgSlideActive is true.
static void updateBgSlide() {
	bgSlideX -= BG_SLIDE_SPEED;
	updatePlayerAnimation();

	if (bgSlideX <= -screenWidth) {
		bgSlideX = -screenWidth;
		bgSlideActive = false;
		player = bgSlideSavedPlayer;
	}
}

// Draws the two connected pictures and the running player.
static void drawBgSlide() {
	iShowImage(bgSlideX, 0, screenWidth, screenHeight, bgSlideFromTex);
	iShowImage(bgSlideX + screenWidth, 0, screenWidth, screenHeight, bgSlideToTex);
	drawPlayer();
}

// Stops the slide at once (used when the player leaves to a menu).
static void cancelBgSlide() {
	bgSlideActive = false;
}

#endif
