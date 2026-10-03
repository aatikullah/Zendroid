#ifndef LEVEL3_SUB2_H
#define LEVEL3_SUB2_H
#define IMG_LV3SUB2_BG "Images/lv_3_sub2bg.bmp"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// ============================================================================
// level3_sub2.h
//
// Second (last) bonus stage of Level 3. Plays after level3_sub1.h, before Level 4.
//
// Level 3 + sub 1 + sub 2 are treated as ONE level, but zeds (lives) and energy
// refill at the start of every sub level (points carry over). No banner is shown when this stage starts;
// the single "Level 3 Completed / Level 4 Starts" banner is shown when the
// portal at the end of THIS stage is entered.
//
// Geometry: every rectangle below is measured off lv_3_sub2bg (700 x 500,
// 1 pixel = 1 game unit, gameY = 500 - pixelY). Each ember block is ~24 x 27:
//   - a 6-block row high on the left            -> top surface y = 401
//   - a 3-block ledge on the mid-left           -> top surface y = 243
//   - one lone block in the centre-left         -> top surface y = 348
//   - a 5-step staircase falling to the right   -> tops 240 .. 132
//   - a 4-step staircase climbing to the right  -> tops 267 .. 349
//   - a 6-block staircase falling to the right  -> tops 422 .. 324
//
// Enemies: 3 patrolling dragons, but they fire only 2 fireballs in total
// (MAX_FIREBALL_SHOTS_S2); after that the stage has no more fire.
// ============================================================================

#define PLATFORM_COUNT_L3SUB2 18
static Platform level3_sub2_platforms[PLATFORM_COUNT_L3SUB2] = {
	// { x1 (left), y1 (bottom edge), x2 (right), y2 (walkable top) }
	// Measured from lv_3_sub2bg by template-matching every ember block.
	{ 63, 374, 208, 401 }, // 0  - top-left ember row (6 blocks), GoldBall 0 here
	{ 18, 216, 91, 243 }, // 1  - mid-left ember ledge (3 blocks), GoldBall 1 here
	{ 273, 321, 297, 348 }, // 2  - lone block (link to the top-left row)

	// Staircase falling to the right (from the ledge area down to the floor)
	{ 215, 213, 239, 240 }, // 3  - highest step
	{ 236, 185, 260, 212 }, // 4
	{ 261, 158, 285, 185 }, // 5
	{ 285, 131, 309, 158 }, // 6
	{ 309, 105, 333, 132 }, // 7  - lowest step, reachable straight off the floor

	// Centre staircase, climbing up and to the right
	{ 340, 240, 364, 267 }, // 8  - first step
	{ 362, 267, 386, 294 }, // 9
	{ 386, 294, 410, 321 }, // 10
	{ 407, 322, 431, 349 }, // 11 - centre peak

	// Right staircase, dropping down and to the right towards the portal.
	// These rectangles now sit exactly on the ember blocks drawn in lv_3_sub2bg
	// (template-matched: tops 422, 398, 374, 349, 324, 324). They had been lowered
	// by 40 units while the background art stayed put, so the hitboxes floated
	// below the visible stairs. The jump from the centre peak (top 349) to the
	// first right step (top 422) is a 73 unit rise over a 70 unit gap; with
	// JUMP_FORCE 18 / GRAVITY 1 the player rises ~153 units, so it is easy.
	{ 501, 395, 525, 422 }, // 12 - highest step, hopped across from the peak
	{ 526, 371, 550, 398 }, // 13
	{ 550, 347, 574, 374 }, // 14
	{ 574, 322, 598, 349 }, // 15
	{ 599, 297, 623, 324 }, // 16
	{ 624, 297, 648, 324 }  // 17 - last step, GoldBall 2 here, drop to the portal
};

#define LEVEL_LEFT_S2   20
#define LEVEL_RIGHT_S2  680
#define LEVEL_BOTTOM_S2 20
#define LEVEL_TOP_S2    500   // same headroom as level 1; the top row sits high

// ===== Dragon Frame Boundaries =====
#define DRAGON_LEFT_START_S2   1
#define DRAGON_LEFT_END_S2     17
#define DRAGON_RIGHT_START_S2  31
#define DRAGON_RIGHT_END_S2    63
#define TOTAL_DRAGON_FRAMES_S2 65
#define DRAGON_SPEED_S2        2
#define DRAGON_ANIM_SPEED_S2   3
#define DRAGON_COUNT_S2        3

// ===== Fireball & Particle Variables =====
#define MAX_FIREBALLS_S2           2    // array size (never more than 2 alive at once)
#define MAX_FIREBALL_SHOTS_S2      2    // total fireballs the dragons fire in this stage - only 2, ever
static int fireballsFiredSub2 = 0;        // shots fired so far (reset in initLevel3Sub2)
#define MAX_TRAIL_PARTICLES_S2     60
#define MAX_EXPLOSION_PARTICLES_S2 40

struct FireballS2 { int x, y; int width, height; float vx, vy; float rotation; bool active; };
struct TrailParticleS2 { float x, y; float radius; int alpha; bool active; };
struct ExplosionParticleS2 { float x, y; float vx, vy; float radius; int life; bool active; };

static FireballS2 fireballsS2[MAX_FIREBALLS_S2];
static TrailParticleS2 trailParticlesS2[MAX_TRAIL_PARTICLES_S2];
static ExplosionParticleS2 explosionParticlesS2[MAX_EXPLOSION_PARTICLES_S2];
static int fireballTextureSub2 = 0;

// ===== Game State Variables =====
static bool level3Sub2Complete = false;
static bool isGameOverSub2 = false;
static bool isPausedSub2 = false;
static bool enterLevel4FromSub2 = false;
static int hitCooldownSub2 = 0;

// ----- Exit transition (bonus stage 2 cleared -> level 4 starts) -----
static bool showLevelSub2ExitTransition = false;
static int level3CompletedTextureSub2 = 0;
static int level4StartsTextureSub2 = 0;
static int level4TransitionTimerIdSub2 = -1;

// ===== Player Lives, Energy & Points =====
static int playerLivesSub2 = 5;
static int& energyFrameSub2 = gEnergyFrame;
static int distanceMovedSub2 = 0;
static int energyTexturesSub2[150] = { 0 };
static int& playerPointsSub2 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTextureSub2 = 0;
static int zedsIconTextureSub2 = 0;

// ===== Pause Menu =====
static int resumeTextureSub2 = 0;
static int restartTextureSub2 = 0;
static int exitTextureSub2 = 0;

static Button pauseResumeBtnSub2 = { 260, 280, 440, 325 };
static Button pauseRestartBtnSub2 = { 260, 220, 440, 265 };
static Button pauseExitBtnSub2 = { 260, 160, 440, 205 };

// ===== Game Over Menu =====
static int totalPointsTextureSub2 = 0;
static int gameOverTextureSub2 = 0;
static int gameOverRestartTextureSub2 = 0;
static int gameOverExitTextureSub2 = 0;

static Button gameOverRestartBtnSub2 = { 240, 115, 460, 165 };
static Button gameOverExitBtnSub2 = { 275, 70, 425, 105 };

// ===== Collectibles =====
struct GoldBallSub2 { int x, y; int width, height; bool collected; };
struct BlueBallSub2 { int x, y; int width, height; bool collected; };

static GoldBallSub2 goldBallsSub2[3];
static int goldBallTextureSub2 = 0;
static int goldBallIconTexturesSub2[5] = { 0 };
static int goldBallsCollectedSub2 = 0;
static int dispearTextureSub2 = 0;

static BlueBallSub2 blueBallsSub2[9];
static int blueBallTextureSub2 = 0;
static int pointsTextureSub2 = 0;

// ===== Dragons =====
struct PatrolDragonSub2 {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
	int minX, maxX;
	int shootTimer;
	int shootCooldown;
};

static PatrolDragonSub2 dragonsSub2[DRAGON_COUNT_S2];
static int dragonTexturesSub2[TOTAL_DRAGON_FRAMES_S2];
static bool texturesLoadedSub2 = false;

// ===========================================================================
// TEXTURE LOADING
// ===========================================================================
static void loadDragonTexturesSub2() {
	if (texturesLoadedSub2) return;

	char path[128];
	for (int i = 1; i <= DRAGON_RIGHT_END_S2; i++) {
		if (i >= 18 && i <= 30) { dragonTexturesSub2[i] = 0; continue; }

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTexturesSub2[i] = iLoadImage(path);

		if (dragonTexturesSub2[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTexturesSub2[i] = iLoadImage(path);
		}
	}
	texturesLoadedSub2 = true;
}

static void loadFireballTextureSub2() {
	static bool loaded = false;
	if (loaded) return;
	fireballTextureSub2 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadZedsTexturesSub2() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTextureSub2 = loadTex("Images/zeds.png", "../Images/zeds.png");
	zedsIconTextureSub2 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTextureSub2 <= 0) zedsIconTextureSub2 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTextureSub2 <= 0) zedsIconTextureSub2 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTexturesSub2() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTexturesSub2[i] = iLoadImage(path);

		if (energyTexturesSub2[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTexturesSub2[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadPauseTexturesSub2() {
	static bool loaded = false;
	if (loaded) return;

	resumeTextureSub2 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTextureSub2 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTextureSub2 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTextureSub2 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTextureSub2 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTextureSub2 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTextureSub2 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadExitTransitionTexturesSub2() {
	static bool loaded = false;
	if (loaded) return;
	level3CompletedTextureSub2 = loadTex("Images/level3completed.png", "../Images/level3completed.png");
	level4StartsTextureSub2 = loadTex("Images/level4starts.png", "../Images/level4starts.png");
	loaded = true;
}

static void loadGoldBallTexturesSub2() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTextureSub2 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTextureSub2 = loadTex("Images/dispear3.png", "../Images/dispear3.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTexturesSub2[i] = iLoadImage(path);
		if (goldBallIconTexturesSub2[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTexturesSub2[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadBlueBallAndPointsTexturesSub2() {
	static bool loaded = false;
	if (loaded) return;
	blueBallTextureSub2 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTextureSub2 = loadTex("Images/points.png", "../Images/points.png");
	loaded = true;
}

// ===========================================================================
// COLLECTIBLE PLACEMENT
//
// The three gold balls sit exactly where they were marked on the layout:
// on the top-left ember row, on the left end of the mid-left ledge, and on
// the last step of the right staircase just before the drop to the portal.
// The blue balls are spread one-per-landing along the intended route so
// every one of them is picked up naturally while climbing.
// ===========================================================================
static void initGoldAndBlueBallsSub2(bool freshStart) {
	loadGoldBallTexturesSub2();
	loadBlueBallAndPointsTexturesSub2();
	goldBallsCollectedSub2 = 0;
	if (freshStart) playerPointsSub2 = 0;
	enterLevel4FromSub2 = false;

	// --- Gold balls: on the upper slabs / stairs, resting on the block top (top + 2) ---
	goldBallsSub2[0].width = 25; goldBallsSub2[0].height = 25;
	goldBallsSub2[0].x = 100; goldBallsSub2[0].y = level3_sub2_platforms[0].y2 + 2;   // top-left ember row
	goldBallsSub2[0].collected = false;

	goldBallsSub2[1].width = 25; goldBallsSub2[1].height = 25;
	goldBallsSub2[1].x = 45;  goldBallsSub2[1].y = level3_sub2_platforms[1].y2 + 2;   // mid-left ledge
	goldBallsSub2[1].collected = false;

	goldBallsSub2[2].width = 25; goldBallsSub2[2].height = 25;
	goldBallsSub2[2].x = 600; goldBallsSub2[2].y = level3_sub2_platforms[17].y2 + 2;   // last right-stair step
	goldBallsSub2[2].collected = false;

	// --- Blue balls: one on each meaningful landing spot ---
	struct { int x, y; } scattered[9] = {
		{ 219, 241 },  // falling staircase, step 1
		{ 265, 187 },  // falling staircase, step 3
		{ 313, 134 },  // falling staircase, lowest step
		{ 344, 269 },  // centre staircase, step 1
		{ 391, 323 },  // centre staircase, step 3
		{ 413, 351 },  // centre peak
		{ 278, 350 },  // lone block
		{ 170, 404 },  // top-left ember row (right of the gold ball)
		{ 555, 375 }   // right staircase, step 3
	};

	for (int i = 0; i < 9; i++) {
		blueBallsSub2[i].width = 18;
		blueBallsSub2[i].height = 18;
		blueBallsSub2[i].x = scattered[i].x;
		blueBallsSub2[i].y = scattered[i].y;
		blueBallsSub2[i].collected = false;
	}
}

static void checkGoldBallCollisionSub2() {
	checkGoldBallCollisionGeneric(goldBallsSub2, &goldBallsCollectedSub2, &level3Sub2Complete);
}
static void checkBlueBallCollisionSub2() {
	checkBlueBallCollisionGeneric(blueBallsSub2, &playerPointsSub2);
}
static void drawGoldBallsSub2() { drawGoldBallsGeneric(goldBallsSub2, goldBallTextureSub2); }
static void drawBlueBallsSub2() { drawBlueBallsGeneric(blueBallsSub2, blueBallTextureSub2); }

// ===========================================================================
// PORTAL
// ===========================================================================
static void drawPortalSub2() {
	if (level3Sub2Complete) {
		if (dispearTextureSub2 > 0) {
			iShowImage(680, 20, 35, 100, dispearTextureSub2);
		}
		else {
			iSetColor(255, 255, 0);
			iRectangle(645, 20, 35, 100);
		}
	}
}

static void triggerEnterLevel4FromSub2() {
	enterLevel4FromSub2 = true;
	showLevelSub2ExitTransition = false;
	if (level4TransitionTimerIdSub2 >= 0) iPauseTimer(level4TransitionTimerIdSub2);
}

static void checkPortalCollisionSub2() {
	if (level3Sub2Complete && !showLevelSub2ExitTransition) {
		bool collideX = (player.x + player.width >= 700);
		bool collideY = (player.y <= 120 && player.y + player.height >= 20);

		if (collideX && collideY) {
			showLevelSub2ExitTransition = true;
			loadExitTransitionTexturesSub2();

			if (level4TransitionTimerIdSub2 < 0)
				level4TransitionTimerIdSub2 = iSetTimer(1000, triggerEnterLevel4FromSub2);
			else
				iResumeTimer(level4TransitionTimerIdSub2);
		}
	}
}

static void drawGoldBallHUDSub2() { drawGoldBallHUDGeneric(goldBallsCollectedSub2, goldBallIconTexturesSub2); }
static void drawPointsHUDSub2() { drawPointsHUDGeneric(pointsTextureSub2, playerPointsSub2); }
static void drawHUDSub2() {
	drawHUDGeneric(zedsLabelTextureSub2, zedsIconTextureSub2, playerLivesSub2, energyFrameSub2, energyTexturesSub2);
}

// ===========================================================================
// DRAGONS
//
// All three hover above the slabs (level 3 style): dragon 0 over the top-left
// row, dragon 1 over the lone block / centre staircase, dragon 2 over the high
// right staircase where the last gold ball sits.
// ===========================================================================
static void stepNextFrameSub2(int i) {
	int safetyCounter = 0;
	do {
		dragonsSub2[i].animFrame++;

		if (dragonsSub2[i].movingRight) {
			if (dragonsSub2[i].animFrame > DRAGON_RIGHT_END_S2 || dragonsSub2[i].animFrame < DRAGON_RIGHT_START_S2)
				dragonsSub2[i].animFrame = DRAGON_RIGHT_START_S2;
		}
		else {
			if (dragonsSub2[i].animFrame > DRAGON_LEFT_END_S2 || dragonsSub2[i].animFrame < DRAGON_LEFT_START_S2)
				dragonsSub2[i].animFrame = DRAGON_LEFT_START_S2;
		}

		safetyCounter++;
		if (safetyCounter > TOTAL_DRAGON_FRAMES_S2) break;
	} while (dragonTexturesSub2[dragonsSub2[i].animFrame] <= 0);
}

static void initDragonSub2() {
	loadDragonTexturesSub2();

	// Above the top-left ember row
	dragonsSub2[0].width = 60; dragonsSub2[0].height = 60;
	dragonsSub2[0].x = 60; dragonsSub2[0].y = 415;
	dragonsSub2[0].minX = 40; dragonsSub2[0].maxX = 150;
	dragonsSub2[0].movingRight = true;
	dragonsSub2[0].animFrame = DRAGON_RIGHT_START_S2;
	dragonsSub2[0].animTimer = 0;
	dragonsSub2[0].shootTimer = 100 + (rand() % 50);
	dragonsSub2[0].shootCooldown = 160;
	if (dragonTexturesSub2[dragonsSub2[0].animFrame] <= 0) stepNextFrameSub2(0);

	// Above the lone block and the centre staircase
	dragonsSub2[1].width = 60; dragonsSub2[1].height = 60;
	dragonsSub2[1].x = 300; dragonsSub2[1].y = 360;
	dragonsSub2[1].minX = 270; dragonsSub2[1].maxX = 400;
	dragonsSub2[1].movingRight = true;
	dragonsSub2[1].animFrame = DRAGON_RIGHT_START_S2;
	dragonsSub2[1].animTimer = 0;
	dragonsSub2[1].shootTimer = 130 + (rand() % 60);
	dragonsSub2[1].shootCooldown = 180;
	if (dragonTexturesSub2[dragonsSub2[1].animFrame] <= 0) stepNextFrameSub2(1);

	// Above the high right staircase
	dragonsSub2[2].width = 60; dragonsSub2[2].height = 60;
	dragonsSub2[2].x = 560; dragonsSub2[2].y = 430;
	dragonsSub2[2].minX = 490; dragonsSub2[2].maxX = 590;
	dragonsSub2[2].movingRight = false;
	dragonsSub2[2].animFrame = DRAGON_LEFT_START_S2;
	dragonsSub2[2].animTimer = 0;
	dragonsSub2[2].shootTimer = 90 + (rand() % 60);
	dragonsSub2[2].shootCooldown = 150;
	if (dragonTexturesSub2[dragonsSub2[2].animFrame] <= 0) stepNextFrameSub2(2);
}

static void updateDragonSub2() {
	for (int i = 0; i < DRAGON_COUNT_S2; i++) {
		if (dragonsSub2[i].movingRight) {
			dragonsSub2[i].x += DRAGON_SPEED_S2;
			if (dragonsSub2[i].x >= dragonsSub2[i].maxX) {
				dragonsSub2[i].x = dragonsSub2[i].maxX;
				dragonsSub2[i].movingRight = false;
				dragonsSub2[i].animFrame = DRAGON_LEFT_START_S2;
				if (dragonTexturesSub2[dragonsSub2[i].animFrame] <= 0) stepNextFrameSub2(i);
			}
		}
		else {
			dragonsSub2[i].x -= DRAGON_SPEED_S2;
			if (dragonsSub2[i].x <= dragonsSub2[i].minX) {
				dragonsSub2[i].x = dragonsSub2[i].minX;
				dragonsSub2[i].movingRight = true;
				dragonsSub2[i].animFrame = DRAGON_RIGHT_START_S2;
				if (dragonTexturesSub2[dragonsSub2[i].animFrame] <= 0) stepNextFrameSub2(i);
			}
		}

		dragonsSub2[i].animTimer++;
		if (dragonsSub2[i].animTimer >= DRAGON_ANIM_SPEED_S2) {
			dragonsSub2[i].animTimer = 0;
			stepNextFrameSub2(i);
		}

		// --- Fireball shooting ---
		dragonsSub2[i].shootTimer--;
		if (dragonsSub2[i].shootTimer <= 0) {
			for (int j = 0; j < MAX_FIREBALLS_S2 && fireballsFiredSub2 < MAX_FIREBALL_SHOTS_S2; j++) {
				if (!fireballsS2[j].active) {
					fireballsS2[j].x = dragonsSub2[i].x + dragonsSub2[i].width / 2 - 10;
					fireballsS2[j].y = dragonsSub2[i].y + dragonsSub2[i].height / 2 - 10;
					fireballsS2[j].width = 20; fireballsS2[j].height = 20;

					float dx = (player.x + player.width / 2.0f) - (fireballsS2[j].x + 10);
					float dy = (player.y + player.height / 2.0f) - (fireballsS2[j].y + 10);
					float dist = sqrt(dx * dx + dy * dy);
					if (dist == 0) dist = 1;

					float speed = 7.0f;
					fireballsS2[j].vx = (dx / dist) * speed;
					fireballsS2[j].vy = (dy / dist) * speed + 4.5f; // arc boost
					fireballsS2[j].rotation = 0.0f;
					fireballsS2[j].active = true;
					fireballsFiredSub2++;
					break;
				}
			}
			dragonsSub2[i].shootTimer = dragonsSub2[i].shootCooldown;
		}
	}
}

static void drawDragonSub2() {
	for (int i = 0; i < DRAGON_COUNT_S2; i++) {
		int textureId = dragonTexturesSub2[dragonsSub2[i].animFrame];

		if (textureId > 0) {
			iShowImage(dragonsSub2[i].x, dragonsSub2[i].y, dragonsSub2[i].width, dragonsSub2[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragonsSub2[i].x, dragonsSub2[i].y, dragonsSub2[i].width, dragonsSub2[i].height);
			iSetColor(255, 255, 255);
			iText(dragonsSub2[i].x + 5, dragonsSub2[i].y + 25, "DRAGON", GLUT_BITMAP_HELVETICA_10);
		}
	}
}

// ===========================================================================
// FIREBALLS (arcs, trails, explosions)
// ===========================================================================
static void spawnExplosionSub2(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES_S2; j++) {
			if (!explosionParticlesS2[j].active) {
				explosionParticlesS2[j].x = x;
				explosionParticlesS2[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticlesS2[j].vx = cos(angle) * speed;
				explosionParticlesS2[j].vy = sin(angle) * speed;
				explosionParticlesS2[j].radius = 3.0f + (rand() % 3);
				explosionParticlesS2[j].life = 20 + (rand() % 10);
				explosionParticlesS2[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticleSub2(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES_S2; i++) {
		if (!trailParticlesS2[i].active) {
			trailParticlesS2[i].x = x + (rand() % 6 - 3);
			trailParticlesS2[i].y = y + (rand() % 6 - 3);
			trailParticlesS2[i].radius = 4.0f + (rand() % 4);
			trailParticlesS2[i].alpha = 255;
			trailParticlesS2[i].active = true;
			break;
		}
	}
}

static void updateFireballsSub2() {
	for (int i = 0; i < MAX_FIREBALLS_S2; i++) {
		if (!fireballsS2[i].active) continue;

		int oldY = fireballsS2[i].y;

		fireballsS2[i].vy -= 0.2f;                       // gravity -> parabola
		fireballsS2[i].x += fireballsS2[i].vx;
		fireballsS2[i].y += fireballsS2[i].vy;
		fireballsS2[i].rotation += fireballsS2[i].vx * 6.0f;

		// Bounce off the floor
		if (fireballsS2[i].y <= LEVEL_BOTTOM_S2) {
			fireballsS2[i].y = LEVEL_BOTTOM_S2;
			fireballsS2[i].vy = -fireballsS2[i].vy * 0.5f;
			fireballsS2[i].vx *= 0.8f;
			spawnExplosionSub2(fireballsS2[i].x + 10, fireballsS2[i].y + 10);
			if (fabs(fireballsS2[i].vy) < 1.5f) fireballsS2[i].active = false;
		}

		// Burst against an ember block instead of passing through it
		if (fireballsS2[i].active && fireballsS2[i].vy < 0) {
			for (int p = 0; p < PLATFORM_COUNT_L3SUB2; p++) {
				Platform plat = level3_sub2_platforms[p];

				bool withinX = (fireballsS2[i].x + fireballsS2[i].width > plat.x1) && (fireballsS2[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballsS2[i].y <= plat.y2) {
					fireballsS2[i].y = plat.y2;
					spawnExplosionSub2(fireballsS2[i].x + 10, fireballsS2[i].y + 10);
					fireballsS2[i].active = false;
					break;
				}
			}
		}

		if (fireballsS2[i].x < -50 || fireballsS2[i].x > 750) fireballsS2[i].active = false;

		if (fireballsS2[i].active)
			spawnTrailParticleSub2(fireballsS2[i].x + 10, fireballsS2[i].y + 10);
	}
}

static void updateTrailParticlesSub2() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES_S2; i++) {
		if (!trailParticlesS2[i].active) continue;
		trailParticlesS2[i].alpha -= 15;
		trailParticlesS2[i].radius -= 0.2f;
		if (trailParticlesS2[i].alpha <= 0 || trailParticlesS2[i].radius <= 0)
			trailParticlesS2[i].active = false;
	}
}

static void updateExplosionParticlesSub2() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES_S2; i++) {
		if (!explosionParticlesS2[i].active) continue;
		explosionParticlesS2[i].x += explosionParticlesS2[i].vx;
		explosionParticlesS2[i].y += explosionParticlesS2[i].vy;
		explosionParticlesS2[i].vy -= 0.1f;
		explosionParticlesS2[i].life--;
		explosionParticlesS2[i].radius -= 0.15f;
		if (explosionParticlesS2[i].life <= 0 || explosionParticlesS2[i].radius <= 0)
			explosionParticlesS2[i].active = false;
	}
}

static void checkFireballCollisionSub2() {
	if (hitCooldownSub2 > 0) return;

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS_S2; i++) {
		if (!fireballsS2[i].active) continue;

		bool collideX = (px + pw > fireballsS2[i].x) && (px < fireballsS2[i].x + fireballsS2[i].width);
		bool collideY = (py + ph > fireballsS2[i].y) && (py < fireballsS2[i].y + fireballsS2[i].height);

		if (collideX && collideY) {
			playerLivesSub2--;
			if (playerLivesSub2 <= 0) { playerLivesSub2 = 0; isGameOverSub2 = true; }
			hitCooldownSub2 = 60;
			spawnExplosionSub2(fireballsS2[i].x + 10, fireballsS2[i].y + 10);
			fireballsS2[i].active = false;
			break;
		}
	}
}

static void drawFireballsSub2() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES_S2; i++) {
		if (!trailParticlesS2[i].active) continue;
		int alpha = trailParticlesS2[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticlesS2[i].x, trailParticlesS2[i].y, trailParticlesS2[i].radius, 20);
	}

	for (int i = 0; i < MAX_EXPLOSION_PARTICLES_S2; i++) {
		if (!explosionParticlesS2[i].active) continue;
		if (explosionParticlesS2[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticlesS2[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticlesS2[i].x, explosionParticlesS2[i].y, explosionParticlesS2[i].radius, 20);
	}

	for (int i = 0; i < MAX_FIREBALLS_S2; i++) {
		if (!fireballsS2[i].active) continue;
		float cx = fireballsS2[i].x + 10;
		float cy = fireballsS2[i].y + 10;

		if (fireballTextureSub2 > 0) {
			iRotate(cx, cy, fireballsS2[i].rotation);
			iShowImage(fireballsS2[i].x, fireballsS2[i].y, fireballsS2[i].width, fireballsS2[i].height, fireballTextureSub2);
			iUnRotate();
		}
		else {
			iSetColor(200, 50, 0);    iFilledCircle(cx, cy, 14, 50);
			iSetColor(255, 120, 0);   iFilledCircle(cx, cy, 10, 50);
			iSetColor(255, 255, 150); iFilledCircle(cx, cy, 5, 50);
		}
	}
}

static void checkDragonPlayerCollisionSub2() {
	if (hitCooldownSub2 > 0) { hitCooldownSub2--; return; }

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < DRAGON_COUNT_S2; i++) {
		int hx = dragonsSub2[i].x;
		int hy = dragonsSub2[i].y;
		int hw = dragonsSub2[i].width;
		int hh = dragonsSub2[i].height;

		if (dragonsSub2[i].animFrame >= 1 && dragonsSub2[i].animFrame <= 17) {
			hx = dragonsSub2[i].x + (int)(dragonsSub2[i].width * 0.556f);
			hy = dragonsSub2[i].y + (int)(dragonsSub2[i].height * 0.312f);
			hw = (int)(dragonsSub2[i].width * 0.363f);
			hh = (int)(dragonsSub2[i].height * 0.347f);
		}
		else if (dragonsSub2[i].animFrame >= DRAGON_RIGHT_START_S2 && dragonsSub2[i].animFrame <= DRAGON_RIGHT_END_S2) {
			hx = dragonsSub2[i].x + (int)(dragonsSub2[i].width * 0.081f);
			hy = dragonsSub2[i].y + (int)(dragonsSub2[i].height * 0.312f);
			hw = (int)(dragonsSub2[i].width * 0.363f);
			hh = (int)(dragonsSub2[i].height * 0.347f);
		}

		bool collideX = (px + pw > hx) && (px < hx + hw);
		bool collideY = (py + ph > hy) && (py < hy + hh);

		if (collideX && collideY) {
			playerLivesSub2--;
			if (playerLivesSub2 <= 0) { playerLivesSub2 = 0; isGameOverSub2 = true; }
			hitCooldownSub2 = 30;
			break;
		}
	}
}

// ===========================================================================
// INIT / UPDATE / DRAW
// ===========================================================================
static void initLevel3Sub2(bool freshStart = true) {
	initPlayer(40, LEVEL_BOTTOM_S2 - 10);
	initDragonSub2();

	loadFireballTextureSub2();
	fireballsFiredSub2 = 0;
	for (int i = 0; i < MAX_FIREBALLS_S2; i++) fireballsS2[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES_S2; i++) trailParticlesS2[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES_S2; i++) explosionParticlesS2[i].active = false;

	initGoldAndBlueBallsSub2(freshStart);
	loadPauseTexturesSub2();
	loadZedsTexturesSub2();
	loadEnergyTexturesSub2();
	loadExitTransitionTexturesSub2();

	level3Sub2Complete = false;
	isGameOverSub2 = false;
	isPausedSub2 = false;
	hitCooldownSub2 = 0;

	showLevelSub2ExitTransition = false;
	if (level4TransitionTimerIdSub2 >= 0) iPauseTimer(level4TransitionTimerIdSub2);

	// Zeds + energy refill at the start of every sub level.
	playerLivesSub2 = 5;
	energyFrameSub2 = 1;
	distanceMovedSub2 = 0;
}

static void drawLevel3Sub2Background() { iShowBMP(0, 0, IMG_LV3SUB2_BG); }

// ===========================================================================
// SOLID BLOCKS - stairs, slabs and walls can't be walked through from any side.
// ===========================================================================
// Standing height fine-tune (pixels). 0 = the bottom of the player's hitbox
// (getPlayerHitbox) rests exactly on the block top. Raise it if the character
// sinks into the blocks, lower it (negative) if it floats above them.
#define FEET_TWEAK_S2 0

// Small rises the player may simply walk up (like a real stair). Anything taller
// is a wall. 0 = every block is a wall.
#define STEP_UP_S2 0

static void getSolidSub2(int i, int &x1, int &y1, int &x2, int &y2) {
	Platform p = level3_sub2_platforms[i];
	x1 = p.x1; y1 = p.y1; x2 = p.x2; y2 = p.y2;
}

// HORIZONTAL pass - call right after player.x has been changed, BEFORE gravity
// moves player.y. Blocks are solid on their sides: the player is pushed back so
// the hitbox just touches the block, or (for a tiny rise) walks up onto it.
static void resolveSolidsXSub2(int oldX) {
	for (int iter = 0; iter < 4; iter++) {
		int px, py, pw, ph;
		getPlayerHitbox(px, py, pw, ph);
		int hx = px - player.x;                       // hitbox left offset
		int feetOff = (py - player.y) + FEET_TWEAK_S2;
		int foot = player.y + feetOff;
		int head = foot + ph;

		bool acted = false;
		for (int i = 0; i < PLATFORM_COUNT_L3SUB2; i++) {
			int x1, y1, x2, y2;
			getSolidSub2(i, x1, y1, x2, y2);
			if (!((px + pw > x1) && (px < x2) && (foot < y2) && (head > y1))) continue;

			int rise = y2 - foot;
			if (rise > 0 && rise <= STEP_UP_S2) {
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
static void resolvePlatformCollisionSub2() {
	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	int feetOff = (py - player.y) + FEET_TWEAK_S2;   // sprite bottom -> feet
	int oldY = player.y - player.velocityY;
	int footNow = player.y + feetOff;
	int footOld = oldY + feetOff;
	int headNow = footNow + ph;
	int headOld = footOld + ph;

	if (player.y <= (LEVEL_BOTTOM_S2 - 10)) {
		player.y = LEVEL_BOTTOM_S2 - 10;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	player.onGround = false;

	if (player.velocityY <= 0) {
		int bestTop = -100000;
		for (int i = 0; i < PLATFORM_COUNT_L3SUB2; i++) {
			int x1, y1, x2, y2;
			getSolidSub2(i, x1, y1, x2, y2);
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
		for (int i = 0; i < PLATFORM_COUNT_L3SUB2; i++) {
			int x1, y1, x2, y2;
			getSolidSub2(i, x1, y1, x2, y2);
			if (!((px + pw > x1) && (px < x2))) continue;
			if (headOld <= y1 && headNow >= y1 && y1 < bestBottom) bestBottom = y1;
		}
		if (bestBottom < 100000) {
			player.y = bestBottom - feetOff - ph;
			player.velocityY = 0;
		}
	}
}

static void updateLevel3Sub2() {
	if (isGameOverSub2 || isPausedSub2 || showLevelSub2ExitTransition) return;

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

	if (player.x < LEVEL_LEFT_S2) player.x = LEVEL_LEFT_S2;

	if (player.x + player.width > LEVEL_RIGHT_S2) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level3Sub2Complete && atPortalCoordinates))
			player.x = LEVEL_RIGHT_S2 - player.width;
	}

	// Every block (ledges, stairs) is solid on its sides too.
	resolveSolidsXSub2(oldX);

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMovedSub2 += moveDist;
		while (distanceMovedSub2 >= 150) {
			distanceMovedSub2 -= 150;
			if (energyFrameSub2 < 145) energyFrameSub2++;
		}
		if (energyFrameSub2 >= 145) isGameOverSub2 = true;
	}

	if ((isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP)) && player.onGround) {
		player.velocityY = JUMP_FORCE;
		player.onGround = false;
		player.jumping = true; requestJumpSfx();
	}

	player.velocityY -= GRAVITY;
	if (player.velocityY < -MAX_FALL_SPEED) player.velocityY = -MAX_FALL_SPEED;
	player.y += player.velocityY;

	resolvePlatformCollisionSub2();

	if (player.y + player.height > LEVEL_TOP_S2) {
		player.y = LEVEL_TOP_S2 - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateDragonSub2();

	updateFireballsSub2();
	updateTrailParticlesSub2();
	updateExplosionParticlesSub2();
	checkFireballCollisionSub2();

	checkDragonPlayerCollisionSub2();
	checkGoldBallCollisionSub2();
	checkBlueBallCollisionSub2();
	checkPortalCollisionSub2();
}

static void drawPauseMenuSub2() {
	drawPauseMenuGeneric(resumeTextureSub2, pauseResumeBtnSub2, restartTextureSub2, pauseRestartBtnSub2, exitTextureSub2, pauseExitBtnSub2);
}

static void drawLevel3Sub2() {
	drawLevel3Sub2Background();

	drawDragonSub2();
	drawFireballsSub2();
	drawGoldBallsSub2();
	drawBlueBallsSub2();
	drawPortalSub2();
	drawPlayer();

	drawHUDSub2();
	drawGoldBallHUDSub2();
	drawPointsHUDSub2();

	if (isPausedSub2) {
		drawPauseMenuSub2();
	}
	else if (showLevelSub2ExitTransition) {
		// The one and only "Level 3 Completed / Level 4 Starts" banner: all three parts are done.
		if (level3CompletedTextureSub2 > 0) {
			iShowImage(165, 270, 350, 70, level3CompletedTextureSub2);
		}
		else {
			iSetColor(255, 255, 255);
			iText(265, 305, "Level 3 Completed", GLUT_BITMAP_HELVETICA_18);
		}

		if (level4StartsTextureSub2 > 0) {
			iShowImage(190, 190, 300, 60, level4StartsTextureSub2);
		}
		else {
			iSetColor(255, 255, 255);
			iText(290, 215, "Level 4 Starts", GLUT_BITMAP_HELVETICA_18);
		}
	}
	else if (isGameOverSub2) {
		drawTotalPointsBoxGeneric(totalPointsTextureSub2, playerPointsSub2, 405);

		if (gameOverTextureSub2 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTextureSub2);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTextureSub2 > 0)
			iShowImage(gameOverRestartBtnSub2.x1, gameOverRestartBtnSub2.y1, 220, 50, gameOverRestartTextureSub2);

		if (gameOverExitTextureSub2 > 0)
			iShowImage(gameOverExitBtnSub2.x1, gameOverExitBtnSub2.y1, 150, 35, gameOverExitTextureSub2);
	}
}

#endif // LEVEL3_SUB2_H