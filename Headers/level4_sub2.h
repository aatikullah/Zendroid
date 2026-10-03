#ifndef LEVEL4_SUB2_H
#define LEVEL4_SUB2_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// ============================================================================
// level4_sub2.h
//
// Second bonus stage after Level 4, played right after level4_sub1.h. Same
// background (lv4_bg.bmp) and the exact same rule set as level4.h:
//   - moving 150 x 23 slabs that carry the player and their collectibles,
//   - hold-left-mouse 4-punch combo, 2 punches per dragon life, 5 lives,
//   - one gold ball per killed dragon, dropped at a random reachable spot
//     (not at the dragon's own position) - 3 gold balls opens the portal,
//   - 9 blue balls at 10 points each,
//   - Zeds / energy HUD, pause menu, game-over screen.
//
// The slab layout is a "climb up the right side, then cross back left"
// route and shares no coordinates with level4.h or level4_sub1.h. Each slab
// owns its own exclusive band of the screen, so no two slabs can ever
// overlap at any point along their travel:
//
//   Slab 1  centre-left lift   vertical    y 100 .. 283  at x  80..230
//   Slab 2  high left walkway  horizontal  x  60 .. 380  at y 350..373
//   Slab 3  right bridge       horizontal  x 380 .. 660  at y 260..283
//   Slab 4  right lift         vertical    y 320 .. 453  at x 460..610
//   Slab 5  upper crossing     horizontal  x 310 .. 620  at y 150..173
//
// Slab 1's column and Slab 5's track used to sit only 10px apart (x 300 vs
// x 310), and Slab 2's max x and Slab 4's column only 40px apart (x 380 vs
// x 420) - close enough that the player's own width could bridge the gap.
// Since Slab 1 passes through Slab 5's height (y 173) and Slab 4 passes
// through Slab 2's height band during their travel, that let the player
// get matched onto BOTH slabs at once (see the carry loop below) and get
// launched skyward. Both columns are now moved a full 80px clear of their
// neighbour so the bands can never touch, no matter how wide the player is.
// ============================================================================

#define IMG_LV4SUB2_BG "Images/lv4_bg.bmp"

#define L4S2_PLATFORM_COUNT 6
#define L4S2_GROUND   0
#define L4S2_SLAB1    1
#define L4S2_SLAB2    2
#define L4S2_SLAB3    3
#define L4S2_SLAB4    4
#define L4S2_SLAB5    5

static Platform level4sub2_platforms[L4S2_PLATFORM_COUNT] = {
	{ 20, 20, 680, 20 },   // Fixed bottom ground (not drawn)
	{ 80, 180, 230, 203 },   // Slab 1: vertical,   travels y 100 .. 283
	{ 60, 350, 210, 373 },   // Slab 2: horizontal, travels x  60 .. 380
	{ 510, 260, 660, 283 },  // Slab 3: horizontal, travels x 380 .. 660
	{ 460, 330, 610, 353 },  // Slab 4: vertical,   travels y 320 .. 453
	{ 310, 150, 460, 173 }   // Slab 5: horizontal, travels x 310 .. 620
};

static const Platform level4sub2_platformStart[L4S2_PLATFORM_COUNT] = {
	{ 20, 20, 680, 70 },
	{ 80, 180, 230, 203 },
	{ 60, 350, 210, 373 },
	{ 510, 260, 660, 283 },
	{ 460, 330, 610, 353 },
	{ 310, 150, 460, 173 }
};

// Patrol limits, one entry per platform (index 0 = ground, unused).
static const int L4S2_SLAB_MIN[L4S2_PLATFORM_COUNT] = { 0, 100, 60, 380, 320, 310 };
static const int L4S2_SLAB_MAX[L4S2_PLATFORM_COUNT] = { 0, 283, 380, 660, 453, 620 };

static int slabDirection4S2[L4S2_PLATFORM_COUNT] = { 0, -1, 1, -1, 1, 1 };
// Slabs move ~20% faster than the original 2 px/frame (2 * 1.2 = 2.4).
// Kept as a float speed + per-slab fractional carry so the extra 0.4
// px/frame accumulates precisely instead of being rounded away every
// frame (see slabStep4S2()).
#define SLAB_SPEED_SCALE4S2 1.2f
static float slabSpeed4S2[L4S2_PLATFORM_COUNT] = { 0.0f, 2 * SLAB_SPEED_SCALE4S2, 2 * SLAB_SPEED_SCALE4S2, 2 * SLAB_SPEED_SCALE4S2, 2 * SLAB_SPEED_SCALE4S2, 2 * SLAB_SPEED_SCALE4S2 };
static float slabCarry4S2[L4S2_PLATFORM_COUNT] = { 0.0f };
static int slabDeltaX4S2[L4S2_PLATFORM_COUNT] = { 0 };
static int slabDeltaY4S2[L4S2_PLATFORM_COUNT] = { 0 };

static int slabTexture4S2 = 0;

#define L4S2_LEFT   20
#define L4S2_RIGHT  680
#define L4S2_BOTTOM 25
#define L4S2_TOP    480

// ===== Dragon Frame Boundaries =====
#define D4S2_LEFT_START    1
#define D4S2_LEFT_END      17
#define D4S2_RIGHT_START   31
#define D4S2_RIGHT_END     63
#define D4S2_TOTAL_FRAMES  65

#define D4S2_SPEED         2
#define D4S2_ANIM_SPEED    3
#define D4S2_COUNT         3

static int dragonHealthBarTextures4S2[6] = { 0 };

// ===== Fireball Breath Attack =====
// The dragons throw a fireball at the player whenever their flight
// animation reaches frame 16 (the last frame of the left-facing cycle)
// or frame 54 (mid right-facing cycle) - the "mouth open / breathing
// fire" poses in the dragon sprite sheet.
#define FIREBALL_ANIM_FRAME_A_S2 16
#define FIREBALL_ANIM_FRAME_B_S2 54

#define MAX_FIREBALLS4S2 10
#define MAX_TRAIL_PARTICLES4S2 60
#define MAX_EXPLOSION_PARTICLES4S2 40

struct Fireball4S2 {
	int x, y;
	int width, height;
	float vx, vy;
	float rotation;
	bool active;
};

struct TrailParticle4S2 {
	float x, y;
	float radius;
	int alpha;
	bool active;
};

struct ExplosionParticle4S2 {
	float x, y;
	float vx, vy;
	float radius;
	int life;
	bool active;
};

static Fireball4S2 fireballs4S2[MAX_FIREBALLS4S2];
static TrailParticle4S2 trailParticles4S2[MAX_TRAIL_PARTICLES4S2];
static ExplosionParticle4S2 explosionParticles4S2[MAX_EXPLOSION_PARTICLES4S2];
static int fireballTexture4S2 = 0;

// Fireball hits needed to drop 1 zed. Kept separate from the melee-touch
// counter below so the two damage sources never interfere with each other.
static int fireballHitsToPlayer4S2 = 0;
static int fireballHitCooldown4S2 = 0;

// ===== Fighting & Combo Variables =====
#define F4S2_FRAMES      LASER_FRAMES
#define F4S2_FRAMES_ALL  LASER_FRAMES_ALL
#define F4S2_ANIM_SPEED  7

static int fightingTextures4S2[F4S2_FRAMES_ALL + 1] = { 0 };
static bool isFighting4S2 = false;
static bool isLeftMouseDown4S2 = false;
static int currentPunchCombo4S2 = 0;
static int fightFrame4S2 = 1;
static int fightAnimTimer4S2 = 0;
static bool punchDamageDealt4S2 = false;

// ===== Game State Variables =====
static bool level4Sub2Complete = false;
static bool isGameOver4S2 = false;
static bool isPaused4S2 = false;
static bool enterLevel5FromSub2 = false;
static int hitCooldown4S2 = 0;
static int dragonHitsToPlayer4S2 = 0;

// ----- Exit transition (bonus stage 2 done -> level 5 starts) -----
// This is the true end of the Level 4 arc (main level + both bonus
// trials), so this is the ONLY place that shows "Level 4 Completed /
// Level 5 Starts".
static bool showLevel4Sub2ExitTransition = false;
static int level4CompletedTexture4S2 = 0;
static int level5StartsTexture4S2 = 0;
static int level5TransitionTimerId4S2 = -1;

// ----- Intro transition -----
// Disabled: per design, no banner is shown between the two bonus trials -
// gameplay starts immediately when Sub 2 loads.
static bool isLevel4Sub2Transition = false;
static int level4Sub2TransitionCounter = 0;

// ===== Player Lives, Energy & Points Variables =====
static int playerLives4S2 = 5;
static int& energyFrame4S2 = gEnergyFrame;
static int distanceMoved4S2 = 0;
static int energyTextures4S2[150] = { 0 };
static int& playerPoints4S2 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTexture4S2 = 0;
static int zedsIconTexture4S2 = 0;

// ===== Menu Textures & Bounding Boxes =====
static int resumeTexture4S2 = 0;
static int restartTexture4S2 = 0;
static int exitTexture4S2 = 0;

static Button pauseResumeBtn4S2 = { 260, 280, 440, 325 };
static Button pauseRestartBtn4S2 = { 260, 220, 440, 265 };
static Button pauseExitBtn4S2 = { 260, 160, 440, 205 };

// ===== Game Over Menu Textures & Bounding Boxes =====
static int totalPointsTexture4S2 = 0;
static int gameOverTexture4S2 = 0;
static int gameOverRestartTexture4S2 = 0;
static int gameOverExitTexture4S2 = 0;

static Button gameOverRestartBtn4S2 = { 240, 115, 460, 165 };
static Button gameOverExitBtn4S2 = { 275, 70, 425, 105 };

// ===== Gold Ball Collectibles, Portal & HUD =====
struct GoldBall4S2 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static GoldBall4S2 goldBalls4S2[3];
static int goldBallTexture4S2 = 0;
static int goldBallIconTextures4S2[5] = { 0 };
static int goldBallsCollected4S2 = 0;
static int dispearTexture4S2 = 0;

// ===== Blue Ball Collectibles & Points HUD =====
struct BlueBall4S2 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static BlueBall4S2 blueBalls4S2[9];
static int blueBallTexture4S2 = 0;
static int pointsTexture4S2 = 0;

struct PatrolDragon4S2 {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
	int health;
	int punchCount;  // 5 laser hits needed per life loss
	bool alive;
	int platformIndex;
	int minX, maxX;
};

static PatrolDragon4S2 dragons4S2[D4S2_COUNT];

static int dragonTextures4S2[D4S2_TOTAL_FRAMES];
static bool texturesLoaded4S2 = false;

// ---------------------------------------------------------------------------
// Texture loading
// ---------------------------------------------------------------------------
static void loadDragonHealthBarTextures4S2() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 5; i++) {
		sprintf(path, "Images/Dragonhealthbar/%d.png", i);
		dragonHealthBarTextures4S2[i] = iLoadImage(path);

		if (dragonHealthBarTextures4S2[i] <= 0) {
			sprintf(path, "../Images/Dragonhealthbar/%d.png", i);
			dragonHealthBarTextures4S2[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadDragonTextures4S2() {
	if (texturesLoaded4S2) return;

	char path[128];
	for (int i = 1; i <= D4S2_RIGHT_END; i++) {
		if (i >= 18 && i <= 30) {
			dragonTextures4S2[i] = 0;
			continue;
		}

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTextures4S2[i] = iLoadImage(path);

		if (dragonTextures4S2[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTextures4S2[i] = iLoadImage(path);
		}
	}

	texturesLoaded4S2 = true;
}

static void loadFireballTexture4S2() {
	static bool loaded = false;
	if (loaded) return;
	fireballTexture4S2 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadFightingTextures4S2() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= F4S2_FRAMES_ALL; i++) {
		sprintf(path, "Images/PlayerLaser/shoot/%d.png", i);
		fightingTextures4S2[i] = iLoadImage(path);

		if (fightingTextures4S2[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/shoot/%d.png", i);
			fightingTextures4S2[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

// Laser attack frames: 1-10 face right, 11-20 are their mirrors (facing left).
static int getFightTextureIndex4S2(int frame, bool facingRight) {
	return laserTextureIndex(frame, facingRight);
}

static void loadExitTransitionTextures4S2() {
	static bool loaded = false;
	if (loaded) return;

	level4CompletedTexture4S2 = loadTex("Images/level4completed.png", "../Images/level4completed.png");
	level5StartsTexture4S2 = loadTex("Images/level5starts.png", "../Images/level5starts.png");

	loaded = true;
}

static void loadZedsTextures4S2() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTexture4S2 = loadTex("Images/zeds.png", "../Images/zeds.png");

	zedsIconTexture4S2 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture4S2 <= 0) zedsIconTexture4S2 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture4S2 <= 0) zedsIconTexture4S2 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTextures4S2() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures4S2[i] = iLoadImage(path);

		if (energyTextures4S2[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures4S2[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadSlabTexture4S2() {
	static bool loaded = false;
	if (loaded) return;

	slabTexture4S2 = loadTex("Images/slabs.png", "../Images/slabs.png");
	loaded = true;
}

static void loadPauseTextures4S2() {
	static bool loaded = false;
	if (loaded) return;

	resumeTexture4S2 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture4S2 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture4S2 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTexture4S2 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTexture4S2 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture4S2 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture4S2 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadGoldBallTextures4S2() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTexture4S2 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture4S2 = loadTex("Images/dispear4.png", "../Images/dispear4.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures4S2[i] = iLoadImage(path);
		if (goldBallIconTextures4S2[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures4S2[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadBlueBallAndPointsTextures4S2() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTexture4S2 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture4S2 = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

// ---------------------------------------------------------------------------
// Collectibles
// ---------------------------------------------------------------------------
static void initGoldAndBlueBalls4S2(bool freshStart) {
	loadGoldBallTextures4S2();
	loadBlueBallAndPointsTextures4S2();
	goldBallsCollected4S2 = 0;
	if (freshStart) playerPoints4S2 = 0;
	enterLevel5FromSub2 = false;

	struct ItemRect {
		int x, y, w, h;
		int platformIndex;
	};
	ItemRect placed[12];
	int placedCount = 0;

	const int MIN_DIST = 45;

	// 1. GOLD BALLS (off-screen until their dragon is killed)
	for (int i = 0; i < 3; i++) {
		goldBalls4S2[i].width = 25;
		goldBalls4S2[i].height = 25;
		goldBalls4S2[i].x = -1000;
		goldBalls4S2[i].y = -1000;
		goldBalls4S2[i].collected = false;
		goldBalls4S2[i].platformIndex = L4S2_GROUND;
	}

	// 2. BLUE BALLS ON SLABS (3)
	for (int i = 0; i < 3; i++) {
		bool valid = false;
		int rx = 0, ry = 0;
		int attempts = 0;
		int pIdx = L4S2_SLAB1;

		while (!valid && attempts < 300) {
			attempts++;
			pIdx = L4S2_SLAB1 + (rand() % 5);
			Platform p = level4sub2_platforms[pIdx];
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

		blueBalls4S2[i].width = 18;
		blueBalls4S2[i].height = 18;
		blueBalls4S2[i].x = rx;
		blueBalls4S2[i].y = ry;
		blueBalls4S2[i].collected = false;
		blueBalls4S2[i].platformIndex = pIdx;

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

		blueBalls4S2[i].width = 18;
		blueBalls4S2[i].height = 18;
		blueBalls4S2[i].x = rx;
		blueBalls4S2[i].y = ry;
		blueBalls4S2[i].collected = false;
		blueBalls4S2[i].platformIndex = L4S2_GROUND;

		placed[placedCount++] = { rx, ry, 18, 18, L4S2_GROUND };
	}
}

static void checkGoldBallCollision4S2() {
	checkGoldBallCollisionGeneric(goldBalls4S2, &goldBallsCollected4S2, &level4Sub2Complete);
}

static void checkBlueBallCollision4S2() {
	checkBlueBallCollisionGeneric(blueBalls4S2, &playerPoints4S2);
}

static void drawGoldBalls4S2() {
	drawGoldBallsGeneric(goldBalls4S2, goldBallTexture4S2);
}

static void drawBlueBalls4S2() {
	drawBlueBallsGeneric(blueBalls4S2, blueBallTexture4S2);
}

static void drawGoldBallHUD4S2() {
	drawGoldBallHUDGeneric(goldBallsCollected4S2, goldBallIconTextures4S2);
}

static void drawPointsHUD4S2() {
	drawPointsHUDGeneric(pointsTexture4S2, playerPoints4S2);
}

// ---------------------------------------------------------------------------
// Portal
// ---------------------------------------------------------------------------
static void drawPortal4S2() {
	if (level4Sub2Complete) {
		if (dispearTexture4S2 > 0) {
			iShowImage(680, 20, 35, 100, dispearTexture4S2);
		}
		else {
			iSetColor(255, 255, 0);
			iRectangle(645, 20, 35, 100);
		}
	}
}

static void triggerEnterLevel5FromSub2() {
	enterLevel5FromSub2 = true;
	showLevel4Sub2ExitTransition = false;

	if (level5TransitionTimerId4S2 >= 0) {
		iPauseTimer(level5TransitionTimerId4S2);
	}
}

static void checkPortalCollision4S2() {
	if (level4Sub2Complete && !showLevel4Sub2ExitTransition) {
		bool collideX = (player.x + player.width >= 700);
		bool collideY = (player.y <= 120 && player.y + player.height >= 20);

		if (collideX && collideY) {
			showLevel4Sub2ExitTransition = true;
			loadExitTransitionTextures4S2();

			if (level5TransitionTimerId4S2 < 0) {
				level5TransitionTimerId4S2 = iSetTimer(1000, triggerEnterLevel5FromSub2);
			}
			else {
				iResumeTimer(level5TransitionTimerId4S2);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Dragons
// ---------------------------------------------------------------------------
static void stepNextFrame4S2(int i) {
	int safetyCounter = 0;
	do {
		dragons4S2[i].animFrame++;

		if (dragons4S2[i].movingRight) {
			if (dragons4S2[i].animFrame > D4S2_RIGHT_END || dragons4S2[i].animFrame < D4S2_RIGHT_START)
				dragons4S2[i].animFrame = D4S2_RIGHT_START;
		}
		else {
			if (dragons4S2[i].animFrame > D4S2_LEFT_END || dragons4S2[i].animFrame < D4S2_LEFT_START)
				dragons4S2[i].animFrame = D4S2_LEFT_START;
		}

		safetyCounter++;
		if (safetyCounter > D4S2_TOTAL_FRAMES) break;
	} while (dragonTextures4S2[dragons4S2[i].animFrame] <= 0);
}

static void initDragon4S2() {
	loadDragonTextures4S2();
	loadDragonHealthBarTextures4S2();

	// Dragon 0: ground patrol, left half of the floor
	dragons4S2[0].width = 70;
	dragons4S2[0].height = 70;
	dragons4S2[0].x = 120;
	dragons4S2[0].y = 20;
	dragons4S2[0].minX = 60;
	dragons4S2[0].maxX = 420;
	dragons4S2[0].movingRight = true;
	dragons4S2[0].animFrame = D4S2_RIGHT_START;
	dragons4S2[0].animTimer = 0;
	dragons4S2[0].health = 5;
	dragons4S2[0].punchCount = 0;
	dragons4S2[0].alive = true;
	dragons4S2[0].platformIndex = L4S2_GROUND;
	if (dragonTextures4S2[dragons4S2[0].animFrame] <= 0) stepNextFrame4S2(0);

	// Dragon 1: right-side air patrol, guards the bridge slab
	dragons4S2[1].width = 70;
	dragons4S2[1].height = 70;
	dragons4S2[1].x = 400;
	dragons4S2[1].y = 190;
	dragons4S2[1].minX = 360;
	dragons4S2[1].maxX = 660;
	dragons4S2[1].movingRight = true;
	dragons4S2[1].animFrame = D4S2_RIGHT_START;
	dragons4S2[1].animTimer = 0;
	dragons4S2[1].health = 5;
	dragons4S2[1].punchCount = 0;
	dragons4S2[1].alive = true;
	dragons4S2[1].platformIndex = L4S2_GROUND;
	if (dragonTextures4S2[dragons4S2[1].animFrame] <= 0) stepNextFrame4S2(1);

	// Dragon 2: high-left air patrol, guards the top walkway
	dragons4S2[2].width = 70;
	dragons4S2[2].height = 70;
	dragons4S2[2].x = 120;
	dragons4S2[2].y = 380;
	dragons4S2[2].minX = 80;
	dragons4S2[2].maxX = 400;
	dragons4S2[2].movingRight = true;
	dragons4S2[2].animFrame = D4S2_RIGHT_START;
	dragons4S2[2].animTimer = 0;
	dragons4S2[2].health = 5;
	dragons4S2[2].punchCount = 0;
	dragons4S2[2].alive = true;
	dragons4S2[2].platformIndex = L4S2_GROUND;
	if (dragonTextures4S2[dragons4S2[2].animFrame] <= 0) stepNextFrame4S2(2);
}

// =========================================================
// FIREBALL BREATH ATTACK (spawn, physics, collision, draw)
// =========================================================
static void spawnExplosion4S2(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES4S2; j++) {
			if (!explosionParticles4S2[j].active) {
				explosionParticles4S2[j].x = x;
				explosionParticles4S2[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticles4S2[j].vx = cos(angle) * speed;
				explosionParticles4S2[j].vy = sin(angle) * speed;
				explosionParticles4S2[j].radius = 3.0f + (rand() % 3);
				explosionParticles4S2[j].life = 20 + (rand() % 10);
				explosionParticles4S2[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticle4S2(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES4S2; i++) {
		if (!trailParticles4S2[i].active) {
			trailParticles4S2[i].x = x + (rand() % 6 - 3);
			trailParticles4S2[i].y = y + (rand() % 6 - 3);
			trailParticles4S2[i].radius = 4.0f + (rand() % 4);
			trailParticles4S2[i].alpha = 255;
			trailParticles4S2[i].active = true;
			break;
		}
	}
}

// Called the instant a dragon's animation reaches its breath-attack frame
// (16 or 54). Spawns one fireball arcing toward the player's current position.
static void spawnFireball4S2(int dragonIndex) {
	for (int j = 0; j < MAX_FIREBALLS4S2; j++) {
		if (!fireballs4S2[j].active) {
			fireballs4S2[j].x = dragons4S2[dragonIndex].x + dragons4S2[dragonIndex].width / 2 - 9;
			fireballs4S2[j].y = dragons4S2[dragonIndex].y + dragons4S2[dragonIndex].height / 2 - 9;
			fireballs4S2[j].width = 18; fireballs4S2[j].height = 18;

			float dx = (player.x + player.width / 2.0f) - (fireballs4S2[j].x + 9);
			float dy = (player.y + player.height / 2.0f) - (fireballs4S2[j].y + 9);
			float dist = sqrt(dx * dx + dy * dy);
			if (dist == 0) dist = 1;

			float speed = 7.0f;
			fireballs4S2[j].vx = (dx / dist) * speed;
			// Arc boost for a parabola instead of a flat, unavoidable line shot
			fireballs4S2[j].vy = (dy / dist) * speed + 4.5f;
			fireballs4S2[j].rotation = 0.0f;
			fireballs4S2[j].active = true;
			break;
		}
	}
}

static void updateFireballs4S2() {
	for (int i = 0; i < MAX_FIREBALLS4S2; i++) {
		if (!fireballs4S2[i].active) continue;

		int oldY = fireballs4S2[i].y;

		// Gravity
		fireballs4S2[i].vy -= 0.2f;

		fireballs4S2[i].x += fireballs4S2[i].vx;
		fireballs4S2[i].y += fireballs4S2[i].vy;
		fireballs4S2[i].rotation += fireballs4S2[i].vx * 6.0f;

		// Bounce off the ground
		if (fireballs4S2[i].y <= L4S2_BOTTOM) {
			fireballs4S2[i].y = L4S2_BOTTOM;
			fireballs4S2[i].vy = -fireballs4S2[i].vy * 0.5f;
			fireballs4S2[i].vx *= 0.8f;
			spawnExplosion4S2(fireballs4S2[i].x + 9, fireballs4S2[i].y + 9);
			if (abs(fireballs4S2[i].vy) < 1.5f) {
				fireballs4S2[i].active = false;
			}
		}

		// Explode against the floating slabs / ground platform
		if (fireballs4S2[i].active && fireballs4S2[i].vy < 0) {
			for (int p = 0; p < L4S2_PLATFORM_COUNT; p++) {
				Platform plat = level4sub2_platforms[p];

				bool withinX = (fireballs4S2[i].x + fireballs4S2[i].width > plat.x1) && (fireballs4S2[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballs4S2[i].y <= plat.y2) {
					fireballs4S2[i].y = plat.y2;
					spawnExplosion4S2(fireballs4S2[i].x + 9, fireballs4S2[i].y + 9);
					fireballs4S2[i].active = false;
					break;
				}
			}
		}

		if (fireballs4S2[i].x < -50 || fireballs4S2[i].x > 750) {
			fireballs4S2[i].active = false;
		}

		if (fireballs4S2[i].active)
			spawnTrailParticle4S2(fireballs4S2[i].x + 9, fireballs4S2[i].y + 9);
	}
}

static void updateTrailParticles4S2() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES4S2; i++) {
		if (!trailParticles4S2[i].active) continue;
		trailParticles4S2[i].alpha -= 15;
		trailParticles4S2[i].radius -= 0.2f;
		if (trailParticles4S2[i].alpha <= 0 || trailParticles4S2[i].radius <= 0) {
			trailParticles4S2[i].active = false;
		}
	}
}

static void updateExplosionParticles4S2() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4S2; i++) {
		if (!explosionParticles4S2[i].active) continue;
		explosionParticles4S2[i].x += explosionParticles4S2[i].vx;
		explosionParticles4S2[i].y += explosionParticles4S2[i].vy;
		explosionParticles4S2[i].vy -= 0.1f;
		explosionParticles4S2[i].life--;
		explosionParticles4S2[i].radius -= 0.15f;
		if (explosionParticles4S2[i].life <= 0 || explosionParticles4S2[i].radius <= 0) {
			explosionParticles4S2[i].active = false;
		}
	}
}

// Fireball hits: 8 hits to lose 1 zed (kept independent of the melee-touch
// counter in checkDragonPlayerCollision4S2).
static void checkFireballCollision4S2() {
	if (fireballHitCooldown4S2 > 0) {
		fireballHitCooldown4S2--;
		return;
	}

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS4S2; i++) {
		if (!fireballs4S2[i].active) continue;

		bool collideX = (px + pw > fireballs4S2[i].x) && (px < fireballs4S2[i].x + fireballs4S2[i].width);
		bool collideY = (py + ph > fireballs4S2[i].y) && (py < fireballs4S2[i].y + fireballs4S2[i].height);

		if (collideX && collideY) {
			fireballHitsToPlayer4S2++;

			// 8 fireball hits to lose 1 zed (life)
			if (fireballHitsToPlayer4S2 >= 8) {
				playerLives4S2--;
				fireballHitsToPlayer4S2 = 0;

				if (playerLives4S2 <= 0) {
					playerLives4S2 = 0;
					isGameOver4S2 = true;
				}
			}

			fireballHitCooldown4S2 = 30;
			spawnExplosion4S2(fireballs4S2[i].x + 9, fireballs4S2[i].y + 9);
			fireballs4S2[i].active = false;
			break;
		}
	}
}

static void drawFireballs4S2() {
	// 1. Trail
	for (int i = 0; i < MAX_TRAIL_PARTICLES4S2; i++) {
		if (!trailParticles4S2[i].active) continue;
		int alpha = trailParticles4S2[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticles4S2[i].x, trailParticles4S2[i].y, trailParticles4S2[i].radius, 20);
	}

	// 2. Explosions
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4S2; i++) {
		if (!explosionParticles4S2[i].active) continue;
		if (explosionParticles4S2[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticles4S2[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticles4S2[i].x, explosionParticles4S2[i].y, explosionParticles4S2[i].radius, 20);
	}

	// 3. Main fireballs
	for (int i = 0; i < MAX_FIREBALLS4S2; i++) {
		if (!fireballs4S2[i].active) continue;
		float cx = fireballs4S2[i].x + 9;
		float cy = fireballs4S2[i].y + 9;

		if (fireballTexture4S2 > 0) {
			iRotate(cx, cy, fireballs4S2[i].rotation);
			iShowImage(fireballs4S2[i].x, fireballs4S2[i].y, fireballs4S2[i].width, fireballs4S2[i].height, fireballTexture4S2);
			iUnRotate();
		}
		else {
			iSetColor(200, 50, 0); iFilledCircle(cx, cy, 14, 50);
			iSetColor(255, 120, 0); iFilledCircle(cx, cy, 10, 50);
			iSetColor(255, 255, 150); iFilledCircle(cx, cy, 5, 50);
		}
	}
}

static void updateDragon4S2() {
	for (int i = 0; i < D4S2_COUNT; i++) {
		if (!dragons4S2[i].alive) continue;

		int minX = dragons4S2[i].minX;
		int maxX = dragons4S2[i].maxX - dragons4S2[i].width;

		if (dragons4S2[i].movingRight) {
			dragons4S2[i].x += D4S2_SPEED;
			if (dragons4S2[i].x >= maxX) {
				dragons4S2[i].x = maxX;
				dragons4S2[i].movingRight = false;
				dragons4S2[i].animFrame = D4S2_LEFT_START;
				if (dragonTextures4S2[dragons4S2[i].animFrame] <= 0) stepNextFrame4S2(i);
			}
		}
		else {
			dragons4S2[i].x -= D4S2_SPEED;
			if (dragons4S2[i].x <= minX) {
				dragons4S2[i].x = minX;
				dragons4S2[i].movingRight = true;
				dragons4S2[i].animFrame = D4S2_RIGHT_START;
				if (dragonTextures4S2[dragons4S2[i].animFrame] <= 0) stepNextFrame4S2(i);
			}
		}

		dragons4S2[i].animTimer++;
		if (dragons4S2[i].animTimer >= D4S2_ANIM_SPEED) {
			dragons4S2[i].animTimer = 0;
			stepNextFrame4S2(i);

			// Breath-attack poses: the instant the animation reaches frame
			// 16 or 54, throw one fireball at the player.
			// 20% fewer fireballs overall: 1-in-5 chance to skip the throw.
			if (dragons4S2[i].animFrame == FIREBALL_ANIM_FRAME_A_S2 || dragons4S2[i].animFrame == FIREBALL_ANIM_FRAME_B_S2) {
				if (rand() % 5 != 0) spawnFireball4S2(i);
			}
		}
	}
}

// One gold ball per dragon. Dropped at a random reachable spot on screen
// (not at the dragon's own position) - the player has to go collect it.
static void spawnGoldBallFromDragon4S2(int i) {
	int bx = 40 + (rand() % 580);  // 40 .. 620
	int by = 30 + (rand() % 370);  // 30 .. 400

	goldBalls4S2[i].x = bx;
	goldBalls4S2[i].y = by;
	goldBalls4S2[i].platformIndex = L4S2_GROUND;
}

// Called by laserUpdateProjectiles() with the laser projectile's rectangle.
// 5 laser hits = 1 dragon life. Returns true if a dragon was hit.
static bool checkPlayerPunchDragonCollision4S2(int attackX, int attackY, int attackW, int attackH) {
	for (int i = 0; i < D4S2_COUNT; i++) {
		if (!dragons4S2[i].alive) continue;

		int dx = dragons4S2[i].x;
		int dy = dragons4S2[i].y;
		int dw = dragons4S2[i].width;
		int dh = dragons4S2[i].height;

		bool collideX = (attackX + attackW > dx) && (attackX < dx + dw);
		bool collideY = (attackY + attackH > dy) && (attackY < dy + dh);

		if (collideX && collideY) {
			dragons4S2[i].punchCount++;

			if (dragons4S2[i].punchCount >= 5) {
				dragons4S2[i].punchCount = 0;
				dragons4S2[i].health--;

				if (dragons4S2[i].health <= 0) {
					dragons4S2[i].health = 0;
					dragons4S2[i].alive = false;
					spawnGoldBallFromDragon4S2(i);
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

static void updateFightingAnimation4S2() {
	laserFightStep(isFighting4S2, isLeftMouseDown4S2, fightFrame4S2, fightAnimTimer4S2, punchDamageDealt4S2);
	laserUpdateProjectiles(checkPlayerPunchDragonCollision4S2);   // moves the lasers + hit test
}

static void handleMouseClickLevel4Sub2(int button, int state) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			isLeftMouseDown4S2 = true;
			if (!isFighting4S2 && !isPaused4S2 && !isGameOver4S2 && !showLevel4Sub2ExitTransition && !isLevel4Sub2Transition) {
				isFighting4S2 = true;
				fightAnimTimer4S2 = 0;
				punchDamageDealt4S2 = false;
				fightFrame4S2 = 1;
			}
		}
		else if (state == GLUT_UP) {
			isLeftMouseDown4S2 = false;
		}
	}
}

static void drawDragon4S2() {
	for (int i = 0; i < D4S2_COUNT; i++) {
		if (!dragons4S2[i].alive) continue;

		int textureId = dragonTextures4S2[dragons4S2[i].animFrame];

		if (textureId > 0) {
			iShowImage(dragons4S2[i].x, dragons4S2[i].y, dragons4S2[i].width, dragons4S2[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragons4S2[i].x, dragons4S2[i].y, dragons4S2[i].width, dragons4S2[i].height);
		}

		int spriteIdx = 6 - dragons4S2[i].health;

		if (spriteIdx >= 1 && spriteIdx <= 5 && dragonHealthBarTextures4S2[spriteIdx] > 0) {
			iShowImage(dragons4S2[i].x + 10, dragons4S2[i].y + dragons4S2[i].height + 5, 50, 8, dragonHealthBarTextures4S2[spriteIdx]);
		}
	}
}

static void checkDragonPlayerCollision4S2() {
	if (hitCooldown4S2 > 0) {
		hitCooldown4S2--;
		return;
	}

	for (int i = 0; i < D4S2_COUNT; i++) {
		if (!dragons4S2[i].alive) continue;

		int hx = dragons4S2[i].x;
		int hy = dragons4S2[i].y;
		int hw = dragons4S2[i].width;
		int hh = dragons4S2[i].height;

		if (dragons4S2[i].animFrame >= 1 && dragons4S2[i].animFrame <= 17) {
			hx = dragons4S2[i].x + (int)(dragons4S2[i].width * 0.556f);
			hy = dragons4S2[i].y + (int)(dragons4S2[i].height * 0.312f);
			hw = (int)(dragons4S2[i].width * 0.363f);
			hh = (int)(dragons4S2[i].height * 0.347f);
		}
		else if (dragons4S2[i].animFrame >= D4S2_RIGHT_START && dragons4S2[i].animFrame <= D4S2_RIGHT_END) {
			hx = dragons4S2[i].x + (int)(dragons4S2[i].width * 0.081f);
			hy = dragons4S2[i].y + (int)(dragons4S2[i].height * 0.312f);
			hw = (int)(dragons4S2[i].width * 0.363f);
			hh = (int)(dragons4S2[i].height * 0.347f);
		}

		bool collideX = (player.x + player.width > hx) && (player.x < hx + hw);
		bool collideY = (player.y + player.height > hy) && (player.y < hy + hh);

		if (collideX && collideY) {
			dragonHitsToPlayer4S2++;

			// 8 touches to lose 1 zed (life)
			if (dragonHitsToPlayer4S2 >= 8) {
				playerLives4S2--;
				dragonHitsToPlayer4S2 = 0;

				if (playerLives4S2 <= 0) {
					playerLives4S2 = 0;
					isGameOver4S2 = true;
				}
			}

			hitCooldown4S2 = 30;
			break;
		}
	}
}

static void drawHUD4S2() {
	drawHUDGeneric(zedsLabelTexture4S2, zedsIconTexture4S2, playerLives4S2, energyFrame4S2, energyTextures4S2);
}

// ---------------------------------------------------------------------------
// Level setup
// ---------------------------------------------------------------------------
static void initLevel4Sub2(bool freshStart = true) {
	for (int i = 0; i < L4S2_PLATFORM_COUNT; i++) {
		level4sub2_platforms[i] = level4sub2_platformStart[i];
		slabCarry4S2[i] = 0.0f;
	}
	slabDirection4S2[L4S2_GROUND] = 0;
	slabDirection4S2[L4S2_SLAB1] = -1;  // starts mid-track, heads down
	slabDirection4S2[L4S2_SLAB2] = 1;   // starts at its left limit, heads right
	slabDirection4S2[L4S2_SLAB3] = -1;  // starts at its right limit, heads left
	slabDirection4S2[L4S2_SLAB4] = 1;   // starts mid-track, heads up
	slabDirection4S2[L4S2_SLAB5] = 1;   // starts at its left limit, heads right

	initPlayer(40, L4S2_BOTTOM);
	initDragon4S2();
	loadSlabTexture4S2();
	initGoldAndBlueBalls4S2(freshStart);
	loadPauseTextures4S2();
	loadZedsTextures4S2();
	loadEnergyTextures4S2();
	loadFightingTextures4S2();
	loadFireballTexture4S2();
	loadExitTransitionTextures4S2();

	for (int i = 0; i < MAX_FIREBALLS4S2; i++) fireballs4S2[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES4S2; i++) trailParticles4S2[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4S2; i++) explosionParticles4S2[i].active = false;
	fireballHitsToPlayer4S2 = 0;
	fireballHitCooldown4S2 = 0;

	level4Sub2Complete = false;
	isGameOver4S2 = false;
	isPaused4S2 = false;
	hitCooldown4S2 = 0;
	dragonHitsToPlayer4S2 = 0;

	showLevel4Sub2ExitTransition = false;
	if (level5TransitionTimerId4S2 >= 0) {
		iPauseTimer(level5TransitionTimerId4S2);
	}

	// No intro banner for this bonus stage - go straight into gameplay.
	isLevel4Sub2Transition = false;
	level4Sub2TransitionCounter = 0;

	currentPunchCombo4S2 = 0;
	isFighting4S2 = false;
	punchDamageDealt4S2 = false;
	laserResetAll();

	playerLives4S2 = 5;
	energyFrame4S2 = 1;   // energy refills (resets) at the start of every level-4 stage
	distanceMoved4S2 = 0;
}

static void drawLevel4Sub2Background() {
	iShowBMP(0, 0, IMG_LV4SUB2_BG);
}

static void resolvePlatformCollision4S2() {
	int oldY = player.y - player.velocityY;
	int oldTopY = oldY + player.height;
	int currentTopY = player.y + player.height;

	if (player.y <= L4S2_BOTTOM) {
		player.y = L4S2_BOTTOM;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	player.onGround = false;

	for (int i = 1; i < L4S2_PLATFORM_COUNT; i++) {
		Platform p = level4sub2_platforms[i];
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

static bool playerStandingOnPlatform4S2(int index) {
	Platform p = level4sub2_platforms[index];
	bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
	int playerBottom = player.y;
	return withinX && abs(playerBottom - p.y2) <= 3 && player.velocityY == 0;
}

// ---------------------------------------------------------------------------
// Slab movement (same bounce-and-clamp scheme as level4.h)
// ---------------------------------------------------------------------------

// Returns this frame's integer displacement for slab i, folding in the
// fractional part (2.4 px/frame) that a plain int step would otherwise
// lose every other frame, so the average speed is exactly slabSpeed4S2[i].
static int slabStep4S2(int i) {
	float raw = slabSpeed4S2[i] * slabDirection4S2[i] + slabCarry4S2[i];
	int step = (int)raw;
	slabCarry4S2[i] = raw - step;
	return step;
}

static void moveSlabHorizontal4S2(int i, int minX, int maxX) {
	int dx = slabStep4S2(i);
	int width = level4sub2_platforms[i].x2 - level4sub2_platforms[i].x1;

	level4sub2_platforms[i].x1 += dx;
	level4sub2_platforms[i].x2 += dx;

	if (level4sub2_platforms[i].x1 <= minX) {
		dx += minX - level4sub2_platforms[i].x1;
		level4sub2_platforms[i].x1 = minX;
		level4sub2_platforms[i].x2 = minX + width;
		slabDirection4S2[i] = 1;
	}
	else if (level4sub2_platforms[i].x2 >= maxX) {
		dx -= level4sub2_platforms[i].x2 - maxX;
		level4sub2_platforms[i].x1 = maxX - width;
		level4sub2_platforms[i].x2 = maxX;
		slabDirection4S2[i] = -1;
	}

	slabDeltaX4S2[i] = dx;
}

static void moveSlabVertical4S2(int i, int minY, int maxY) {
	int dy = slabStep4S2(i);
	int height = level4sub2_platforms[i].y2 - level4sub2_platforms[i].y1;

	level4sub2_platforms[i].y1 += dy;
	level4sub2_platforms[i].y2 += dy;

	if (level4sub2_platforms[i].y1 <= minY) {
		dy += minY - level4sub2_platforms[i].y1;
		level4sub2_platforms[i].y1 = minY;
		level4sub2_platforms[i].y2 = minY + height;
		slabDirection4S2[i] = 1;
	}
	else if (level4sub2_platforms[i].y2 >= maxY) {
		dy -= level4sub2_platforms[i].y2 - maxY;
		level4sub2_platforms[i].y1 = maxY - height;
		level4sub2_platforms[i].y2 = maxY;
		slabDirection4S2[i] = -1;
	}

	slabDeltaY4S2[i] = dy;
}

static void updateMovingSlabs4S2() {
	for (int i = 0; i < L4S2_PLATFORM_COUNT; i++) {
		slabDeltaX4S2[i] = 0;
		slabDeltaY4S2[i] = 0;
	}

	bool playerOnSlab[L4S2_PLATFORM_COUNT] = { false };
	for (int i = 1; i < L4S2_PLATFORM_COUNT; i++) {
		playerOnSlab[i] = playerStandingOnPlatform4S2(i);
	}

	// Slab 1: vertical, y = 100 .. 283
	moveSlabVertical4S2(L4S2_SLAB1, L4S2_SLAB_MIN[L4S2_SLAB1], L4S2_SLAB_MAX[L4S2_SLAB1]);

	// Slab 2: horizontal, x = 60 .. 380
	moveSlabHorizontal4S2(L4S2_SLAB2, L4S2_SLAB_MIN[L4S2_SLAB2], L4S2_SLAB_MAX[L4S2_SLAB2]);

	// Slab 3: horizontal, x = 380 .. 660
	moveSlabHorizontal4S2(L4S2_SLAB3, L4S2_SLAB_MIN[L4S2_SLAB3], L4S2_SLAB_MAX[L4S2_SLAB3]);

	// Slab 4: vertical, y = 320 .. 453
	moveSlabVertical4S2(L4S2_SLAB4, L4S2_SLAB_MIN[L4S2_SLAB4], L4S2_SLAB_MAX[L4S2_SLAB4]);

	// Slab 5: horizontal, x = 310 .. 620
	moveSlabHorizontal4S2(L4S2_SLAB5, L4S2_SLAB_MIN[L4S2_SLAB5], L4S2_SLAB_MAX[L4S2_SLAB5]);

	// Carry the player when standing on a moving slab. A player can only
	// ever really be standing on one slab - stop at the first match so a
	// coincidental height match with a second slab (however unlikely) can
	// never add a second, unwanted nudge on top of the real one.
	for (int i = 1; i < L4S2_PLATFORM_COUNT; i++) {
		if (playerOnSlab[i]) {
			player.x += slabDeltaX4S2[i];
			player.y += slabDeltaY4S2[i];
			break;
		}
	}

	// Move slab-attached gold balls
	for (int i = 0; i < 3; i++) {
		if (!goldBalls4S2[i].collected) {
			int p = goldBalls4S2[i].platformIndex;
			goldBalls4S2[i].x += slabDeltaX4S2[p];
			goldBalls4S2[i].y += slabDeltaY4S2[p];
		}
	}

	// Move slab-attached blue balls
	for (int i = 0; i < 9; i++) {
		if (!blueBalls4S2[i].collected) {
			int p = blueBalls4S2[i].platformIndex;
			blueBalls4S2[i].x += slabDeltaX4S2[p];
			blueBalls4S2[i].y += slabDeltaY4S2[p];
		}
	}
}

static void drawPlatforms4S2() {
	if (slabTexture4S2 <= 0) return;

	for (int i = 1; i < L4S2_PLATFORM_COUNT; i++) {
		int width = level4sub2_platforms[i].x2 - level4sub2_platforms[i].x1;
		int height = level4sub2_platforms[i].y2 - level4sub2_platforms[i].y1;
		iShowImage(level4sub2_platforms[i].x1, level4sub2_platforms[i].y1, width, height, slabTexture4S2);
	}
}

static void updateLevel4Sub2() {
	if (isLevel4Sub2Transition) {
		level4Sub2TransitionCounter++;
		if (level4Sub2TransitionCounter >= 100) {
			isLevel4Sub2Transition = false;
		}
		return;
	}

	if (isGameOver4S2 || isPaused4S2 || showLevel4Sub2ExitTransition) return;

	updateMovingSlabs4S2();

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

	if (player.x < L4S2_LEFT) player.x = L4S2_LEFT;

	if (player.x + player.width > L4S2_RIGHT) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level4Sub2Complete && atPortalCoordinates)) {
			player.x = L4S2_RIGHT - player.width;
		}
	}

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved4S2 += moveDist;
		while (distanceMoved4S2 >= 150) {
			distanceMoved4S2 -= 150;
			if (energyFrame4S2 < 145) {
				energyFrame4S2++;
			}
		}
		if (energyFrame4S2 >= 145) {
			isGameOver4S2 = true;
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

	resolvePlatformCollision4S2();

	if (player.y + player.height > L4S2_TOP) {
		player.y = L4S2_TOP - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateFightingAnimation4S2();
	updateDragon4S2();

	// --- FIREBALL & PARTICLE UPDATES ---
	updateFireballs4S2();
	updateTrailParticles4S2();
	updateExplosionParticles4S2();
	checkFireballCollision4S2();

	checkDragonPlayerCollision4S2();
	checkGoldBallCollision4S2();
	checkBlueBallCollision4S2();
	checkPortalCollision4S2();
}

static void drawPauseMenu4S2() {
	drawPauseMenuGeneric(resumeTexture4S2, pauseResumeBtn4S2, restartTexture4S2, pauseRestartBtn4S2, exitTexture4S2, pauseExitBtn4S2);
}

static void drawLevel4Sub2() {
	drawLevel4Sub2Background();

	if (isLevel4Sub2Transition) {
		// No banner between the two bonus trials - kept only so the
		// state machine above (dead now that the flag stays false) has
		// a matching draw branch if it's ever re-enabled.
		return;
	}

	drawPlatforms4S2();
	drawDragon4S2();
	drawFireballs4S2(); // Draws trails, explosions, and the fireballs themselves
	drawGoldBalls4S2();
	drawBlueBalls4S2();
	drawPortal4S2();

	if (!(isFighting4S2 && drawLaserFightSprite(fightingTextures4S2, fightFrame4S2, player.facingRight))) {
		drawGunPlayerBody();
	}
	drawLaserProjectiles();

	drawHUD4S2();
	drawGoldBallHUD4S2();
	drawPointsHUD4S2();

	if (isPaused4S2) {
		drawPauseMenu4S2();
	}
	else if (showLevel4Sub2ExitTransition) {
		// The true end of the Level 4 arc (main level + both bonus
		// trials) - this is the only banner that says "Level 4
		// Completed / Level 5 Starts".
		if (level4CompletedTexture4S2 > 0) {
			iShowImage(165, 270, 350, 70, level4CompletedTexture4S2);
		}
		else {
			iSetColor(255, 255, 255);
			iText(265, 305, "Level 4 Completed", GLUT_BITMAP_HELVETICA_18);
		}

		if (level5StartsTexture4S2 > 0) {
			iShowImage(190, 190, 300, 60, level5StartsTexture4S2);
		}
		else {
			iSetColor(255, 255, 255);
			iText(290, 215, "Level 5 Starts", GLUT_BITMAP_HELVETICA_18);
		}
	}
	else if (isGameOver4S2) {
		drawTotalPointsBoxGeneric(totalPointsTexture4S2, playerPoints4S2, 405);

		if (gameOverTexture4S2 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTexture4S2);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTexture4S2 > 0)
			iShowImage(gameOverRestartBtn4S2.x1, gameOverRestartBtn4S2.y1, 220, 50, gameOverRestartTexture4S2);

		if (gameOverExitTexture4S2 > 0)
			iShowImage(gameOverExitBtn4S2.x1, gameOverExitBtn4S2.y1, 150, 35, gameOverExitTexture4S2);
	}
}

#endif