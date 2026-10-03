#ifndef LEVEL3_SUB1_H
#define LEVEL3_SUB1_H
#define IMG_LV3SUB1_BG "Images/lv_3_sub1bg.bmp"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// ============================================================================
// level3_sub1.h
//
// First bonus stage of Level 3 (plays after level 3, before level3_sub2.h).
//
// Level 3 + sub 1 + sub 2 are treated as ONE level:
//   - zeds (lives) and energy refill at the start of every sub level (points carry over)
//   - no banner is shown when moving between them; the only
//     "Level 3 Completed / Level 4 Starts" banner comes at the end of sub 2
//
// Geometry: every rectangle below is measured off lv_3_sub1bg (700 x 500,
// 1 pixel = 1 game unit, gameY = 500 - pixelY). Each ember block is ~24 x 27:
//   - two ember floor slabs (7 blocks each) joined to the middle by two tall
//     wall pillars                      -> floors top y = 128, walls top y = 186
//   - a 4-step staircase on the left, climbing up and to the LEFT   (peak top 431)
//   - a 4-step staircase in the centre, climbing up and to the RIGHT (peak top 403)
//   - a 5-step staircase on the right, dropping down and to the right,
//     plus one lone block next to the right wall  (last step top 324)
//
// Enemies: 3 patrolling dragons, but they fire only 2 fireballs in total
// (MAX_FIREBALL_SHOTS_S1); after that the stage has no more fire.
//
// Gold balls: one on the ground slabs (right slab), one on the middle (centre)
// staircase peak, one in the right corner just under the lone block.
// ============================================================================

#define PLATFORM_COUNT_L3SUB1 18
static Platform level3_sub1_platforms[PLATFORM_COUNT_L3SUB1] = {
	// { x1 (left), y1 (bottom edge), x2 (right), y2 (walkable top) }
	// Measured from lv_3_sub1bg by template-matching every ember block.

	// Ground-level ember floors (7 blocks each)
	{ 108, 101, 278, 128 }, // 0  - left floor
	{ 350, 101, 519, 128 }, // 1  - right floor

	// Wall pillars standing on the inner end of each floor (solid on the sides too)
	{ 254, 128, 278, 186 }, // 2  - left wall
	{ 350, 128, 374, 186 }, // 3  - right wall

	// Left staircase, climbing up and to the left
	{ 131, 321, 155, 348 }, // 4  - lowest step
	{ 107, 349, 131, 376 }, // 5
	{ 84, 376, 108, 403 }, // 6
	{ 60, 404, 84, 431 }, // 7  - left peak, GoldBall 0 sits here

	// Centre staircase, climbing up and to the right
	{ 303, 293, 327, 320 }, // 8  - lowest step
	{ 327, 321, 351, 348 }, // 9
	{ 352, 348, 376, 375 }, // 10
	{ 376, 376, 400, 403 }, // 11 - centre peak, GoldBall 1 sits here

	// Right staircase, dropping down and to the right
	{ 475, 396, 499, 423 }, // 12 - highest step
	{ 501, 371, 525, 398 }, // 13
	{ 527, 346, 551, 373 }, // 14
	{ 553, 322, 577, 349 }, // 15
	{ 579, 297, 603, 324 }, // 16

	// Lone block beside the right wall
	{ 656, 297, 680, 324 }  // 17 - GoldBall 2 sits here, drop off it to the portal
};

#define LEVEL_LEFT_S1   20
#define LEVEL_RIGHT_S1  680
#define LEVEL_BOTTOM_S1 20
#define LEVEL_TOP_S1    500   // headroom for the left peak (top y = 431), like sub 2

// ===== Dragon Frame Boundaries =====
#define DRAGON_LEFT_START_S1   1
#define DRAGON_LEFT_END_S1     17
#define DRAGON_RIGHT_START_S1  31
#define DRAGON_RIGHT_END_S1    63
#define TOTAL_DRAGON_FRAMES_S1 65
#define DRAGON_SPEED_S1        2
#define DRAGON_ANIM_SPEED_S1   3
#define DRAGON_COUNT_S1        3

// ===== Fireball & Particle Variables =====
#define MAX_FIREBALLS_S1           2    // array size (never more than 2 alive at once)
#define MAX_FIREBALL_SHOTS_S1      2    // total fireballs the dragons fire in this stage - only 2, ever
static int fireballsFiredSub1 = 0;        // shots fired so far (reset in initLevel3Sub1)
#define MAX_TRAIL_PARTICLES_S1     60
#define MAX_EXPLOSION_PARTICLES_S1 40

struct FireballS1 { int x, y; int width, height; float vx, vy; float rotation; bool active; };
struct TrailParticleS1 { float x, y; float radius; int alpha; bool active; };
struct ExplosionParticleS1 { float x, y; float vx, vy; float radius; int life; bool active; };

static FireballS1 fireballsS1[MAX_FIREBALLS_S1];
static TrailParticleS1 trailParticlesS1[MAX_TRAIL_PARTICLES_S1];
static ExplosionParticleS1 explosionParticlesS1[MAX_EXPLOSION_PARTICLES_S1];
static int fireballTextureSub1 = 0;

// ===== Game State Variables =====
static bool level3Sub1Complete = false;
static bool isGameOverSub1 = false;
static bool isPausedSub1 = false;
static bool enterLevel4FromSub1 = false;
static int hitCooldownSub1 = 0;

// ----- Exit transition (sub-level completed -> level4 starts) -----
static bool showLevelSub1ExitTransition = false;
static int level4TransitionTimerIdSub1 = -1;

// Intro banner is gone (Level 3 flows straight into this stage). The flag is kept,
// always false, so any other file that still reads it keeps compiling.
static bool isLevelSub1Transition = false;
static int levelSub1TransitionCounter = 0;

// ===== Player Lives, Energy & Points Variables =====
static int playerLivesSub1 = 5;
static int& energyFrameSub1 = gEnergyFrame;
static int distanceMovedSub1 = 0;
static int energyTexturesSub1[150] = { 0 };
static int& playerPointsSub1 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTextureSub1 = 0;
static int zedsIconTextureSub1 = 0;

// ===== Menu Textures & Bounding Boxes =====
static int resumeTextureSub1 = 0;
static int restartTextureSub1 = 0;
static int exitTextureSub1 = 0;

static Button pauseResumeBtnSub1 = { 260, 280, 440, 325 };
static Button pauseRestartBtnSub1 = { 260, 220, 440, 265 };
static Button pauseExitBtnSub1 = { 260, 160, 440, 205 };

// ===== Game Over Menu Textures & Bounding Boxes =====
static int totalPointsTextureSub1 = 0;
static int gameOverTextureSub1 = 0;
static int gameOverRestartTextureSub1 = 0;
static int gameOverExitTextureSub1 = 0;

static Button gameOverRestartBtnSub1 = { 240, 115, 460, 165 };
static Button gameOverExitBtnSub1 = { 275, 70, 425, 105 };

// ===== Gold Ball Collectibles, Portal & HUD =====
struct GoldBallSub1 {
	int x, y;
	int width, height;
	bool collected;
};

static GoldBallSub1 goldBallsSub1[3];
static int goldBallTextureSub1 = 0;
static int goldBallIconTexturesSub1[5] = { 0 };
static int goldBallsCollectedSub1 = 0;
static int dispearTextureSub1 = 0;

// ===== Blue Ball Collectibles & Points HUD =====
struct BlueBallSub1 {
	int x, y;
	int width, height;
	bool collected;
};

static BlueBallSub1 blueBallsSub1[9];
static int blueBallTextureSub1 = 0;
static int pointsTextureSub1 = 0;

// ===== Dragons =====
struct PatrolDragonSub1 {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
	int minX, maxX;
	int shootTimer;
	int shootCooldown;
};

static PatrolDragonSub1 dragonsSub1[DRAGON_COUNT_S1];
static int dragonTexturesSub1[TOTAL_DRAGON_FRAMES_S1];
static bool texturesLoadedSub1 = false;

static void loadDragonTexturesSub1() {
	if (texturesLoadedSub1) return;

	char path[128];
	for (int i = 1; i <= DRAGON_RIGHT_END_S1; i++) {
		if (i >= 18 && i <= 30) { dragonTexturesSub1[i] = 0; continue; }

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTexturesSub1[i] = iLoadImage(path);

		if (dragonTexturesSub1[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTexturesSub1[i] = iLoadImage(path);
		}
	}
	texturesLoadedSub1 = true;
}

static void loadFireballTextureSub1() {
	static bool loaded = false;
	if (loaded) return;
	fireballTextureSub1 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadZedsTexturesSub1() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTextureSub1 = loadTex("Images/zeds.png", "../Images/zeds.png");

	zedsIconTextureSub1 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTextureSub1 <= 0) zedsIconTextureSub1 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTextureSub1 <= 0) zedsIconTextureSub1 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTexturesSub1() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTexturesSub1[i] = iLoadImage(path);

		if (energyTexturesSub1[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTexturesSub1[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadPauseTexturesSub1() {
	static bool loaded = false;
	if (loaded) return;

	resumeTextureSub1 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTextureSub1 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTextureSub1 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTextureSub1 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTextureSub1 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTextureSub1 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTextureSub1 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadGoldBallTexturesSub1() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTextureSub1 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTextureSub1 = loadTex("Images/dispear3.png", "../Images/dispear3.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTexturesSub1[i] = iLoadImage(path);
		if (goldBallIconTexturesSub1[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTexturesSub1[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadBlueBallAndPointsTexturesSub1() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTextureSub1 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTextureSub1 = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

static void initGoldAndBlueBallsSub1(bool freshStart) {
	loadGoldBallTexturesSub1();
	loadBlueBallAndPointsTexturesSub1();
	goldBallsCollectedSub1 = 0;
	if (freshStart) playerPointsSub1 = 0;
	enterLevel4FromSub1 = false;

	// --- Gold balls ---
	// 0: on the ground slabs (right slab, resting on the slab top + 2)
	goldBallsSub1[0].width = 25; goldBallsSub1[0].height = 25;
	goldBallsSub1[0].x = 445;  goldBallsSub1[0].y = level3_sub1_platforms[1].y2 + 2;
	goldBallsSub1[0].collected = false;

	goldBallsSub1[1].width = 25; goldBallsSub1[1].height = 25;
	// 1: middle staircase peak
	goldBallsSub1[1].x = level3_sub1_platforms[11].x1 + 2; goldBallsSub1[1].y = level3_sub1_platforms[11].y2 + 2;
	goldBallsSub1[1].collected = false;

	goldBallsSub1[2].width = 25; goldBallsSub1[2].height = 25;
	// 2: right corner, hanging just under the lone block (block bottom = 297)
	goldBallsSub1[2].x = level3_sub1_platforms[17].x1 - 40; goldBallsSub1[2].y = level3_sub1_platforms[17].y1 - 40;
	goldBallsSub1[2].collected = false;

	// --- Blue balls: on the floors, walls and stair steps ---
	struct { int x, y; } scattered[9] = {
		{ 150, 130 }, // left floor
		{ 420, 130 }, // right floor
		{ 258, 187 }, // left wall top
		{ 354, 187 }, // right wall top
		{ 110, 377 }, // left staircase, step 3
		{ 308, 322 }, // centre staircase, step 1
		{ 356, 377 }, // centre staircase, step 3
		{ 505, 400 }, // right staircase, step 2
		{ 557, 351 }  // right staircase, step 4
	};

	for (int i = 0; i < 9; i++) {
		blueBallsSub1[i].width = 18;
		blueBallsSub1[i].height = 18;
		blueBallsSub1[i].x = scattered[i].x;
		blueBallsSub1[i].y = scattered[i].y;
		blueBallsSub1[i].collected = false;
	}
}

static void checkGoldBallCollisionSub1() {
	checkGoldBallCollisionGeneric(goldBallsSub1, &goldBallsCollectedSub1, &level3Sub1Complete);
}

static void checkBlueBallCollisionSub1() {
	checkBlueBallCollisionGeneric(blueBallsSub1, &playerPointsSub1);
}

static void drawGoldBallsSub1() {
	drawGoldBallsGeneric(goldBallsSub1, goldBallTextureSub1);
}

static void drawBlueBallsSub1() {
	drawBlueBallsGeneric(blueBallsSub1, blueBallTextureSub1);
}

static void drawPortalSub1() {
	if (level3Sub1Complete) {
		if (dispearTextureSub1 > 0) {
			iShowImage(680, 20, 35, 100, dispearTextureSub1);
		}
		else {
			iSetColor(255, 255, 0);
			iRectangle(645, 20, 35, 100);
		}
	}
}

static void triggerEnterLevel4FromSub1() {
	enterLevel4FromSub1 = true;
	showLevelSub1ExitTransition = false;

	if (level4TransitionTimerIdSub1 >= 0) {
		iPauseTimer(level4TransitionTimerIdSub1);
	}
}

static void checkPortalCollisionSub1() {
	if (level3Sub1Complete && !showLevelSub1ExitTransition) {
		bool collideX = (player.x + player.width >= 700);
		bool collideY = (player.y <= 120 && player.y + player.height >= 20);

		if (collideX && collideY) {
			showLevelSub1ExitTransition = true;

			if (level4TransitionTimerIdSub1 < 0) {
				level4TransitionTimerIdSub1 = iSetTimer(1000, triggerEnterLevel4FromSub1);
			}
			else {
				iResumeTimer(level4TransitionTimerIdSub1);
			}
		}
	}
}

static void drawGoldBallHUDSub1() {
	drawGoldBallHUDGeneric(goldBallsCollectedSub1, goldBallIconTexturesSub1);
}

static void drawPointsHUDSub1() {
	drawPointsHUDGeneric(pointsTextureSub1, playerPointsSub1);
}

static void drawHUDSub1() {
	drawHUDGeneric(zedsLabelTextureSub1, zedsIconTextureSub1, playerLivesSub1, energyFrameSub1, energyTexturesSub1);
}

// ===========================================================================
// DRAGONS
//
// All three hover above the slabs (level 3 style). Dragon 0 sweeps over the
// two floors and the wall pillars, dragon 1 guards the centre staircase peak,
// dragon 2 patrols above the right staircase / lone block.
// ===========================================================================
static void stepNextFrameSub1(int i) {
	int safetyCounter = 0;
	do {
		dragonsSub1[i].animFrame++;

		if (dragonsSub1[i].movingRight) {
			if (dragonsSub1[i].animFrame > DRAGON_RIGHT_END_S1 || dragonsSub1[i].animFrame < DRAGON_RIGHT_START_S1)
				dragonsSub1[i].animFrame = DRAGON_RIGHT_START_S1;
		}
		else {
			if (dragonsSub1[i].animFrame > DRAGON_LEFT_END_S1 || dragonsSub1[i].animFrame < DRAGON_LEFT_START_S1)
				dragonsSub1[i].animFrame = DRAGON_LEFT_START_S1;
		}

		safetyCounter++;
		if (safetyCounter > TOTAL_DRAGON_FRAMES_S1) break;
	} while (dragonTexturesSub1[dragonsSub1[i].animFrame] <= 0);
}

static void initDragonSub1() {
	loadDragonTexturesSub1();

	// Over the floors and walls
	dragonsSub1[0].width = 60; dragonsSub1[0].height = 60;
	dragonsSub1[0].x = 130; dragonsSub1[0].y = 205;
	dragonsSub1[0].minX = 110; dragonsSub1[0].maxX = 450;
	dragonsSub1[0].movingRight = true;
	dragonsSub1[0].animFrame = DRAGON_RIGHT_START_S1;
	dragonsSub1[0].animTimer = 0;
	dragonsSub1[0].shootTimer = 100 + (rand() % 50);
	dragonsSub1[0].shootCooldown = 160;
	if (dragonTexturesSub1[dragonsSub1[0].animFrame] <= 0) stepNextFrameSub1(0);

	// Above the centre staircase peak
	dragonsSub1[1].width = 60; dragonsSub1[1].height = 60;
	dragonsSub1[1].x = 380; dragonsSub1[1].y = 415;
	dragonsSub1[1].minX = 290; dragonsSub1[1].maxX = 430;
	dragonsSub1[1].movingRight = false;
	dragonsSub1[1].animFrame = DRAGON_LEFT_START_S1;
	dragonsSub1[1].animTimer = 0;
	dragonsSub1[1].shootTimer = 130 + (rand() % 60);
	dragonsSub1[1].shootCooldown = 180;
	if (dragonTexturesSub1[dragonsSub1[1].animFrame] <= 0) stepNextFrameSub1(1);

	// Above the right staircase and the lone block
	dragonsSub1[2].width = 60; dragonsSub1[2].height = 60;
	dragonsSub1[2].x = 600; dragonsSub1[2].y = 365;
	dragonsSub1[2].minX = 540; dragonsSub1[2].maxX = 620;
	dragonsSub1[2].movingRight = false;
	dragonsSub1[2].animFrame = DRAGON_LEFT_START_S1;
	dragonsSub1[2].animTimer = 0;
	dragonsSub1[2].shootTimer = 90 + (rand() % 60);
	dragonsSub1[2].shootCooldown = 150;
	if (dragonTexturesSub1[dragonsSub1[2].animFrame] <= 0) stepNextFrameSub1(2);
}

static void updateDragonSub1() {
	for (int i = 0; i < DRAGON_COUNT_S1; i++) {
		if (dragonsSub1[i].movingRight) {
			dragonsSub1[i].x += DRAGON_SPEED_S1;
			if (dragonsSub1[i].x >= dragonsSub1[i].maxX) {
				dragonsSub1[i].x = dragonsSub1[i].maxX;
				dragonsSub1[i].movingRight = false;
				dragonsSub1[i].animFrame = DRAGON_LEFT_START_S1;
				if (dragonTexturesSub1[dragonsSub1[i].animFrame] <= 0) stepNextFrameSub1(i);
			}
		}
		else {
			dragonsSub1[i].x -= DRAGON_SPEED_S1;
			if (dragonsSub1[i].x <= dragonsSub1[i].minX) {
				dragonsSub1[i].x = dragonsSub1[i].minX;
				dragonsSub1[i].movingRight = true;
				dragonsSub1[i].animFrame = DRAGON_RIGHT_START_S1;
				if (dragonTexturesSub1[dragonsSub1[i].animFrame] <= 0) stepNextFrameSub1(i);
			}
		}

		dragonsSub1[i].animTimer++;
		if (dragonsSub1[i].animTimer >= DRAGON_ANIM_SPEED_S1) {
			dragonsSub1[i].animTimer = 0;
			stepNextFrameSub1(i);
		}

		// --- Fireball shooting ---
		dragonsSub1[i].shootTimer--;
		if (dragonsSub1[i].shootTimer <= 0) {
			for (int j = 0; j < MAX_FIREBALLS_S1 && fireballsFiredSub1 < MAX_FIREBALL_SHOTS_S1; j++) {
				if (!fireballsS1[j].active) {
					fireballsS1[j].x = dragonsSub1[i].x + dragonsSub1[i].width / 2 - 10;
					fireballsS1[j].y = dragonsSub1[i].y + dragonsSub1[i].height / 2 - 10;
					fireballsS1[j].width = 20; fireballsS1[j].height = 20;

					float dx = (player.x + player.width / 2.0f) - (fireballsS1[j].x + 10);
					float dy = (player.y + player.height / 2.0f) - (fireballsS1[j].y + 10);
					float dist = sqrt(dx * dx + dy * dy);
					if (dist == 0) dist = 1;

					float speed = 7.0f;
					fireballsS1[j].vx = (dx / dist) * speed;
					fireballsS1[j].vy = (dy / dist) * speed + 4.5f; // arc boost
					fireballsS1[j].rotation = 0.0f;
					fireballsS1[j].active = true;
					fireballsFiredSub1++;
					break;
				}
			}
			dragonsSub1[i].shootTimer = dragonsSub1[i].shootCooldown;
		}
	}
}

static void drawDragonSub1() {
	for (int i = 0; i < DRAGON_COUNT_S1; i++) {
		int textureId = dragonTexturesSub1[dragonsSub1[i].animFrame];

		if (textureId > 0) {
			iShowImage(dragonsSub1[i].x, dragonsSub1[i].y, dragonsSub1[i].width, dragonsSub1[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragonsSub1[i].x, dragonsSub1[i].y, dragonsSub1[i].width, dragonsSub1[i].height);
			iSetColor(255, 255, 255);
			iText(dragonsSub1[i].x + 5, dragonsSub1[i].y + 25, "DRAGON", GLUT_BITMAP_HELVETICA_10);
		}
	}
}

// ===========================================================================
// FIREBALLS (arcs, trails, explosions)
// ===========================================================================
static void spawnExplosionSub1(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES_S1; j++) {
			if (!explosionParticlesS1[j].active) {
				explosionParticlesS1[j].x = x;
				explosionParticlesS1[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticlesS1[j].vx = cos(angle) * speed;
				explosionParticlesS1[j].vy = sin(angle) * speed;
				explosionParticlesS1[j].radius = 3.0f + (rand() % 3);
				explosionParticlesS1[j].life = 20 + (rand() % 10);
				explosionParticlesS1[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticleSub1(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES_S1; i++) {
		if (!trailParticlesS1[i].active) {
			trailParticlesS1[i].x = x + (rand() % 6 - 3);
			trailParticlesS1[i].y = y + (rand() % 6 - 3);
			trailParticlesS1[i].radius = 4.0f + (rand() % 4);
			trailParticlesS1[i].alpha = 255;
			trailParticlesS1[i].active = true;
			break;
		}
	}
}

static void updateFireballsSub1() {
	for (int i = 0; i < MAX_FIREBALLS_S1; i++) {
		if (!fireballsS1[i].active) continue;

		int oldY = fireballsS1[i].y;

		fireballsS1[i].vy -= 0.2f;                       // gravity -> parabola
		fireballsS1[i].x += fireballsS1[i].vx;
		fireballsS1[i].y += fireballsS1[i].vy;
		fireballsS1[i].rotation += fireballsS1[i].vx * 6.0f;

		// Bounce off the floor
		if (fireballsS1[i].y <= LEVEL_BOTTOM_S1) {
			fireballsS1[i].y = LEVEL_BOTTOM_S1;
			fireballsS1[i].vy = -fireballsS1[i].vy * 0.5f;
			fireballsS1[i].vx *= 0.8f;
			spawnExplosionSub1(fireballsS1[i].x + 10, fireballsS1[i].y + 10);
			if (fabs(fireballsS1[i].vy) < 1.5f) fireballsS1[i].active = false;
		}

		// Burst against an ember block instead of passing through it
		if (fireballsS1[i].active && fireballsS1[i].vy < 0) {
			for (int p = 0; p < PLATFORM_COUNT_L3SUB1; p++) {
				Platform plat = level3_sub1_platforms[p];

				bool withinX = (fireballsS1[i].x + fireballsS1[i].width > plat.x1) && (fireballsS1[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballsS1[i].y <= plat.y2) {
					fireballsS1[i].y = plat.y2;
					spawnExplosionSub1(fireballsS1[i].x + 10, fireballsS1[i].y + 10);
					fireballsS1[i].active = false;
					break;
				}
			}
		}

		if (fireballsS1[i].x < -50 || fireballsS1[i].x > 750) fireballsS1[i].active = false;

		if (fireballsS1[i].active)
			spawnTrailParticleSub1(fireballsS1[i].x + 10, fireballsS1[i].y + 10);
	}
}

static void updateTrailParticlesSub1() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES_S1; i++) {
		if (!trailParticlesS1[i].active) continue;
		trailParticlesS1[i].alpha -= 15;
		trailParticlesS1[i].radius -= 0.2f;
		if (trailParticlesS1[i].alpha <= 0 || trailParticlesS1[i].radius <= 0)
			trailParticlesS1[i].active = false;
	}
}

static void updateExplosionParticlesSub1() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES_S1; i++) {
		if (!explosionParticlesS1[i].active) continue;
		explosionParticlesS1[i].x += explosionParticlesS1[i].vx;
		explosionParticlesS1[i].y += explosionParticlesS1[i].vy;
		explosionParticlesS1[i].vy -= 0.1f;
		explosionParticlesS1[i].life--;
		explosionParticlesS1[i].radius -= 0.15f;
		if (explosionParticlesS1[i].life <= 0 || explosionParticlesS1[i].radius <= 0)
			explosionParticlesS1[i].active = false;
	}
}

static void checkFireballCollisionSub1() {
	if (hitCooldownSub1 > 0) return;

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS_S1; i++) {
		if (!fireballsS1[i].active) continue;

		bool collideX = (px + pw > fireballsS1[i].x) && (px < fireballsS1[i].x + fireballsS1[i].width);
		bool collideY = (py + ph > fireballsS1[i].y) && (py < fireballsS1[i].y + fireballsS1[i].height);

		if (collideX && collideY) {
			playerLivesSub1--;
			if (playerLivesSub1 <= 0) { playerLivesSub1 = 0; isGameOverSub1 = true; }
			hitCooldownSub1 = 60;
			spawnExplosionSub1(fireballsS1[i].x + 10, fireballsS1[i].y + 10);
			fireballsS1[i].active = false;
			break;
		}
	}
}

static void drawFireballsSub1() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES_S1; i++) {
		if (!trailParticlesS1[i].active) continue;
		int alpha = trailParticlesS1[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticlesS1[i].x, trailParticlesS1[i].y, trailParticlesS1[i].radius, 20);
	}

	for (int i = 0; i < MAX_EXPLOSION_PARTICLES_S1; i++) {
		if (!explosionParticlesS1[i].active) continue;
		if (explosionParticlesS1[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticlesS1[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticlesS1[i].x, explosionParticlesS1[i].y, explosionParticlesS1[i].radius, 20);
	}

	for (int i = 0; i < MAX_FIREBALLS_S1; i++) {
		if (!fireballsS1[i].active) continue;
		float cx = fireballsS1[i].x + 10;
		float cy = fireballsS1[i].y + 10;

		if (fireballTextureSub1 > 0) {
			iRotate(cx, cy, fireballsS1[i].rotation);
			iShowImage(fireballsS1[i].x, fireballsS1[i].y, fireballsS1[i].width, fireballsS1[i].height, fireballTextureSub1);
			iUnRotate();
		}
		else {
			iSetColor(200, 50, 0);    iFilledCircle(cx, cy, 14, 50);
			iSetColor(255, 120, 0);   iFilledCircle(cx, cy, 10, 50);
			iSetColor(255, 255, 150); iFilledCircle(cx, cy, 5, 50);
		}
	}
}

static void checkDragonPlayerCollisionSub1() {
	if (hitCooldownSub1 > 0) { hitCooldownSub1--; return; }

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < DRAGON_COUNT_S1; i++) {
		int hx = dragonsSub1[i].x;
		int hy = dragonsSub1[i].y;
		int hw = dragonsSub1[i].width;
		int hh = dragonsSub1[i].height;

		if (dragonsSub1[i].animFrame >= 1 && dragonsSub1[i].animFrame <= 17) {
			hx = dragonsSub1[i].x + (int)(dragonsSub1[i].width * 0.556f);
			hy = dragonsSub1[i].y + (int)(dragonsSub1[i].height * 0.312f);
			hw = (int)(dragonsSub1[i].width * 0.363f);
			hh = (int)(dragonsSub1[i].height * 0.347f);
		}
		else if (dragonsSub1[i].animFrame >= DRAGON_RIGHT_START_S1 && dragonsSub1[i].animFrame <= DRAGON_RIGHT_END_S1) {
			hx = dragonsSub1[i].x + (int)(dragonsSub1[i].width * 0.081f);
			hy = dragonsSub1[i].y + (int)(dragonsSub1[i].height * 0.312f);
			hw = (int)(dragonsSub1[i].width * 0.363f);
			hh = (int)(dragonsSub1[i].height * 0.347f);
		}

		bool collideX = (px + pw > hx) && (px < hx + hw);
		bool collideY = (py + ph > hy) && (py < hy + hh);

		if (collideX && collideY) {
			playerLivesSub1--;
			if (playerLivesSub1 <= 0) { playerLivesSub1 = 0; isGameOverSub1 = true; }
			hitCooldownSub1 = 30;
			break;
		}
	}
}

static void initLevel3Sub1(bool freshStart = true) {
	initPlayer(40, LEVEL_BOTTOM_S1 - 10);
	initDragonSub1();

	loadFireballTextureSub1();
	fireballsFiredSub1 = 0;
	for (int i = 0; i < MAX_FIREBALLS_S1; i++) fireballsS1[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES_S1; i++) trailParticlesS1[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES_S1; i++) explosionParticlesS1[i].active = false;

	initGoldAndBlueBallsSub1(freshStart);
	loadPauseTexturesSub1();
	loadZedsTexturesSub1();
	loadEnergyTexturesSub1();

	level3Sub1Complete = false;
	isGameOverSub1 = false;
	isPausedSub1 = false;
	hitCooldownSub1 = 0;

	showLevelSub1ExitTransition = false;
	if (level4TransitionTimerIdSub1 >= 0) {
		iPauseTimer(level4TransitionTimerIdSub1);
	}

	isLevelSub1Transition = false;
	levelSub1TransitionCounter = 0;

	// Zeds + energy refill at the start of every sub level.
	playerLivesSub1 = 5;
	energyFrameSub1 = 1;
	distanceMovedSub1 = 0;
}

static void drawLevel3Sub1Background() {
	iShowBMP(0, 0, IMG_LV3SUB1_BG);
}

// ===========================================================================
// SOLID BLOCKS - stairs, slabs and walls can't be walked through from any side.
// ===========================================================================
// Standing height fine-tune (pixels). 0 = the bottom of the player's hitbox
// (getPlayerHitbox) rests exactly on the block top. Raise it if the character
// sinks into the blocks, lower it (negative) if it floats above them.
#define FEET_TWEAK_S1 0

// Small rises the player may simply walk up (like a real stair). Anything taller
// is a wall. 0 = every block is a wall.
#define STEP_UP_S1 0

static void getSolidSub1(int i, int &x1, int &y1, int &x2, int &y2) {
	Platform p = level3_sub1_platforms[i];
	x1 = p.x1; y1 = p.y1; x2 = p.x2; y2 = p.y2;
}

// HORIZONTAL pass - call right after player.x has been changed, BEFORE gravity
// moves player.y. Blocks are solid on their sides: the player is pushed back so
// the hitbox just touches the block, or (for a tiny rise) walks up onto it.
static void resolveSolidsXSub1(int oldX) {
	for (int iter = 0; iter < 4; iter++) {
		int px, py, pw, ph;
		getPlayerHitbox(px, py, pw, ph);
		int hx = px - player.x;                       // hitbox left offset
		int feetOff = (py - player.y) + FEET_TWEAK_S1;
		int foot = player.y + feetOff;
		int head = foot + ph;

		bool acted = false;
		for (int i = 0; i < PLATFORM_COUNT_L3SUB1; i++) {
			int x1, y1, x2, y2;
			getSolidSub1(i, x1, y1, x2, y2);
			if (!((px + pw > x1) && (px < x2) && (foot < y2) && (head > y1))) continue;

			int rise = y2 - foot;
			if (rise > 0 && rise <= STEP_UP_S1) {
				player.y = y2 - feetOff;                  // walk up the small step
				player.velocityY = 0;
				player.onGround = true;
				player.jumping = false;
				acted = true;
			}
			else if (player.x > oldX) {
				player.x = x1 - pw - hx;                  // blocked from the left side
				acted = true;
			}
			else if (player.x < oldX) {
				player.x = x2 - hx;                       // blocked from the right side
				acted = true;
			}
			if (acted) break;
		}
		if (!acted) break;
	}
}

// VERTICAL pass - call after player.y has been changed by gravity / jumping.
// Lands on top of a block (feet on its top edge) or bumps the head on its underside.
// If several blocks are crossed in one frame the nearest one wins.
static void resolvePlatformCollisionSub1() {
	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	int feetOff = (py - player.y) + FEET_TWEAK_S1;   // sprite bottom -> feet
	int oldY = player.y - player.velocityY;
	int footNow = player.y + feetOff;
	int footOld = oldY + feetOff;
	int headNow = footNow + ph;
	int headOld = footOld + ph;

	if (player.y <= (LEVEL_BOTTOM_S1 - 10)) {
		player.y = LEVEL_BOTTOM_S1 - 10;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	player.onGround = false;

	if (player.velocityY <= 0) {
		int bestTop = -100000;
		for (int i = 0; i < PLATFORM_COUNT_L3SUB1; i++) {
			int x1, y1, x2, y2;
			getSolidSub1(i, x1, y1, x2, y2);
			if (!((px + pw > x1) && (px < x2))) continue;
			if (footOld >= y2 && footNow <= y2 && y2 > bestTop) bestTop = y2;
		}
		if (bestTop > -100000) {
			player.y = bestTop - feetOff;
			player.velocityY = 0;
			player.onGround = true;
			player.jumping = false;
		}
	}
	else {
		int bestBottom = 100000;
		for (int i = 0; i < PLATFORM_COUNT_L3SUB1; i++) {
			int x1, y1, x2, y2;
			getSolidSub1(i, x1, y1, x2, y2);
			if (!((px + pw > x1) && (px < x2))) continue;
			if (headOld <= y1 && headNow >= y1 && y1 < bestBottom) bestBottom = y1;
		}
		if (bestBottom < 100000) {
			player.y = bestBottom - feetOff - ph;
			player.velocityY = 0;
		}
	}
}

static void updateLevel3Sub1() {
	if (isLevelSub1Transition) {
		levelSub1TransitionCounter++;
		if (levelSub1TransitionCounter >= 100) {
			isLevelSub1Transition = false;
		}
		return;
	}

	if (isGameOverSub1 || isPausedSub1 || showLevelSub1ExitTransition) return;

	int oldX = player.x;
	player.isMoving = false;

	if (isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT)) {
		player.x -= MOVE_SPEED;
		player.facingRight = false;
		player.isMoving = true;
	}
	if (isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
		player.x += MOVE_SPEED;
		player.facingRight = true;
		player.isMoving = true;
	}

	if (player.x < LEVEL_LEFT_S1) player.x = LEVEL_LEFT_S1;

	if (player.x + player.width > LEVEL_RIGHT_S1) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level3Sub1Complete && atPortalCoordinates)) {
			player.x = LEVEL_RIGHT_S1 - player.width;
		}
	}

	// Every block (floors, walls, stairs) is solid on its sides too.
	resolveSolidsXSub1(oldX);

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMovedSub1 += moveDist;
		while (distanceMovedSub1 >= 150) {
			distanceMovedSub1 -= 150;
			if (energyFrameSub1 < 145) {
				energyFrameSub1++;
			}
		}
		if (energyFrameSub1 >= 145) {
			isGameOverSub1 = true;
		}
	}

	if ((isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP)) && player.onGround) {
		player.velocityY = JUMP_FORCE;
		player.onGround = false;
		player.jumping = true; requestJumpSfx();
	}

	player.velocityY -= GRAVITY;
	if (player.velocityY < -MAX_FALL_SPEED)
		player.velocityY = -MAX_FALL_SPEED;

	player.y += player.velocityY;

	resolvePlatformCollisionSub1();

	if (player.y + player.height > LEVEL_TOP_S1) {
		player.y = LEVEL_TOP_S1 - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateDragonSub1();

	updateFireballsSub1();
	updateTrailParticlesSub1();
	updateExplosionParticlesSub1();
	checkFireballCollisionSub1();

	checkDragonPlayerCollisionSub1();
	checkGoldBallCollisionSub1();
	checkBlueBallCollisionSub1();
	checkPortalCollisionSub1();
}

static void drawPauseMenuSub1() {
	drawPauseMenuGeneric(resumeTextureSub1, pauseResumeBtnSub1, restartTextureSub1, pauseRestartBtnSub1, exitTextureSub1, pauseExitBtnSub1);
}

static void drawLevel3Sub1() {
	drawLevel3Sub1Background();

	drawDragonSub1();
	drawFireballsSub1();
	drawGoldBallsSub1();
	drawBlueBallsSub1();
	drawPortalSub1();
	drawPlayer();

	drawHUDSub1();
	drawGoldBallHUDSub1();
	drawPointsHUDSub1();

	if (isPausedSub1) {
		drawPauseMenuSub1();
	}
	else if (showLevelSub1ExitTransition) {
		// No banner: this stage continues straight into sub 2.
	}
	else if (isGameOverSub1) {
		drawTotalPointsBoxGeneric(totalPointsTextureSub1, playerPointsSub1, 405);

		if (gameOverTextureSub1 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTextureSub1);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTextureSub1 > 0)
			iShowImage(gameOverRestartBtnSub1.x1, gameOverRestartBtnSub1.y1, 220, 50, gameOverRestartTextureSub1);

		if (gameOverExitTextureSub1 > 0)
			iShowImage(gameOverExitBtnSub1.x1, gameOverExitBtnSub1.y1, 150, 35, gameOverExitTextureSub1);
	}
}

#endif