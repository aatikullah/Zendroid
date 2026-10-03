#ifndef LEVEL_COMMON_H
#define LEVEL_COMMON_H

// ============================================================================
// level_common.h
//
// level1.h .. level5.h were originally five independent, hand-copied files.
// Each one re-implemented the exact same HUD, pause-menu, collectible and
// texture-loading code under a per-level name (e.g. drawHUD / drawHUD2 /
// drawHUD3 ...). This header pulls out every piece that was byte-for-byte
// identical between the levels (only the variable names differed) into a
// single, reusable implementation. Each level file now just forwards its own
// (still per-level) variables into these shared functions, so behaviour and
// on-screen output are unchanged - only the duplication is gone.
//
// Things that genuinely differ between levels (platform layouts, collectible
// placement algorithms, dragon counts/AI, portal/transition logic, level 4's
// hold-to-charge mouse mechanic, etc.) are intentionally left untouched in
// each level file, since unifying those really would change behavior.
// ============================================================================

#include "iGraphics.h"
#include "game_objects.h"
#include "player.h"
#include "laser_fight.h"
#include <cstdio>

// ---------------------------------------------------------------------------
// Points and energy now persist across levels instead of resetting when a
// new level starts. There's one shared value for the whole playthrough;
// each level's own "playerPoints"/"energyFrame" variable is just a reference
// to these, so every level naturally reads/writes the same running total.
// They only get reset back to 0 / 1 on an explicit fresh start (New Game,
// Load Game level-select, or Restart) - see the `freshStart` parameter on
// each level's initLevelN().
// ---------------------------------------------------------------------------
static int gPlayerPoints = 0;
static int gEnergyFrame = 1;

// ---------------------------------------------------------------------------
// Texture loading with a fallback path.
// Replaces the repeated:
//     tex = iLoadImage("Images/x.png");
//     if (tex <= 0) tex = iLoadImage("../Images/x.png");
// pattern that appeared dozens of times per level file.
// ---------------------------------------------------------------------------
static int loadTex(const char* primaryPath, const char* fallbackPath) {
	int tex = iLoadImage((char*)primaryPath);
	if (tex <= 0) tex = iLoadImage((char*)fallbackPath);
	return tex;
}

// ---------------------------------------------------------------------------
// Filled rectangle with rounded corners (was copy-pasted as
// iFilledRoundRect / iFilledRoundRect2 / 3 / 4 / 5, all identical).
// ---------------------------------------------------------------------------
static void iFilledRoundRect(double x, double y, double width, double height, double radius) {
	if (radius <= 0) {
		iFilledRectangle(x, y, width, height);
		return;
	}
	iFilledRectangle(x + radius, y, width - 2 * radius, height);
	iFilledRectangle(x, y + radius, width, height - 2 * radius);

	iFilledCircle(x + radius, y + radius, radius, 100);
	iFilledCircle(x + width - radius, y + radius, radius, 100);
	iFilledCircle(x + radius, y + height - radius, radius, 100);
	iFilledCircle(x + width - radius, y + height - radius, radius, 100);
}

// ---------------------------------------------------------------------------
// Shared "Zeds" (lives) + energy-bar HUD, identical across every level.
// ---------------------------------------------------------------------------
static void drawHUDGeneric(int zedsLabelTex, int zedsIconTex, int lives,
	int energyFrame, int energyTextures[]) {
	if (zedsLabelTex > 0) {
		iShowImage(20, 460, 60, 31, zedsLabelTex);
	}
	else {
		iSetColor(0, 150, 255);
		iFilledRectangle(20, 460, 60, 31);
		iSetColor(255, 255, 255);
		iText(30, 470, "Zeds", GLUT_BITMAP_HELVETICA_12);
	}

	for (int i = 0; i < lives; i++) {
		if (zedsIconTex > 0) {
			iShowImage(88 + (i * 24), 464, 20, 22, zedsIconTex);
		}
		else {
			iSetColor(220, 0, 0);
			iFilledCircle(98 + (i * 24), 475, 8, 100);
		}
	}

	int energyX = 215;
	int energyY = 464;

	if (energyFrame >= 1 && energyFrame <= 145 && energyTextures[energyFrame] > 0) {
		iShowImage(energyX, energyY, 120, 68, energyTextures[energyFrame]);
	}
	else {
		iSetColor(200, 200, 200);
		iFilledRectangle(energyX, energyY, 120, 19);
		iSetColor(0, 0, 0);
		iText(energyX + 30, energyY + 4, "ENERGY", GLUT_BITMAP_HELVETICA_12);
	}
}

// ---------------------------------------------------------------------------
// Shared pause-menu (Resume / Restart / Exit) drawing.
// ---------------------------------------------------------------------------
static void drawPauseMenuGeneric(int resumeTex, Button resumeBtn,
	int restartTex, Button restartBtn,
	int exitTex, Button exitBtn) {
	if (resumeTex > 0)
		iShowImage(resumeBtn.x1, resumeBtn.y1, 180, 45, resumeTex);

	if (restartTex > 0)
		iShowImage(restartBtn.x1, restartBtn.y1, 180, 45, restartTex);

	if (exitTex > 0)
		iShowImage(exitBtn.x1, exitBtn.y1, 180, 45, exitTex);
}

// ---------------------------------------------------------------------------
// Gold ball collision + draw + HUD icon.
// Templated on the level's own GoldBall struct type (GoldBall, GoldBall2, ...)
// so the per-level struct definitions don't need to change at all - only the
// duplicated function *bodies* are shared.
// ---------------------------------------------------------------------------
template <typename GoldBallT>
static void checkGoldBallCollisionGeneric(GoldBallT goldBalls[3], int* collectedCount, bool* levelCompleteFlag) {
	for (int i = 0; i < 3; i++) {
		if (!goldBalls[i].collected) {
			bool collideX = (player.x + player.width > goldBalls[i].x) && (player.x < goldBalls[i].x + goldBalls[i].width);
			bool collideY = (player.y + player.height > goldBalls[i].y) && (player.y < goldBalls[i].y + goldBalls[i].height);

			if (collideX && collideY) {
				goldBalls[i].collected = true;
				(*collectedCount)++;
				if (*collectedCount >= 3) {
					*levelCompleteFlag = true;
				}
			}
		}
	}
}

template <typename GoldBallT>
static void drawGoldBallsGeneric(GoldBallT goldBalls[3], int goldBallTex) {
	for (int i = 0; i < 3; i++) {
		if (!goldBalls[i].collected) {
			if (goldBallTex > 0) {
				iShowImage(goldBalls[i].x, goldBalls[i].y, goldBalls[i].width, goldBalls[i].height, goldBallTex);
			}
			else {
				iSetColor(255, 215, 0);
				iFilledRectangle(goldBalls[i].x, goldBalls[i].y, goldBalls[i].width, goldBalls[i].height);
			}
		}
	}
}

static void drawGoldBallHUDGeneric(int goldBallsCollected, int goldBallIconTextures[5]) {
	int iconIndex = goldBallsCollected + 1;
	if (iconIndex > 4) iconIndex = 4;
	if (iconIndex < 1) iconIndex = 1;

	if (goldBallIconTextures[iconIndex] > 0) {
		iShowImage(510, 455, 110, 30, goldBallIconTextures[iconIndex]);
	}
}

// ---------------------------------------------------------------------------
// Blue ball collision + draw + points HUD.
// ---------------------------------------------------------------------------
template <typename BlueBallT>
static void checkBlueBallCollisionGeneric(BlueBallT blueBalls[9], int* playerPointsPtr) {
	for (int i = 0; i < 9; i++) {
		if (!blueBalls[i].collected) {
			bool collideX = (player.x + player.width > blueBalls[i].x) && (player.x < blueBalls[i].x + blueBalls[i].width);
			bool collideY = (player.y + player.height > blueBalls[i].y) && (player.y < blueBalls[i].y + blueBalls[i].height);

			if (collideX && collideY) {
				blueBalls[i].collected = true;
				*playerPointsPtr += 10;
			}
		}
	}
}

template <typename BlueBallT>
static void drawBlueBallsGeneric(BlueBallT blueBalls[9], int blueBallTex) {
	for (int i = 0; i < 9; i++) {
		if (!blueBalls[i].collected) {
			if (blueBallTex > 0) {
				iShowImage(blueBalls[i].x, blueBalls[i].y, blueBalls[i].width, blueBalls[i].height, blueBallTex);
			}
			else {
				iSetColor(0, 100, 255);
				iFilledCircle(blueBalls[i].x + 9, blueBalls[i].y + 9, 9, 100);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// "YOUR TOTAL POINTS" box shown on the Game Over screen. Takes the y
// position as a parameter since level1's game-over art leaves room at
// y=360, while levels 2-5 use a taller game-over graphic and need the box
// placed higher up (see each level's drawLevelN()) to avoid overlapping it.
// ---------------------------------------------------------------------------
static void drawTotalPointsBoxGeneric(int totalPointsTex, int playerPoints, double boxY) {
	if (totalPointsTex > 0) {
		iShowImage(210, boxY, 180, 34, totalPointsTex);
	}
	else {
		iSetColor(50, 50, 50);
		iFilledRectangle(210, boxY, 180, 34);
		iSetColor(255, 255, 255);
		iText(220, boxY + 10, "TOTAL POINTS", GLUT_BITMAP_HELVETICA_12);
	}

	// Rounded rectangle background for the score
	iSetColor(0, 0, 0);
	iFilledRoundRect(395, boxY, 85, 34, 6);

	iSetColor(255, 255, 255);
	char finalScoreStr[16];
	sprintf(finalScoreStr, "%d", playerPoints);
	iText(425, boxY + 8, finalScoreStr, GLUT_BITMAP_HELVETICA_18);
}

static void drawPointsHUDGeneric(int pointsTex, int playerPoints) {
	if (pointsTex > 0) {
		iShowImage(360, 458, 60, 24, pointsTex);
	}
	else {
		iSetColor(255, 255, 255);
		iText(360, 463, "PTS", GLUT_BITMAP_HELVETICA_12);
	}

	// Semi-transparent black background with smooth rounded corners (Radius: 6px)
	iSetColor(0, 0, 0);
	iFilledRoundRect(425, 458, 65, 24, 6);

	// Solid outline around points box
	iSetColor(255, 255, 255);

	char scoreStr[16];
	sprintf(scoreStr, "%d", playerPoints);
	iText(440, 462, scoreStr, GLUT_BITMAP_HELVETICA_18);
}

#endif