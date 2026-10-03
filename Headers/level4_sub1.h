#ifndef LEVEL4_SUB1_H
#define LEVEL4_SUB1_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// ============================================================================
// level4_sub1.h
//
// First bonus stage after Level 4. It re-uses Level 4's background
// (lv4_bg.bmp) and every gameplay rule from level4.h:
//   - 150 x 23 floating slabs that patrol horizontally / vertically and
//     carry the player (and any collectible sitting on them) along,
//   - the hold-left-mouse 4-punch combo (2 punches = 1 dragon life,
//     5 lives per dragon),
//   - killing a dragon drops one gold ball; 3 gold balls opens the portal,
//   - 9 blue balls worth 10 points each,
//   - Zeds / energy HUD, pause menu, game-over screen.
//
// Only the LAYOUT is new: the slab rest positions and their patrol ranges
// are completely different from level4.h (and from level4_sub2.h), and no
// two slabs ever occupy the same band of the screen, so they can never
// overlap each other at any point of their travel.
//
// Slab map (x1,y1 - x2,y2 are the rest positions; travel ranges in comments)
//   Slab 1  low-left      horizontal  x 30 .. 250   at y 110..133
//   Slab 2  right tower   vertical    y 150 .. 330  at x 500..650
//   Slab 3  mid bridge    horizontal  x 200 .. 470  at y 230..253
//   Slab 4  left tower    vertical    y 260 .. 400  at x  60..210
// ============================================================================

#define IMG_LV4SUB1_BG "Images/lv4_bg.bmp"

#define L4S1_PLATFORM_COUNT 5
#define L4S1_GROUND   0
#define L4S1_SLAB1    1
#define L4S1_SLAB2    2
#define L4S1_SLAB3    3
#define L4S1_SLAB4    4

// Slab size is kept at 150 x 23 so slabs.png keeps its original proportions.
static Platform level4sub1_platforms[L4S1_PLATFORM_COUNT] = {
	{ 20, 20, 680, 20 },   // Fixed bottom ground (not drawn)
	{ 30, 110, 180, 133 },   // Slab 1: horizontal, travels x 30 .. 250
	{ 500, 150, 650, 173 },  // Slab 2: vertical,   travels y 150 .. 330
	{ 320, 230, 470, 253 },  // Slab 3: horizontal, travels x 200 .. 470
	{ 60, 300, 210, 323 }    // Slab 4: vertical,   travels y 260 .. 400
};

static const Platform level4sub1_platformStart[L4S1_PLATFORM_COUNT] = {
	{ 20, 20, 680, 70 },
	{ 30, 110, 180, 133 },
	{ 500, 150, 650, 173 },
	{ 320, 230, 470, 253 },
	{ 60, 300, 210, 323 }
};

// Patrol limits, one entry per platform (index 0 = ground, unused).
static const int L4S1_SLAB_MIN[L4S1_PLATFORM_COUNT] = { 0, 30, 150, 200, 260 };
static const int L4S1_SLAB_MAX[L4S1_PLATFORM_COUNT] = { 0, 250, 353, 470, 423 };

static int slabDirection4S1[L4S1_PLATFORM_COUNT] = { 0, 1, 1, -1, 1 };
// Slabs move ~20% faster than the original 2 px/frame (2 * 1.2 = 2.4).
// Kept as a float speed + per-slab fractional carry so the extra 0.4
// px/frame accumulates precisely instead of being rounded away every
// frame (see slabStep4S1()).
#define SLAB_SPEED_SCALE4S1 1.2f
static float slabSpeed4S1[L4S1_PLATFORM_COUNT] = { 0.0f, 2 * SLAB_SPEED_SCALE4S1, 2 * SLAB_SPEED_SCALE4S1, 2 * SLAB_SPEED_SCALE4S1, 2 * SLAB_SPEED_SCALE4S1 };
static float slabCarry4S1[L4S1_PLATFORM_COUNT] = { 0.0f };
static int slabDeltaX4S1[L4S1_PLATFORM_COUNT] = { 0 };
static int slabDeltaY4S1[L4S1_PLATFORM_COUNT] = { 0 };

static int slabTexture4S1 = 0;

#define L4S1_LEFT   20
#define L4S1_RIGHT  680
#define L4S1_BOTTOM 25
#define L4S1_TOP    480

// ===== Dragon Frame Boundaries =====
#define D4S1_LEFT_START    1
#define D4S1_LEFT_END      17
#define D4S1_RIGHT_START   31
#define D4S1_RIGHT_END     63
#define D4S1_TOTAL_FRAMES  65

#define D4S1_SPEED         2
#define D4S1_ANIM_SPEED    3
#define D4S1_COUNT         3

static int dragonHealthBarTextures4S1[6] = { 0 };

// ===== Fireball Breath Attack =====
// The dragons throw a fireball at the player whenever their flight
// animation reaches frame 16 (the last frame of the left-facing cycle)
// or frame 54 (mid right-facing cycle) - the "mouth open / breathing
// fire" poses in the dragon sprite sheet.
#define FIREBALL_ANIM_FRAME_A_S1 16
#define FIREBALL_ANIM_FRAME_B_S1 54

#define MAX_FIREBALLS4S1 10
#define MAX_TRAIL_PARTICLES4S1 60
#define MAX_EXPLOSION_PARTICLES4S1 40

struct Fireball4S1 {
	int x, y;
	int width, height;
	float vx, vy;
	float rotation;
	bool active;
};

struct TrailParticle4S1 {
	float x, y;
	float radius;
	int alpha;
	bool active;
};

struct ExplosionParticle4S1 {
	float x, y;
	float vx, vy;
	float radius;
	int life;
	bool active;
};

static Fireball4S1 fireballs4S1[MAX_FIREBALLS4S1];
static TrailParticle4S1 trailParticles4S1[MAX_TRAIL_PARTICLES4S1];
static ExplosionParticle4S1 explosionParticles4S1[MAX_EXPLOSION_PARTICLES4S1];
static int fireballTexture4S1 = 0;

// Fireball hits needed to drop 1 zed. Kept separate from the melee-touch
// counter below so the two damage sources never interfere with each other.
static int fireballHitsToPlayer4S1 = 0;
static int fireballHitCooldown4S1 = 0;

// ===== Fighting & Combo Variables =====
#define F4S1_FRAMES      LASER_FRAMES
#define F4S1_FRAMES_ALL  LASER_FRAMES_ALL
#define F4S1_ANIM_SPEED  7

static int fightingTextures4S1[F4S1_FRAMES_ALL + 1] = { 0 };
static bool isFighting4S1 = false;
static bool isLeftMouseDown4S1 = false;
static int currentPunchCombo4S1 = 0;
static int fightFrame4S1 = 1;
static int fightAnimTimer4S1 = 0;
static bool punchDamageDealt4S1 = false;

// ===== Game State Variables =====
static bool level4Sub1Complete = false;
static bool isGameOver4S1 = false;
static bool isPaused4S1 = false;
static bool enterLevel4Sub2 = false;
static int hitCooldown4S1 = 0;
static int dragonHitsToPlayer4S1 = 0;

// ----- Exit transition (bonus stage 1 done -> bonus stage 2 starts) -----
static bool showLevel4Sub1ExitTransition = false;
static int level4CompletedTexture4S1 = 0;
static int level4Sub2TransitionTimerId4S1 = -1;

// ----- Intro transition (level 4 completed -> bonus stage 1 starts) -----
// Disabled: per design, no banner is shown between Level 4 and this bonus
// stage - gameplay starts immediately. The flag/texture stay in place
// (unused) so the draw/update code below doesn't need to be torn out.
static bool isLevel4Sub1Transition = false;
static int level4Sub1TransitionCounter = 0;
static int level4IntroTexture4S1 = 0;

// ===== Player Lives, Energy & Points Variables =====
static int playerLives4S1 = 5;
static int& energyFrame4S1 = gEnergyFrame;
static int distanceMoved4S1 = 0;
static int energyTextures4S1[150] = { 0 };
static int& playerPoints4S1 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTexture4S1 = 0;
static int zedsIconTexture4S1 = 0;

// ===== Menu Textures & Bounding Boxes =====
static int resumeTexture4S1 = 0;
static int restartTexture4S1 = 0;
static int exitTexture4S1 = 0;

static Button pauseResumeBtn4S1 = { 260, 280, 440, 325 };
static Button pauseRestartBtn4S1 = { 260, 220, 440, 265 };
static Button pauseExitBtn4S1 = { 260, 160, 440, 205 };

// ===== Game Over Menu Textures & Bounding Boxes =====
static int totalPointsTexture4S1 = 0;
static int gameOverTexture4S1 = 0;
static int gameOverRestartTexture4S1 = 0;
static int gameOverExitTexture4S1 = 0;

static Button gameOverRestartBtn4S1 = { 240, 115, 460, 165 };
static Button gameOverExitBtn4S1 = { 275, 70, 425, 105 };

// ===== Gold Ball Collectibles, Portal & HUD =====
struct GoldBall4S1 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static GoldBall4S1 goldBalls4S1[3];
static int goldBallTexture4S1 = 0;
static int goldBallIconTextures4S1[5] = { 0 };
static int goldBallsCollected4S1 = 0;
static int dispearTexture4S1 = 0;

// ===== Blue Ball Collectibles & Points HUD =====
struct BlueBall4S1 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static BlueBall4S1 blueBalls4S1[9];
static int blueBallTexture4S1 = 0;
static int pointsTexture4S1 = 0;

struct PatrolDragon4S1 {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
	int health;      // Starts at 5 (5 lives)
	int punchCount;  // 5 laser hits needed per life loss
	bool alive;
	int platformIndex;
	int minX, maxX;
};

static PatrolDragon4S1 dragons4S1[D4S1_COUNT];

static int dragonTextures4S1[D4S1_TOTAL_FRAMES];
static bool texturesLoaded4S1 = false;

// ---------------------------------------------------------------------------
// Texture loading
// ---------------------------------------------------------------------------
static void loadDragonHealthBarTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 5; i++) {
		sprintf(path, "Images/Dragonhealthbar/%d.png", i);
		dragonHealthBarTextures4S1[i] = iLoadImage(path);

		if (dragonHealthBarTextures4S1[i] <= 0) {
			sprintf(path, "../Images/Dragonhealthbar/%d.png", i);
			dragonHealthBarTextures4S1[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadDragonTextures4S1() {
	if (texturesLoaded4S1) return;

	char path[128];
	for (int i = 1; i <= D4S1_RIGHT_END; i++) {
		if (i >= 18 && i <= 30) {
			dragonTextures4S1[i] = 0;
			continue;
		}

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTextures4S1[i] = iLoadImage(path);

		if (dragonTextures4S1[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTextures4S1[i] = iLoadImage(path);
		}
	}

	texturesLoaded4S1 = true;
}

static void loadFireballTexture4S1() {
	static bool loaded = false;
	if (loaded) return;
	fireballTexture4S1 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadFightingTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= F4S1_FRAMES_ALL; i++) {
		sprintf(path, "Images/PlayerLaser/shoot/%d.png", i);
		fightingTextures4S1[i] = iLoadImage(path);

		if (fightingTextures4S1[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/shoot/%d.png", i);
			fightingTextures4S1[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

// Laser attack frames: 1-10 face right, 11-20 are their mirrors (facing left).
static int getFightTextureIndex4S1(int frame, bool facingRight) {
	return laserTextureIndex(frame, facingRight);
}

static void loadTransitionTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	level4IntroTexture4S1 = loadTex("Images/level4completed.png", "../Images/level4completed.png");

	loaded = true;
}

static void loadExitTransitionTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	level4CompletedTexture4S1 = loadTex("Images/level4completed.png", "../Images/level4completed.png");

	loaded = true;
}

static void loadZedsTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTexture4S1 = loadTex("Images/zeds.png", "../Images/zeds.png");

	zedsIconTexture4S1 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture4S1 <= 0) zedsIconTexture4S1 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture4S1 <= 0) zedsIconTexture4S1 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures4S1[i] = iLoadImage(path);

		if (energyTextures4S1[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures4S1[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadSlabTexture4S1() {
	static bool loaded = false;
	if (loaded) return;

	slabTexture4S1 = loadTex("Images/slabs.png", "../Images/slabs.png");
	loaded = true;
}

static void loadPauseTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	resumeTexture4S1 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture4S1 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture4S1 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTexture4S1 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTexture4S1 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture4S1 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture4S1 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadGoldBallTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTexture4S1 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture4S1 = loadTex("Images/dispear4.png", "../Images/dispear4.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures4S1[i] = iLoadImage(path);
		if (goldBallIconTextures4S1[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures4S1[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadBlueBallAndPointsTextures4S1() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTexture4S1 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture4S1 = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

// ---------------------------------------------------------------------------
// Collectibles
//
// Gold balls are parked off-screen at start - exactly like level4.h - and are
// only placed on the map when a dragon dies (one ball per dragon).
// Blue balls: 3 ride the slabs, 6 hang in mid-air.
// ---------------------------------------------------------------------------
static void initGoldAndBlueBalls4S1(bool freshStart) {
	loadGoldBallTextures4S1();
	loadBlueBallAndPointsTextures4S1();
	goldBallsCollected4S1 = 0;
	if (freshStart) playerPoints4S1 = 0;
	enterLevel4Sub2 = false;

	struct ItemRect {
		int x, y, w, h;
		int platformIndex;
	};
	ItemRect placed[12];
	int placedCount = 0;

	const int MIN_DIST = 45; // Pixel clearance required between any two collectibles

	// 1. GOLD BALLS (off-screen until their dragon is killed)
	for (int i = 0; i < 3; i++) {
		goldBalls4S1[i].width = 25;
		goldBalls4S1[i].height = 25;
		goldBalls4S1[i].x = -1000;
		goldBalls4S1[i].y = -1000;
		goldBalls4S1[i].collected = false;
		goldBalls4S1[i].platformIndex = L4S1_GROUND;
	}

	// 2. BLUE BALLS ON SLABS (3)
	for (int i = 0; i < 3; i++) {
		bool valid = false;
		int rx = 0, ry = 0;
		int attempts = 0;
		int pIdx = L4S1_SLAB1;

		while (!valid && attempts < 300) {
			attempts++;
			pIdx = L4S1_SLAB1 + (rand() % 4);
			Platform p = level4sub1_platforms[pIdx];
			int minX = p.x1 + 15;
			int maxX = p.x2 - 25;
			if (maxX <= minX) maxX = minX + 1;
			rx = minX + (rand() % (maxX - minX));
			ry = p.y2 + 5;

			valid = true;
			for (int j = 0; j < placedCount; j++) {
				int dx = rx - placed[j].x;
				int dy = ry - placed[j].y;
				if ((dx * dx + dy * dy) < (MIN_DIST * MIN_DIST)) {
					valid = false;
					break;
				}
			}
		}

		blueBalls4S1[i].width = 18;
		blueBalls4S1[i].height = 18;
		blueBalls4S1[i].x = rx;
		blueBalls4S1[i].y = ry;
		blueBalls4S1[i].collected = false;
		blueBalls4S1[i].platformIndex = pIdx;

		placed[placedCount++] = { rx, ry, 18, 18, pIdx };
	}

	// 3. BLUE BALLS IN MID-AIR (6)
	for (int i = 3; i < 9; i++) {
		bool valid = false;
		int rx = 0, ry = 0;
		int attempts = 0;

		while (!valid && attempts < 300) {
			attempts++;
			rx = 60 + (rand() % 560);
			ry = 100 + (rand() % 300);

			valid = true;
			for (int j = 0; j < placedCount; j++) {
				int dx = rx - placed[j].x;
				int dy = ry - placed[j].y;
				if ((dx * dx + dy * dy) < (MIN_DIST * MIN_DIST)) {
					valid = false;
					break;
				}
			}
		}

		blueBalls4S1[i].width = 18;
		blueBalls4S1[i].height = 18;
		blueBalls4S1[i].x = rx;
		blueBalls4S1[i].y = ry;
		blueBalls4S1[i].collected = false;
		blueBalls4S1[i].platformIndex = L4S1_GROUND;

		placed[placedCount++] = { rx, ry, 18, 18, L4S1_GROUND };
	}
}

static void checkGoldBallCollision4S1() {
	checkGoldBallCollisionGeneric(goldBalls4S1, &goldBallsCollected4S1, &level4Sub1Complete);
}

static void checkBlueBallCollision4S1() {
	checkBlueBallCollisionGeneric(blueBalls4S1, &playerPoints4S1);
}

static void drawGoldBalls4S1() {
	drawGoldBallsGeneric(goldBalls4S1, goldBallTexture4S1);
}

static void drawBlueBalls4S1() {
	drawBlueBallsGeneric(blueBalls4S1, blueBallTexture4S1);
}

static void drawGoldBallHUD4S1() {
	drawGoldBallHUDGeneric(goldBallsCollected4S1, goldBallIconTextures4S1);
}

static void drawPointsHUD4S1() {
	drawPointsHUDGeneric(pointsTexture4S1, playerPoints4S1);
}

// ---------------------------------------------------------------------------
// Portal (opens once all 3 gold balls are collected)
// ---------------------------------------------------------------------------
static void drawPortal4S1() {
	if (level4Sub1Complete) {
		if (dispearTexture4S1 > 0) {
			iShowImage(680, 20, 35, 100, dispearTexture4S1);
		}
		else {
			iSetColor(255, 255, 0);
			iRectangle(645, 20, 35, 100);
		}
	}
}

static void triggerEnterLevel4Sub2() {
	enterLevel4Sub2 = true;
	showLevel4Sub1ExitTransition = false;

	if (level4Sub2TransitionTimerId4S1 >= 0) {
		iPauseTimer(level4Sub2TransitionTimerId4S1);
	}
}

static void checkPortalCollision4S1() {
	if (level4Sub1Complete && !showLevel4Sub1ExitTransition) {
		bool collideX = (player.x + player.width >= 700);
		bool collideY = (player.y <= 120 && player.y + player.height >= 20);

		if (collideX && collideY) {
			showLevel4Sub1ExitTransition = true;
			loadExitTransitionTextures4S1();

			if (level4Sub2TransitionTimerId4S1 < 0) {
				level4Sub2TransitionTimerId4S1 = iSetTimer(1000, triggerEnterLevel4Sub2);
			}
			else {
				iResumeTimer(level4Sub2TransitionTimerId4S1);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Dragons
// ---------------------------------------------------------------------------
static void stepNextFrame4S1(int i) {
	int safetyCounter = 0;
	do {
		dragons4S1[i].animFrame++;

		if (dragons4S1[i].movingRight) {
			if (dragons4S1[i].animFrame > D4S1_RIGHT_END || dragons4S1[i].animFrame < D4S1_RIGHT_START)
				dragons4S1[i].animFrame = D4S1_RIGHT_START;
		}
		else {
			if (dragons4S1[i].animFrame > D4S1_LEFT_END || dragons4S1[i].animFrame < D4S1_LEFT_START)
				dragons4S1[i].animFrame = D4S1_LEFT_START;
		}

		safetyCounter++;
		if (safetyCounter > D4S1_TOTAL_FRAMES) break;
	} while (dragonTextures4S1[dragons4S1[i].animFrame] <= 0);
}

static void initDragon4S1() {
	loadDragonTextures4S1();
	loadDragonHealthBarTextures4S1();

	// Dragon 0: ground patrol, right half of the floor
	dragons4S1[0].width = 70;
	dragons4S1[0].height = 70;
	dragons4S1[0].x = 300;
	dragons4S1[0].y = 20;
	dragons4S1[0].minX = 240;
	dragons4S1[0].maxX = 620;
	dragons4S1[0].movingRight = true;
	dragons4S1[0].animFrame = D4S1_RIGHT_START;
	dragons4S1[0].animTimer = 0;
	dragons4S1[0].health = 5;
	dragons4S1[0].punchCount = 0;
	dragons4S1[0].alive = true;
	dragons4S1[0].platformIndex = L4S1_GROUND;
	if (dragonTextures4S1[dragons4S1[0].animFrame] <= 0) stepNextFrame4S1(0);

	// Dragon 1: low-left air patrol, just above slab 1's lane
	dragons4S1[1].width = 70;
	dragons4S1[1].height = 70;
	dragons4S1[1].x = 90;
	dragons4S1[1].y = 150;
	dragons4S1[1].minX = 40;
	dragons4S1[1].maxX = 320;
	dragons4S1[1].movingRight = true;
	dragons4S1[1].animFrame = D4S1_RIGHT_START;
	dragons4S1[1].animTimer = 0;
	dragons4S1[1].health = 5;
	dragons4S1[1].punchCount = 0;
	dragons4S1[1].alive = true;
	dragons4S1[1].platformIndex = L4S1_GROUND;
	if (dragonTextures4S1[dragons4S1[1].animFrame] <= 0) stepNextFrame4S1(1);

	// Dragon 2: high-right air patrol, above slab 2's top position
	dragons4S1[2].width = 70;
	dragons4S1[2].height = 70;
	dragons4S1[2].x = 420;
	dragons4S1[2].y = 390;
	dragons4S1[2].minX = 380;
	dragons4S1[2].maxX = 660;
	dragons4S1[2].movingRight = true;
	dragons4S1[2].animFrame = D4S1_RIGHT_START;
	dragons4S1[2].animTimer = 0;
	dragons4S1[2].health = 5;
	dragons4S1[2].punchCount = 0;
	dragons4S1[2].alive = true;
	dragons4S1[2].platformIndex = L4S1_GROUND;
	if (dragonTextures4S1[dragons4S1[2].animFrame] <= 0) stepNextFrame4S1(2);
}

// =========================================================
// FIREBALL BREATH ATTACK (spawn, physics, collision, draw)
// =========================================================
static void spawnExplosion4S1(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES4S1; j++) {
			if (!explosionParticles4S1[j].active) {
				explosionParticles4S1[j].x = x;
				explosionParticles4S1[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticles4S1[j].vx = cos(angle) * speed;
				explosionParticles4S1[j].vy = sin(angle) * speed;
				explosionParticles4S1[j].radius = 3.0f + (rand() % 3);
				explosionParticles4S1[j].life = 20 + (rand() % 10);
				explosionParticles4S1[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticle4S1(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES4S1; i++) {
		if (!trailParticles4S1[i].active) {
			trailParticles4S1[i].x = x + (rand() % 6 - 3);
			trailParticles4S1[i].y = y + (rand() % 6 - 3);
			trailParticles4S1[i].radius = 4.0f + (rand() % 4);
			trailParticles4S1[i].alpha = 255;
			trailParticles4S1[i].active = true;
			break;
		}
	}
}

// Called the instant a dragon's animation reaches its breath-attack frame
// (16 or 54). Spawns one fireball arcing toward the player's current position.
static void spawnFireball4S1(int dragonIndex) {
	for (int j = 0; j < MAX_FIREBALLS4S1; j++) {
		if (!fireballs4S1[j].active) {
			fireballs4S1[j].x = dragons4S1[dragonIndex].x + dragons4S1[dragonIndex].width / 2 - 9;
			fireballs4S1[j].y = dragons4S1[dragonIndex].y + dragons4S1[dragonIndex].height / 2 - 9;
			fireballs4S1[j].width = 18; fireballs4S1[j].height = 18;

			float dx = (player.x + player.width / 2.0f) - (fireballs4S1[j].x + 9);
			float dy = (player.y + player.height / 2.0f) - (fireballs4S1[j].y + 9);
			float dist = sqrt(dx * dx + dy * dy);
			if (dist == 0) dist = 1;

			float speed = 7.0f;
			fireballs4S1[j].vx = (dx / dist) * speed;
			// Arc boost for a parabola instead of a flat, unavoidable line shot
			fireballs4S1[j].vy = (dy / dist) * speed + 4.5f;
			fireballs4S1[j].rotation = 0.0f;
			fireballs4S1[j].active = true;
			break;
		}
	}
}

static void updateFireballs4S1() {
	for (int i = 0; i < MAX_FIREBALLS4S1; i++) {
		if (!fireballs4S1[i].active) continue;

		int oldY = fireballs4S1[i].y;

		// Gravity
		fireballs4S1[i].vy -= 0.2f;

		fireballs4S1[i].x += fireballs4S1[i].vx;
		fireballs4S1[i].y += fireballs4S1[i].vy;
		fireballs4S1[i].rotation += fireballs4S1[i].vx * 6.0f;

		// Bounce off the ground
		if (fireballs4S1[i].y <= L4S1_BOTTOM) {
			fireballs4S1[i].y = L4S1_BOTTOM;
			fireballs4S1[i].vy = -fireballs4S1[i].vy * 0.5f;
			fireballs4S1[i].vx *= 0.8f;
			spawnExplosion4S1(fireballs4S1[i].x + 9, fireballs4S1[i].y + 9);
			if (abs(fireballs4S1[i].vy) < 1.5f) {
				fireballs4S1[i].active = false;
			}
		}

		// Explode against the floating slabs / ground platform
		if (fireballs4S1[i].active && fireballs4S1[i].vy < 0) {
			for (int p = 0; p < L4S1_PLATFORM_COUNT; p++) {
				Platform plat = level4sub1_platforms[p];

				bool withinX = (fireballs4S1[i].x + fireballs4S1[i].width > plat.x1) && (fireballs4S1[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballs4S1[i].y <= plat.y2) {
					fireballs4S1[i].y = plat.y2;
					spawnExplosion4S1(fireballs4S1[i].x + 9, fireballs4S1[i].y + 9);
					fireballs4S1[i].active = false;
					break;
				}
			}
		}

		if (fireballs4S1[i].x < -50 || fireballs4S1[i].x > 750) {
			fireballs4S1[i].active = false;
		}

		if (fireballs4S1[i].active)
			spawnTrailParticle4S1(fireballs4S1[i].x + 9, fireballs4S1[i].y + 9);
	}
}

static void updateTrailParticles4S1() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES4S1; i++) {
		if (!trailParticles4S1[i].active) continue;
		trailParticles4S1[i].alpha -= 15;
		trailParticles4S1[i].radius -= 0.2f;
		if (trailParticles4S1[i].alpha <= 0 || trailParticles4S1[i].radius <= 0) {
			trailParticles4S1[i].active = false;
		}
	}
}

static void updateExplosionParticles4S1() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4S1; i++) {
		if (!explosionParticles4S1[i].active) continue;
		explosionParticles4S1[i].x += explosionParticles4S1[i].vx;
		explosionParticles4S1[i].y += explosionParticles4S1[i].vy;
		explosionParticles4S1[i].vy -= 0.1f;
		explosionParticles4S1[i].life--;
		explosionParticles4S1[i].radius -= 0.15f;
		if (explosionParticles4S1[i].life <= 0 || explosionParticles4S1[i].radius <= 0) {
			explosionParticles4S1[i].active = false;
		}
	}
}

// Fireball hits: 8 hits to lose 1 zed (kept independent of the melee-touch
// counter in checkDragonPlayerCollision4S1).
static void checkFireballCollision4S1() {
	if (fireballHitCooldown4S1 > 0) {
		fireballHitCooldown4S1--;
		return;
	}

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS4S1; i++) {
		if (!fireballs4S1[i].active) continue;

		bool collideX = (px + pw > fireballs4S1[i].x) && (px < fireballs4S1[i].x + fireballs4S1[i].width);
		bool collideY = (py + ph > fireballs4S1[i].y) && (py < fireballs4S1[i].y + fireballs4S1[i].height);

		if (collideX && collideY) {
			fireballHitsToPlayer4S1++;

			// 8 fireball hits to lose 1 zed (life)
			if (fireballHitsToPlayer4S1 >= 8) {
				playerLives4S1--;
				fireballHitsToPlayer4S1 = 0;

				if (playerLives4S1 <= 0) {
					playerLives4S1 = 0;
					isGameOver4S1 = true;
				}
			}

			fireballHitCooldown4S1 = 30;
			spawnExplosion4S1(fireballs4S1[i].x + 9, fireballs4S1[i].y + 9);
			fireballs4S1[i].active = false;
			break;
		}
	}
}

static void drawFireballs4S1() {
	// 1. Trail
	for (int i = 0; i < MAX_TRAIL_PARTICLES4S1; i++) {
		if (!trailParticles4S1[i].active) continue;
		int alpha = trailParticles4S1[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticles4S1[i].x, trailParticles4S1[i].y, trailParticles4S1[i].radius, 20);
	}

	// 2. Explosions
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4S1; i++) {
		if (!explosionParticles4S1[i].active) continue;
		if (explosionParticles4S1[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticles4S1[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticles4S1[i].x, explosionParticles4S1[i].y, explosionParticles4S1[i].radius, 20);
	}

	// 3. Main fireballs
	for (int i = 0; i < MAX_FIREBALLS4S1; i++) {
		if (!fireballs4S1[i].active) continue;
		float cx = fireballs4S1[i].x + 9;
		float cy = fireballs4S1[i].y + 9;

		if (fireballTexture4S1 > 0) {
			iRotate(cx, cy, fireballs4S1[i].rotation);
			iShowImage(fireballs4S1[i].x, fireballs4S1[i].y, fireballs4S1[i].width, fireballs4S1[i].height, fireballTexture4S1);
			iUnRotate();
		}
		else {
			iSetColor(200, 50, 0); iFilledCircle(cx, cy, 14, 50);
			iSetColor(255, 120, 0); iFilledCircle(cx, cy, 10, 50);
			iSetColor(255, 255, 150); iFilledCircle(cx, cy, 5, 50);
		}
	}
}

static void updateDragon4S1() {
	for (int i = 0; i < D4S1_COUNT; i++) {
		if (!dragons4S1[i].alive) continue;

		int minX = dragons4S1[i].minX;
		int maxX = dragons4S1[i].maxX - dragons4S1[i].width;

		if (dragons4S1[i].movingRight) {
			dragons4S1[i].x += D4S1_SPEED;
			if (dragons4S1[i].x >= maxX) {
				dragons4S1[i].x = maxX;
				dragons4S1[i].movingRight = false;
				dragons4S1[i].animFrame = D4S1_LEFT_START;
				if (dragonTextures4S1[dragons4S1[i].animFrame] <= 0) stepNextFrame4S1(i);
			}
		}
		else {
			dragons4S1[i].x -= D4S1_SPEED;
			if (dragons4S1[i].x <= minX) {
				dragons4S1[i].x = minX;
				dragons4S1[i].movingRight = true;
				dragons4S1[i].animFrame = D4S1_RIGHT_START;
				if (dragonTextures4S1[dragons4S1[i].animFrame] <= 0) stepNextFrame4S1(i);
			}
		}

		dragons4S1[i].animTimer++;
		if (dragons4S1[i].animTimer >= D4S1_ANIM_SPEED) {
			dragons4S1[i].animTimer = 0;
			stepNextFrame4S1(i);

			// Breath-attack poses: the instant the animation reaches frame
			// 16 or 54, throw one fireball at the player.
			// 20% fewer fireballs overall: 1-in-5 chance to skip the throw.
			if (dragons4S1[i].animFrame == FIREBALL_ANIM_FRAME_A_S1 || dragons4S1[i].animFrame == FIREBALL_ANIM_FRAME_B_S1) {
				if (rand() % 5 != 0) spawnFireball4S1(i);
			}
		}
	}
}

// Each dragon carries one gold ball: when it dies the ball drops where the
// dragon was standing (clamped into the reachable play area).
static void spawnGoldBallFromDragon4S1(int i) {
	int bx = dragons4S1[i].x + (dragons4S1[i].width / 2) - 12;
	int by = dragons4S1[i].y;

	if (bx < 40)  bx = 40;
	if (bx > 620) bx = 620;
	if (by < 30)  by = 30;
	if (by > 400) by = 400;

	goldBalls4S1[i].x = bx;
	goldBalls4S1[i].y = by;
	goldBalls4S1[i].platformIndex = L4S1_GROUND;
}

// Called by laserUpdateProjectiles() with the laser projectile's rectangle.
// 5 laser hits = 1 dragon life. Returns true if a dragon was hit.
static bool checkPlayerPunchDragonCollision4S1(int attackX, int attackY, int attackW, int attackH) {
	for (int i = 0; i < D4S1_COUNT; i++) {
		if (!dragons4S1[i].alive) continue;

		int dx = dragons4S1[i].x;
		int dy = dragons4S1[i].y;
		int dw = dragons4S1[i].width;
		int dh = dragons4S1[i].height;

		bool collideX = (attackX + attackW > dx) && (attackX < dx + dw);
		bool collideY = (attackY + attackH > dy) && (attackY < dy + dh);

		if (collideX && collideY) {
			dragons4S1[i].punchCount++;

			if (dragons4S1[i].punchCount >= 5) {
				dragons4S1[i].punchCount = 0;
				dragons4S1[i].health--;

				if (dragons4S1[i].health <= 0) {
					dragons4S1[i].health = 0;
					dragons4S1[i].alive = false;
					spawnGoldBallFromDragon4S1(i);
				}
			}
			laserHitCenterX = dx + dw / 2;
			laserHitCenterY = dy + dh / 2;
			laserHitCenterSet = true;
			return true;
		}
	}
	return false;
}

static void updateFightingAnimation4S1() {
	laserFightStep(isFighting4S1, isLeftMouseDown4S1, fightFrame4S1, fightAnimTimer4S1, punchDamageDealt4S1);
	laserUpdateProjectiles(checkPlayerPunchDragonCollision4S1);   // moves the lasers + hit test
}

static void handleMouseClickLevel4Sub1(int button, int state) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			isLeftMouseDown4S1 = true;
			if (!isFighting4S1 && !isPaused4S1 && !isGameOver4S1 && !showLevel4Sub1ExitTransition && !isLevel4Sub1Transition) {
				isFighting4S1 = true;
				fightAnimTimer4S1 = 0;
				punchDamageDealt4S1 = false;
				fightFrame4S1 = 1;
			}
		}
		else if (state == GLUT_UP) {
			isLeftMouseDown4S1 = false;
		}
	}
}

static void drawDragon4S1() {
	for (int i = 0; i < D4S1_COUNT; i++) {
		if (!dragons4S1[i].alive) continue;

		int textureId = dragonTextures4S1[dragons4S1[i].animFrame];

		if (textureId > 0) {
			iShowImage(dragons4S1[i].x, dragons4S1[i].y, dragons4S1[i].width, dragons4S1[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragons4S1[i].x, dragons4S1[i].y, dragons4S1[i].width, dragons4S1[i].height);
		}

		// health 5 -> 1.png, health 4 -> 2.png, ...
		int spriteIdx = 6 - dragons4S1[i].health;

		if (spriteIdx >= 1 && spriteIdx <= 5 && dragonHealthBarTextures4S1[spriteIdx] > 0) {
			iShowImage(dragons4S1[i].x + 10, dragons4S1[i].y + dragons4S1[i].height + 5, 50, 8, dragonHealthBarTextures4S1[spriteIdx]);
		}
	}
}

static void checkDragonPlayerCollision4S1() {
	if (hitCooldown4S1 > 0) {
		hitCooldown4S1--;
		return;
	}

	for (int i = 0; i < D4S1_COUNT; i++) {
		if (!dragons4S1[i].alive) continue;

		int hx = dragons4S1[i].x;
		int hy = dragons4S1[i].y;
		int hw = dragons4S1[i].width;
		int hh = dragons4S1[i].height;

		if (dragons4S1[i].animFrame >= 1 && dragons4S1[i].animFrame <= 17) {
			hx = dragons4S1[i].x + (int)(dragons4S1[i].width * 0.556f);
			hy = dragons4S1[i].y + (int)(dragons4S1[i].height * 0.312f);
			hw = (int)(dragons4S1[i].width * 0.363f);
			hh = (int)(dragons4S1[i].height * 0.347f);
		}
		else if (dragons4S1[i].animFrame >= D4S1_RIGHT_START && dragons4S1[i].animFrame <= D4S1_RIGHT_END) {
			hx = dragons4S1[i].x + (int)(dragons4S1[i].width * 0.081f);
			hy = dragons4S1[i].y + (int)(dragons4S1[i].height * 0.312f);
			hw = (int)(dragons4S1[i].width * 0.363f);
			hh = (int)(dragons4S1[i].height * 0.347f);
		}

		bool collideX = (player.x + player.width > hx) && (player.x < hx + hw);
		bool collideY = (player.y + player.height > hy) && (player.y < hy + hh);

		if (collideX && collideY) {
			dragonHitsToPlayer4S1++;

			// 8 touches to lose 1 zed (life)
			if (dragonHitsToPlayer4S1 >= 8) {
				playerLives4S1--;
				dragonHitsToPlayer4S1 = 0;

				if (playerLives4S1 <= 0) {
					playerLives4S1 = 0;
					isGameOver4S1 = true;
				}
			}

			hitCooldown4S1 = 30;
			break;
		}
	}
}

static void drawHUD4S1() {
	drawHUDGeneric(zedsLabelTexture4S1, zedsIconTexture4S1, playerLives4S1, energyFrame4S1, energyTextures4S1);
}

// ---------------------------------------------------------------------------
// Level setup
// ---------------------------------------------------------------------------
static void initLevel4Sub1(bool freshStart = true) {
	for (int i = 0; i < L4S1_PLATFORM_COUNT; i++) {
		level4sub1_platforms[i] = level4sub1_platformStart[i];
	}
	slabDirection4S1[L4S1_GROUND] = 0;
	slabDirection4S1[L4S1_SLAB1] = 1;   // starts at its left limit, heads right
	slabDirection4S1[L4S1_SLAB2] = 1;   // starts at its bottom limit, heads up
	slabDirection4S1[L4S1_SLAB3] = -1;  // starts at its right limit, heads left
	slabDirection4S1[L4S1_SLAB4] = 1;   // starts mid-track, heads up

	for (int i = 0; i < L4S1_PLATFORM_COUNT; i++) slabCarry4S1[i] = 0.0f;

	initPlayer(40, L4S1_BOTTOM);
	initDragon4S1();
	loadSlabTexture4S1();
	initGoldAndBlueBalls4S1(freshStart);
	loadPauseTextures4S1();
	loadZedsTextures4S1();
	loadEnergyTextures4S1();
	loadFightingTextures4S1();
	loadFireballTexture4S1();
	loadExitTransitionTextures4S1();
	loadTransitionTextures4S1();

	for (int i = 0; i < MAX_FIREBALLS4S1; i++) fireballs4S1[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES4S1; i++) trailParticles4S1[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4S1; i++) explosionParticles4S1[i].active = false;
	fireballHitsToPlayer4S1 = 0;
	fireballHitCooldown4S1 = 0;

	level4Sub1Complete = false;
	isGameOver4S1 = false;
	isPaused4S1 = false;
	hitCooldown4S1 = 0;
	dragonHitsToPlayer4S1 = 0;

	showLevel4Sub1ExitTransition = false;
	if (level4Sub2TransitionTimerId4S1 >= 0) {
		iPauseTimer(level4Sub2TransitionTimerId4S1);
	}

	// No intro banner for this bonus stage - go straight into gameplay.
	isLevel4Sub1Transition = false;
	level4Sub1TransitionCounter = 0;

	currentPunchCombo4S1 = 0;
	isFighting4S1 = false;
	punchDamageDealt4S1 = false;
	laserResetAll();

	playerLives4S1 = 5;
	energyFrame4S1 = 1;   // energy refills (resets) at the start of every level-4 stage
	distanceMoved4S1 = 0;
}

static void drawLevel4Sub1Background() {
	iShowBMP(0, 0, IMG_LV4SUB1_BG);
}

static void resolvePlatformCollision4S1() {
	int oldY = player.y - player.velocityY;
	int oldTopY = oldY + player.height;
	int currentTopY = player.y + player.height;

	if (player.y <= L4S1_BOTTOM) {
		player.y = L4S1_BOTTOM;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	player.onGround = false;

	for (int i = 1; i < L4S1_PLATFORM_COUNT; i++) {
		Platform p = level4sub1_platforms[i];
		bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);

		if (!withinX) continue;

		if (player.velocityY <= 0) {
			int platformTop = p.y2;
			if (oldY >= platformTop && player.y <= platformTop) {
				player.y = platformTop;
				player.velocityY = 0;
				player.onGround = true;
				player.jumping = false;
				break;
			}
		}
		else {
			int platformBottom = p.y1;
			if (oldTopY <= platformBottom && currentTopY >= platformBottom) {
				player.y = platformBottom - player.height;
				player.velocityY = 0;
				break;
			}
		}
	}
}

static bool playerStandingOnPlatform4S1(int index) {
	Platform p = level4sub1_platforms[index];
	bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
	int playerBottom = player.y;
	return withinX && abs(playerBottom - p.y2) <= 3 && player.velocityY == 0;
}

// ---------------------------------------------------------------------------
// Slab movement. Same bounce-and-clamp scheme as level4.h, generalised so the
// limits come from L4S1_SLAB_MIN / L4S1_SLAB_MAX. Horizontal slabs clamp on
// x, vertical slabs on y; the resulting per-frame delta is then applied to
// whatever is riding that slab (the player, gold balls, blue balls).
// ---------------------------------------------------------------------------

// Returns this frame's integer displacement for slab i, folding in the
// fractional part (2.4 px/frame) that a plain int step would otherwise
// lose every other frame, so the average speed is exactly slabSpeed4S1[i].
static int slabStep4S1(int i) {
	float raw = slabSpeed4S1[i] * slabDirection4S1[i] + slabCarry4S1[i];
	int step = (int)raw;
	slabCarry4S1[i] = raw - step;
	return step;
}

static void moveSlabHorizontal4S1(int i, int minX, int maxX) {
	int dx = slabStep4S1(i);
	int width = level4sub1_platforms[i].x2 - level4sub1_platforms[i].x1;

	level4sub1_platforms[i].x1 += dx;
	level4sub1_platforms[i].x2 += dx;

	if (level4sub1_platforms[i].x1 <= minX) {
		dx += minX - level4sub1_platforms[i].x1;
		level4sub1_platforms[i].x1 = minX;
		level4sub1_platforms[i].x2 = minX + width;
		slabDirection4S1[i] = 1;
	}
	else if (level4sub1_platforms[i].x2 >= maxX) {
		dx -= level4sub1_platforms[i].x2 - maxX;
		level4sub1_platforms[i].x1 = maxX - width;
		level4sub1_platforms[i].x2 = maxX;
		slabDirection4S1[i] = -1;
	}

	slabDeltaX4S1[i] = dx;
}

static void moveSlabVertical4S1(int i, int minY, int maxY) {
	int dy = slabStep4S1(i);
	int height = level4sub1_platforms[i].y2 - level4sub1_platforms[i].y1;

	level4sub1_platforms[i].y1 += dy;
	level4sub1_platforms[i].y2 += dy;

	if (level4sub1_platforms[i].y1 <= minY) {
		dy += minY - level4sub1_platforms[i].y1;
		level4sub1_platforms[i].y1 = minY;
		level4sub1_platforms[i].y2 = minY + height;
		slabDirection4S1[i] = 1;
	}
	else if (level4sub1_platforms[i].y2 >= maxY) {
		dy -= level4sub1_platforms[i].y2 - maxY;
		level4sub1_platforms[i].y1 = maxY - height;
		level4sub1_platforms[i].y2 = maxY;
		slabDirection4S1[i] = -1;
	}

	slabDeltaY4S1[i] = dy;
}

static void updateMovingSlabs4S1() {
	for (int i = 0; i < L4S1_PLATFORM_COUNT; i++) {
		slabDeltaX4S1[i] = 0;
		slabDeltaY4S1[i] = 0;
	}

	bool playerOnSlab[L4S1_PLATFORM_COUNT] = { false };
	for (int i = 1; i < L4S1_PLATFORM_COUNT; i++) {
		playerOnSlab[i] = playerStandingOnPlatform4S1(i);
	}

	// Slab 1: horizontal, x = 30 .. 250
	moveSlabHorizontal4S1(L4S1_SLAB1, L4S1_SLAB_MIN[L4S1_SLAB1], L4S1_SLAB_MAX[L4S1_SLAB1]);

	// Slab 2: vertical, y = 150 .. 353
	moveSlabVertical4S1(L4S1_SLAB2, L4S1_SLAB_MIN[L4S1_SLAB2], L4S1_SLAB_MAX[L4S1_SLAB2]);

	// Slab 3: horizontal, x = 200 .. 470
	moveSlabHorizontal4S1(L4S1_SLAB3, L4S1_SLAB_MIN[L4S1_SLAB3], L4S1_SLAB_MAX[L4S1_SLAB3]);

	// Slab 4: vertical, y = 260 .. 423
	moveSlabVertical4S1(L4S1_SLAB4, L4S1_SLAB_MIN[L4S1_SLAB4], L4S1_SLAB_MAX[L4S1_SLAB4]);

	// Carry the player when standing on a moving slab
	for (int i = 1; i < L4S1_PLATFORM_COUNT; i++) {
		if (playerOnSlab[i]) {
			player.x += slabDeltaX4S1[i];
			player.y += slabDeltaY4S1[i];
		}
	}

	// Move slab-attached gold balls
	for (int i = 0; i < 3; i++) {
		if (!goldBalls4S1[i].collected) {
			int p = goldBalls4S1[i].platformIndex;
			goldBalls4S1[i].x += slabDeltaX4S1[p];
			goldBalls4S1[i].y += slabDeltaY4S1[p];
		}
	}

	// Move slab-attached blue balls
	for (int i = 0; i < 9; i++) {
		if (!blueBalls4S1[i].collected) {
			int p = blueBalls4S1[i].platformIndex;
			blueBalls4S1[i].x += slabDeltaX4S1[p];
			blueBalls4S1[i].y += slabDeltaY4S1[p];
		}
	}
}

static void drawPlatforms4S1() {
	if (slabTexture4S1 <= 0) return;

	for (int i = 1; i < L4S1_PLATFORM_COUNT; i++) {
		int width = level4sub1_platforms[i].x2 - level4sub1_platforms[i].x1;
		int height = level4sub1_platforms[i].y2 - level4sub1_platforms[i].y1;
		iShowImage(level4sub1_platforms[i].x1, level4sub1_platforms[i].y1, width, height, slabTexture4S1);
	}
}

static void updateLevel4Sub1() {
	if (isLevel4Sub1Transition) {
		level4Sub1TransitionCounter++;
		if (level4Sub1TransitionCounter >= 100) {
			isLevel4Sub1Transition = false;
		}
		return;
	}

	if (isGameOver4S1 || isPaused4S1 || showLevel4Sub1ExitTransition) return;

	updateMovingSlabs4S1();

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

	if (player.x < L4S1_LEFT) player.x = L4S1_LEFT;

	if (player.x + player.width > L4S1_RIGHT) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level4Sub1Complete && atPortalCoordinates)) {
			player.x = L4S1_RIGHT - player.width;
		}
	}

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved4S1 += moveDist;
		while (distanceMoved4S1 >= 150) {
			distanceMoved4S1 -= 150;
			if (energyFrame4S1 < 145) {
				energyFrame4S1++;
			}
		}
		if (energyFrame4S1 >= 145) {
			isGameOver4S1 = true;
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

	resolvePlatformCollision4S1();

	if (player.y + player.height > L4S1_TOP) {
		player.y = L4S1_TOP - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateFightingAnimation4S1();
	updateDragon4S1();

	// --- FIREBALL & PARTICLE UPDATES ---
	updateFireballs4S1();
	updateTrailParticles4S1();
	updateExplosionParticles4S1();
	checkFireballCollision4S1();

	checkDragonPlayerCollision4S1();
	checkGoldBallCollision4S1();
	checkBlueBallCollision4S1();
	checkPortalCollision4S1();
}

static void drawPauseMenu4S1() {
	drawPauseMenuGeneric(resumeTexture4S1, pauseResumeBtn4S1, restartTexture4S1, pauseRestartBtn4S1, exitTexture4S1, pauseExitBtn4S1);
}

static void drawLevel4Sub1() {
	drawLevel4Sub1Background();

	if (isLevel4Sub1Transition) {
		// No banner between Level 4 and this bonus stage - kept only so
		// the state machine above (dead now that the flag stays false)
		// has a matching draw branch if it's ever re-enabled.
		return;
	}

	drawPlatforms4S1();
	drawDragon4S1();
	drawFireballs4S1(); // Draws trails, explosions, and the fireballs themselves
	drawGoldBalls4S1();
	drawBlueBalls4S1();
	drawPortal4S1();

	if (!(isFighting4S1 && drawLaserFightSprite(fightingTextures4S1, fightFrame4S1, player.facingRight))) {
		drawGunPlayerBody();
	}
	drawLaserProjectiles();

	drawHUD4S1();
	drawGoldBallHUD4S1();
	drawPointsHUD4S1();

	if (isPaused4S1) {
		drawPauseMenu4S1();
	}
	else if (showLevel4Sub1ExitTransition) {
		// No banner between the two bonus trials - transitions quietly
		// into Sub 2 once the brief delay above elapses.
	}
	else if (isGameOver4S1) {
		drawTotalPointsBoxGeneric(totalPointsTexture4S1, playerPoints4S1, 405);

		if (gameOverTexture4S1 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTexture4S1);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTexture4S1 > 0)
			iShowImage(gameOverRestartBtn4S1.x1, gameOverRestartBtn4S1.y1, 220, 50, gameOverRestartTexture4S1);

		if (gameOverExitTexture4S1 > 0)
			iShowImage(gameOverExitBtn4S1.x1, gameOverExitBtn4S1.y1, 150, 35, gameOverExitTexture4S1);
	}
}

#endif