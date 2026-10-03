#ifndef LEVEL4_H
#define LEVEL4_H

#include <math.h> 
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

#define LEVEL4_PLATFORM_COUNT 4
#define GROUND_PLATFORM 0
#define SLAB1_PLATFORM  1
#define SLAB2_PLATFORM  2
#define SLAB3_PLATFORM  3
#define IMG_LV4_BG "Images/lv4_bg.bmp"

// Bottom ground is fixed. Floating slabs are 150 x 23.
static Platform level4_platforms[LEVEL4_PLATFORM_COUNT] = {
	{ 20, 20, 680, 20 },       // Fixed bottom ground
	{ 70, 140, 220, 163 },     // Slab 1: moves horizontally
	{ 275, 250, 425, 273 },    // Slab 2: moves vertically
	{ 480, 360, 630, 383 }     // Slab 3: moves horizontally
};

static const Platform level4_platformStart[LEVEL4_PLATFORM_COUNT] = {
	{ 20, 20, 680, 70 },
	{ 70, 140, 220, 163 },
	{ 275, 250, 425, 273 },
	{ 480, 360, 630, 383 }
};

static int slabDirection4[LEVEL4_PLATFORM_COUNT] = { 0, 1, 1, -1 };
// Slabs move ~20% faster than the original 2 px/frame (2 * 1.2 = 2.4).
// Kept as a float speed + per-slab fractional carry so the extra 0.4
// px/frame accumulates precisely instead of being rounded away every
// frame (see slabStep4()).
#define SLAB_SPEED_SCALE4 1.2f
static float slabSpeed4[LEVEL4_PLATFORM_COUNT] = { 0.0f, 2 * SLAB_SPEED_SCALE4, 2 * SLAB_SPEED_SCALE4, 2 * SLAB_SPEED_SCALE4 };
static float slabCarry4[LEVEL4_PLATFORM_COUNT] = { 0.0f };
static int slabDeltaX4[LEVEL4_PLATFORM_COUNT] = { 0 };
static int slabDeltaY4[LEVEL4_PLATFORM_COUNT] = { 0 };

static int slabTexture4 = 0;

#define LEVEL4_LEFT   20
#define LEVEL4_RIGHT  680
#define LEVEL4_BOTTOM 25
#define LEVEL4_TOP    480

// ===== Dragon Frame Boundaries =====
#define DRAGON_LEFT_START   1
#define DRAGON_LEFT_END     17
#define DRAGON_RIGHT_START  31
#define DRAGON_RIGHT_END    63
#define TOTAL_DRAGON_FRAMES 65

#define DRAGON_SPEED        2
#define DRAGON_ANIM_SPEED   3 
#define DRAGON_COUNT        3

// ===== Dragon Health Bar Sprites =====
static int dragonHealthBarTextures4[6] = { 0 }; // 1.png (5 lives) to 5.png (1 live)

// ===== Fireball Breath Attack =====
// The dragons throw a fireball at the player whenever their flight
// animation reaches frame 16 (the last frame of the left-facing cycle)
// or frame 54 (mid right-facing cycle) - these are the "mouth open /
// breathing fire" poses in the dragon sprite sheet.
#define FIREBALL_ANIM_FRAME_A 16
#define FIREBALL_ANIM_FRAME_B 54

#define MAX_FIREBALLS4 10
#define MAX_TRAIL_PARTICLES4 60
#define MAX_EXPLOSION_PARTICLES4 40

struct Fireball4 {
	int x, y;
	int width, height;
	float vx, vy;
	float rotation;
	bool active;
};

struct TrailParticle4 {
	float x, y;
	float radius;
	int alpha;
	bool active;
};

struct ExplosionParticle4 {
	float x, y;
	float vx, vy;
	float radius;
	int life;
	bool active;
};

static Fireball4 fireballs4[MAX_FIREBALLS4];
static TrailParticle4 trailParticles4[MAX_TRAIL_PARTICLES4];
static ExplosionParticle4 explosionParticles4[MAX_EXPLOSION_PARTICLES4];
static int fireballTexture4 = 0;

// Fireball hits needed to drop 1 zed. Kept separate from the melee-touch
// counter below so the two damage sources never interfere with each other.
static int fireballHitsToPlayer4 = 0;
static int fireballHitCooldown4 = 0;

// ===== Fighting & Combo Variables =====
#define TOTAL_FIGHT_FRAMES LASER_FRAMES
// Images/PlayerLaser/shoot holds the 8-frame laser attack (frames 1-8 face
// right, 9-16 are the mirrored left-facing copies). See laser_fight.h.
#define TOTAL_FIGHT_FRAMES_ALL LASER_FRAMES_ALL
static int fightingTextures4[TOTAL_FIGHT_FRAMES_ALL + 1] = { 0 };
static bool isFighting4 = false;
static bool isLeftMouseDown4 = false;
static int currentPunchCombo4 = 0;
static int fightFrame4 = 1;
static int fightAnimTimer4 = 0;
static bool punchDamageDealt4 = false;
#define FIGHT_ANIM_SPEED 7

// ===== Game State Variables =====
static bool level4Complete = false;
static bool isGameOver4 = false;
static bool isPaused4 = false;
static bool enterLevel5 = false;
static int hitCooldown4 = 0;
static int dragonHitsToPlayer4 = 0;

// ----- Exit transition (level4 completed -> level5 starts) -----
static bool showLevel4ExitTransition = false;
static int level4CompletedTexture4 = 0;
static int level5StartsTexture4 = 0;
static int level5TransitionTimerId4 = -1;

// ----- Intro transition (level3 completed -> level4 starts, shown at level start) -----
static bool isLevel4Transition = true;
static int level4TransitionCounter = 0;
static int level3CompletedTexture4 = 0;
static int level4StartsTexture4 = 0;

// ===== Player Lives, Energy & Points Variables =====
static int playerLives4 = 5;
static int& energyFrame4 = gEnergyFrame;
static int distanceMoved4 = 0;
static int energyTextures4[150] = { 0 };
static int& playerPoints4 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTexture4 = 0;
static int zedsIconTexture4 = 0;

// ===== Menu Textures & Bounding Boxes =====
static int resumeTexture4 = 0;
static int restartTexture4 = 0;
static int exitTexture4 = 0;

static Button pauseResumeBtn4 = { 260, 280, 440, 325 };
static Button pauseRestartBtn4 = { 260, 220, 440, 265 };
static Button pauseExitBtn4 = { 260, 160, 440, 205 };

// ===== Game Over Menu Textures & Bounding Boxes =====
static int totalPointsTexture4 = 0;
static int gameOverTexture4 = 0;
static int gameOverRestartTexture4 = 0;
static int gameOverExitTexture4 = 0;

static Button gameOverRestartBtn4 = { 240, 115, 460, 165 };
static Button gameOverExitBtn4 = { 275, 70, 425, 105 };

// ===== Gold Ball Collectibles, Portal & HUD =====
struct GoldBall4 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static GoldBall4 goldBalls4[3];
static int goldBallTexture4 = 0;
static int goldBallIconTextures4[5] = { 0 };
static int goldBallsCollected4 = 0;
static int dispearTexture4 = 0;

// ===== Blue Ball Collectibles & Points HUD =====
struct BlueBall4 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static BlueBall4 blueBalls4[9];
static int blueBallTexture4 = 0;
static int pointsTexture4 = 0;

struct PatrolDragon4 {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
	int health;      // Starts at 5 (5 lives)
	int punchCount;  // 5 laser hits needed per life loss
	bool alive;      // Becomes false when health hits 0
	int platformIndex;
	int minX, maxX;  // Independent horizontal patrol limits
};

static PatrolDragon4 dragons4[DRAGON_COUNT];

// Preloaded texture array
static int dragonTextures4[TOTAL_DRAGON_FRAMES];
static bool texturesLoaded4 = false;

static void loadDragonHealthBarTextures4() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 5; i++) {
		sprintf(path, "Images/Dragonhealthbar/%d.png", i);
		dragonHealthBarTextures4[i] = iLoadImage(path);

		if (dragonHealthBarTextures4[i] <= 0) {
			sprintf(path, "../Images/Dragonhealthbar/%d.png", i);
			dragonHealthBarTextures4[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadDragonTextures4() {
	if (texturesLoaded4) return;

	char path[128];
	for (int i = 1; i <= DRAGON_RIGHT_END; i++) {
		if (i >= 18 && i <= 30) {
			dragonTextures4[i] = 0;
			continue;
		}

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTextures4[i] = iLoadImage(path);

		if (dragonTextures4[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTextures4[i] = iLoadImage(path);
		}
	}

	texturesLoaded4 = true;
}

static void loadFireballTexture4() {
	static bool loaded = false;
	if (loaded) return;
	fireballTexture4 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadFightingTextures4() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= TOTAL_FIGHT_FRAMES_ALL; i++) {
		sprintf(path, "Images/PlayerLaser/shoot/%d.png", i);
		fightingTextures4[i] = iLoadImage(path);

		if (fightingTextures4[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/shoot/%d.png", i);
			fightingTextures4[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

// Laser attack frames: 1-10 face right, 11-20 are their mirrors (facing left).
static int getFightTextureIndex4(int frame, bool facingRight) {
	return laserTextureIndex(frame, facingRight);
}

static void loadTransitionTextures4() {
	static bool loaded = false;
	if (loaded) return;

	level3CompletedTexture4 = loadTex("Images/level3completed.png", "../Images/level3completed.png");
	level4StartsTexture4 = loadTex("Images/level4starts.png", "../Images/level4starts.png");

	loaded = true;
}

static void loadZedsTextures4() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTexture4 = loadTex("Images/zeds.png", "../Images/zeds.png");

	zedsIconTexture4 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture4 <= 0) zedsIconTexture4 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture4 <= 0) zedsIconTexture4 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTextures4() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures4[i] = iLoadImage(path);

		if (energyTextures4[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures4[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadSlabTexture4() {
	static bool loaded = false;
	if (loaded) return;

	slabTexture4 = loadTex("Images/slabs.png", "../Images/slabs.png");
	loaded = true;
}

static void loadPauseTextures4() {
	static bool loaded = false;
	if (loaded) return;

	resumeTexture4 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture4 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture4 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTexture4 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTexture4 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture4 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture4 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadExitTransitionTextures4() {
	static bool loaded = false;
	if (loaded) return;

	level4CompletedTexture4 = loadTex("Images/level4completed.png", "../Images/level4completed.png");
	level5StartsTexture4 = loadTex("Images/level5starts.png", "../Images/level5starts.png");

	loaded = true;
}

static void loadGoldBallTextures4() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTexture4 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture4 = loadTex("Images/dispear4.png", "../Images/dispear4.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures4[i] = iLoadImage(path);
		if (goldBallIconTextures4[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures4[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadBlueBallAndPointsTextures4() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTexture4 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture4 = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

static void initGoldAndBlueBalls4(bool freshStart) {
	loadGoldBallTextures4();
	loadBlueBallAndPointsTextures4();
	goldBallsCollected4 = 0;
	if (freshStart) playerPoints4 = 0;
	enterLevel5 = false;

	struct ItemRect {
		int x, y, w, h;
		int platformIndex;
	};
	ItemRect placed[12];
	int placedCount = 0;

	const int MIN_DIST = 45; // Pixel clearance required between any two collectibles

	// 1. GOLD BALLS (Placed off-screen initially, spawn randomly when dragons die)
	for (int i = 0; i < 3; i++) {
		goldBalls4[i].width = 25;
		goldBalls4[i].height = 25;
		goldBalls4[i].x = -1000;
		goldBalls4[i].y = -1000;
		goldBalls4[i].collected = false;
		goldBalls4[i].platformIndex = GROUND_PLATFORM;
	}

	// 2. BLUE BALLS ON SLABS (3 Blue Balls)
	for (int i = 0; i < 3; i++) {
		bool valid = false;
		int rx = 0, ry = 0;
		int attempts = 0;
		int pIdx = SLAB1_PLATFORM;

		while (!valid && attempts < 300) {
			attempts++;
			pIdx = SLAB1_PLATFORM + (rand() % 3);
			Platform p = level4_platforms[pIdx];
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

		blueBalls4[i].width = 18;
		blueBalls4[i].height = 18;
		blueBalls4[i].x = rx;
		blueBalls4[i].y = ry;
		blueBalls4[i].collected = false;
		blueBalls4[i].platformIndex = pIdx;

		placed[placedCount++] = { rx, ry, 18, 18, pIdx };
	}

	// 3. BLUE BALLS IN MID-AIR (6 Blue Balls)
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

		blueBalls4[i].width = 18;
		blueBalls4[i].height = 18;
		blueBalls4[i].x = rx;
		blueBalls4[i].y = ry;
		blueBalls4[i].collected = false;
		blueBalls4[i].platformIndex = GROUND_PLATFORM;

		placed[placedCount++] = { rx, ry, 18, 18, GROUND_PLATFORM };
	}
}

static void checkGoldBallCollision4() {
	checkGoldBallCollisionGeneric(goldBalls4, &goldBallsCollected4, &level4Complete);
}

static void checkBlueBallCollision4() {
	checkBlueBallCollisionGeneric(blueBalls4, &playerPoints4);
}

static void drawGoldBalls4() {
	drawGoldBallsGeneric(goldBalls4, goldBallTexture4);
}

static void drawBlueBalls4() {
	drawBlueBallsGeneric(blueBalls4, blueBallTexture4);
}

static void drawPortal4() {
	if (level4Complete) {
		if (dispearTexture4 > 0) {
			iShowImage(680, 20, 35, 100, dispearTexture4);
		}
		else {
			iSetColor(255, 255, 0);
			iRectangle(645, 20, 35, 100);
		}
	}
}

static void triggerEnterLevel5() {
	enterLevel5 = true;
	showLevel4ExitTransition = false;

	if (level5TransitionTimerId4 >= 0) {
		iPauseTimer(level5TransitionTimerId4);
	}
}

static void checkPortalCollision4() {
	if (level4Complete && !showLevel4ExitTransition) {
		bool collideX = (player.x + player.width >= 700);
		bool collideY = (player.y <= 120 && player.y + player.height >= 20);

		if (collideX && collideY) {
			showLevel4ExitTransition = true;
			loadExitTransitionTextures4();

			if (level5TransitionTimerId4 < 0) {
				level5TransitionTimerId4 = iSetTimer(1000, triggerEnterLevel5);
			}
			else {
				iResumeTimer(level5TransitionTimerId4);
			}
		}
	}
}

static void drawGoldBallHUD4() {
	drawGoldBallHUDGeneric(goldBallsCollected4, goldBallIconTextures4);
}

static void drawPointsHUD4() {
	drawPointsHUDGeneric(pointsTexture4, playerPoints4);
}

static void stepNextFrame4(int i) {
	int safetyCounter = 0;
	do {
		dragons4[i].animFrame++;

		if (dragons4[i].movingRight) {
			if (dragons4[i].animFrame > DRAGON_RIGHT_END || dragons4[i].animFrame < DRAGON_RIGHT_START)
				dragons4[i].animFrame = DRAGON_RIGHT_START;
		}
		else {
			if (dragons4[i].animFrame > DRAGON_LEFT_END || dragons4[i].animFrame < DRAGON_LEFT_START)
				dragons4[i].animFrame = DRAGON_LEFT_START;
		}

		safetyCounter++;
		if (safetyCounter > TOTAL_DRAGON_FRAMES) break;
	} while (dragonTextures4[dragons4[i].animFrame] <= 0);
}

static void initDragon4() {
	loadDragonTextures4();
	loadDragonHealthBarTextures4();

	// Dragon 0: Ground Patrol (Fixed Y = 20)
	dragons4[0].width = 70;
	dragons4[0].height = 70;
	dragons4[0].x = 100;
	dragons4[0].y = 20;
	dragons4[0].minX = 40;
	dragons4[0].maxX = 600;
	dragons4[0].movingRight = true;
	dragons4[0].animFrame = DRAGON_RIGHT_START;
	dragons4[0].animTimer = 0;
	dragons4[0].health = 5;
	dragons4[0].punchCount = 0;
	dragons4[0].alive = true;
	dragons4[0].platformIndex = GROUND_PLATFORM;
	if (dragonTextures4[dragons4[0].animFrame] <= 0) stepNextFrame4(0);

	// Dragon 1: Middle Level Air Patrol (Fixed Y = 210)
	dragons4[1].width = 70;
	dragons4[1].height = 70;
	dragons4[1].x = 260;
	dragons4[1].y = 300;
	dragons4[1].minX = 200;
	dragons4[1].maxX = 460;
	dragons4[1].movingRight = true;
	dragons4[1].animFrame = DRAGON_RIGHT_START;
	dragons4[1].animTimer = 0;
	dragons4[1].health = 5;
	dragons4[1].punchCount = 0;
	dragons4[1].alive = true;
	dragons4[1].platformIndex = GROUND_PLATFORM;
	if (dragonTextures4[dragons4[1].animFrame] <= 0) stepNextFrame4(1);

	// Dragon 2: Top Level Air Patrol (Fixed Y = 320)
	dragons4[2].width = 70;
	dragons4[2].height = 70;
	dragons4[2].x = 480;
	dragons4[2].y = 380;
	dragons4[2].minX = 420;
	dragons4[2].maxX = 640;
	dragons4[2].movingRight = true;
	dragons4[2].animFrame = DRAGON_RIGHT_START;
	dragons4[2].animTimer = 0;
	dragons4[2].health = 5;
	dragons4[2].punchCount = 0;
	dragons4[2].alive = true;
	dragons4[2].platformIndex = GROUND_PLATFORM;
	if (dragonTextures4[dragons4[2].animFrame] <= 0) stepNextFrame4(2);
}

// =========================================================
// FIREBALL BREATH ATTACK (spawn, physics, collision, draw)
// =========================================================
static void spawnExplosion4(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES4; j++) {
			if (!explosionParticles4[j].active) {
				explosionParticles4[j].x = x;
				explosionParticles4[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticles4[j].vx = cos(angle) * speed;
				explosionParticles4[j].vy = sin(angle) * speed;
				explosionParticles4[j].radius = 3.0f + (rand() % 3);
				explosionParticles4[j].life = 20 + (rand() % 10);
				explosionParticles4[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticle4(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES4; i++) {
		if (!trailParticles4[i].active) {
			trailParticles4[i].x = x + (rand() % 6 - 3);
			trailParticles4[i].y = y + (rand() % 6 - 3);
			trailParticles4[i].radius = 4.0f + (rand() % 4);
			trailParticles4[i].alpha = 255;
			trailParticles4[i].active = true;
			break;
		}
	}
}

// Called the instant a dragon's animation reaches its breath-attack frame
// (16 or 54). Spawns one fireball arcing toward the player's current position.
static void spawnFireball4(int dragonIndex) {
	for (int j = 0; j < MAX_FIREBALLS4; j++) {
		if (!fireballs4[j].active) {
			fireballs4[j].x = dragons4[dragonIndex].x + dragons4[dragonIndex].width / 2 - 10;
			fireballs4[j].y = dragons4[dragonIndex].y + dragons4[dragonIndex].height / 2 - 10;
			fireballs4[j].width = 20; fireballs4[j].height = 20;

			float dx = (player.x + player.width / 2.0f) - (fireballs4[j].x + 10);
			float dy = (player.y + player.height / 2.0f) - (fireballs4[j].y + 10);
			float dist = sqrt(dx * dx + dy * dy);
			if (dist == 0) dist = 1;

			float speed = 7.0f;
			fireballs4[j].vx = (dx / dist) * speed;
			// Arc boost for a parabola instead of a flat, unavoidable line shot
			fireballs4[j].vy = (dy / dist) * speed + 4.5f;
			fireballs4[j].rotation = 0.0f;
			fireballs4[j].active = true;
			break;
		}
	}
}

static void updateFireballs4() {
	for (int i = 0; i < MAX_FIREBALLS4; i++) {
		if (!fireballs4[i].active) continue;

		int oldY = fireballs4[i].y;

		// Gravity
		fireballs4[i].vy -= 0.2f;

		fireballs4[i].x += fireballs4[i].vx;
		fireballs4[i].y += fireballs4[i].vy;
		fireballs4[i].rotation += fireballs4[i].vx * 6.0f;

		// Bounce off the ground
		if (fireballs4[i].y <= LEVEL4_BOTTOM) {
			fireballs4[i].y = LEVEL4_BOTTOM;
			fireballs4[i].vy = -fireballs4[i].vy * 0.5f;
			fireballs4[i].vx *= 0.8f;
			spawnExplosion4(fireballs4[i].x + 10, fireballs4[i].y + 10);
			if (abs(fireballs4[i].vy) < 1.5f) {
				fireballs4[i].active = false;
			}
		}

		// Explode against the floating slabs / ground platform
		if (fireballs4[i].active && fireballs4[i].vy < 0) {
			for (int p = 0; p < LEVEL4_PLATFORM_COUNT; p++) {
				Platform plat = level4_platforms[p];

				bool withinX = (fireballs4[i].x + fireballs4[i].width > plat.x1) && (fireballs4[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballs4[i].y <= plat.y2) {
					fireballs4[i].y = plat.y2;
					spawnExplosion4(fireballs4[i].x + 10, fireballs4[i].y + 10);
					fireballs4[i].active = false;
					break;
				}
			}
		}

		if (fireballs4[i].x < -50 || fireballs4[i].x > 750) {
			fireballs4[i].active = false;
		}

		if (fireballs4[i].active)
			spawnTrailParticle4(fireballs4[i].x + 10, fireballs4[i].y + 10);
	}
}

static void updateTrailParticles4() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES4; i++) {
		if (!trailParticles4[i].active) continue;
		trailParticles4[i].alpha -= 15;
		trailParticles4[i].radius -= 0.2f;
		if (trailParticles4[i].alpha <= 0 || trailParticles4[i].radius <= 0) {
			trailParticles4[i].active = false;
		}
	}
}

static void updateExplosionParticles4() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4; i++) {
		if (!explosionParticles4[i].active) continue;
		explosionParticles4[i].x += explosionParticles4[i].vx;
		explosionParticles4[i].y += explosionParticles4[i].vy;
		explosionParticles4[i].vy -= 0.1f;
		explosionParticles4[i].life--;
		explosionParticles4[i].radius -= 0.15f;
		if (explosionParticles4[i].life <= 0 || explosionParticles4[i].radius <= 0) {
			explosionParticles4[i].active = false;
		}
	}
}

// Fireball hits: 8 hits to lose 1 zed (kept independent of the melee-touch
// counter in checkDragonPlayerCollision4).
static void checkFireballCollision4() {
	if (fireballHitCooldown4 > 0) {
		fireballHitCooldown4--;
		return;
	}

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS4; i++) {
		if (!fireballs4[i].active) continue;

		bool collideX = (px + pw > fireballs4[i].x) && (px < fireballs4[i].x + fireballs4[i].width);
		bool collideY = (py + ph > fireballs4[i].y) && (py < fireballs4[i].y + fireballs4[i].height);

		if (collideX && collideY) {
			fireballHitsToPlayer4++;

			// 8 fireball hits to lose 1 zed (life)
			if (fireballHitsToPlayer4 >= 8) {
				playerLives4--;
				fireballHitsToPlayer4 = 0;

				if (playerLives4 <= 0) {
					playerLives4 = 0;
					isGameOver4 = true;
				}
			}

			fireballHitCooldown4 = 30;
			spawnExplosion4(fireballs4[i].x + 10, fireballs4[i].y + 10);
			fireballs4[i].active = false;
			break;
		}
	}
}

static void drawFireballs4() {
	// 1. Trail
	for (int i = 0; i < MAX_TRAIL_PARTICLES4; i++) {
		if (!trailParticles4[i].active) continue;
		int alpha = trailParticles4[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticles4[i].x, trailParticles4[i].y, trailParticles4[i].radius, 20);
	}

	// 2. Explosions
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4; i++) {
		if (!explosionParticles4[i].active) continue;
		if (explosionParticles4[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticles4[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticles4[i].x, explosionParticles4[i].y, explosionParticles4[i].radius, 20);
	}

	// 3. Main fireballs
	for (int i = 0; i < MAX_FIREBALLS4; i++) {
		if (!fireballs4[i].active) continue;
		float cx = fireballs4[i].x + 10;
		float cy = fireballs4[i].y + 10;

		if (fireballTexture4 > 0) {
			iRotate(cx, cy, fireballs4[i].rotation);
			iShowImage(fireballs4[i].x, fireballs4[i].y, fireballs4[i].width, fireballs4[i].height, fireballTexture4);
			iUnRotate();
		}
		else {
			iSetColor(200, 50, 0); iFilledCircle(cx, cy, 14, 50);
			iSetColor(255, 120, 0); iFilledCircle(cx, cy, 10, 50);
			iSetColor(255, 255, 150); iFilledCircle(cx, cy, 5, 50);
		}
	}
}

static void updateDragon4() {
	for (int i = 0; i < DRAGON_COUNT; i++) {
		if (!dragons4[i].alive) continue;

		int minX = dragons4[i].minX;
		int maxX = dragons4[i].maxX - dragons4[i].width;

		if (dragons4[i].movingRight) {
			dragons4[i].x += DRAGON_SPEED;
			if (dragons4[i].x >= maxX) {
				dragons4[i].x = maxX;
				dragons4[i].movingRight = false;
				dragons4[i].animFrame = DRAGON_LEFT_START;
				if (dragonTextures4[dragons4[i].animFrame] <= 0) stepNextFrame4(i);
			}
		}
		else {
			dragons4[i].x -= DRAGON_SPEED;
			if (dragons4[i].x <= minX) {
				dragons4[i].x = minX;
				dragons4[i].movingRight = true;
				dragons4[i].animFrame = DRAGON_RIGHT_START;
				if (dragonTextures4[dragons4[i].animFrame] <= 0) stepNextFrame4(i);
			}
		}

		dragons4[i].animTimer++;
		if (dragons4[i].animTimer >= DRAGON_ANIM_SPEED) {
			dragons4[i].animTimer = 0;
			stepNextFrame4(i);

			// Breath-attack poses: the instant the animation reaches frame
			// 16 or 54, throw one fireball at the player.
			// 20% fewer fireballs overall: 1-in-5 chance to skip the throw.
			if (dragons4[i].animFrame == FIREBALL_ANIM_FRAME_A || dragons4[i].animFrame == FIREBALL_ANIM_FRAME_B) {
				if (rand() % 5 != 0) spawnFireball4(i);
			}
		}
	}
}

// Called by laserUpdateProjectiles() with the laser projectile's rectangle.
// 5 laser hits = 1 dragon life. Returns true if a dragon was hit.
static bool checkPlayerPunchDragonCollision4(int attackX, int attackY, int attackW, int attackH) {
	for (int i = 0; i < DRAGON_COUNT; i++) {
		if (!dragons4[i].alive) continue;

		int dx = dragons4[i].x;
		int dy = dragons4[i].y;
		int dw = dragons4[i].width;
		int dh = dragons4[i].height;

		bool collideX = (attackX + attackW > dx) && (attackX < dx + dw);
		bool collideY = (attackY + attackH > dy) && (attackY < dy + dh);

		if (collideX && collideY) {
			dragons4[i].punchCount++;

			if (dragons4[i].punchCount >= 5) {
				dragons4[i].punchCount = 0;
				dragons4[i].health--;

				if (dragons4[i].health <= 0) {
					dragons4[i].health = 0;
					dragons4[i].alive = false;

					// Spawn the gold ball at a random reachable position on screen
					goldBalls4[i].x = 60 + (rand() % 560);
					goldBalls4[i].y = 100 + (rand() % 280);
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

static void updateFightingAnimation4() {
	laserFightStep(isFighting4, isLeftMouseDown4, fightFrame4, fightAnimTimer4, punchDamageDealt4);
	laserUpdateProjectiles(checkPlayerPunchDragonCollision4);   // moves the lasers + hit test
}

static void handleMouseClickLevel4(int button, int state) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			isLeftMouseDown4 = true;
			if (!isFighting4 && !isPaused4 && !isGameOver4 && !showLevel4ExitTransition && !isLevel4Transition) {
				isFighting4 = true;
				fightAnimTimer4 = 0;
				punchDamageDealt4 = false;
				fightFrame4 = 1;
			}
		}
		else if (state == GLUT_UP) {
			isLeftMouseDown4 = false;
		}
	}
}

static void drawDragon4() {
	for (int i = 0; i < DRAGON_COUNT; i++) {
		if (!dragons4[i].alive) continue;

		int textureId = dragonTextures4[dragons4[i].animFrame];

		if (textureId > 0) {
			iShowImage(dragons4[i].x, dragons4[i].y, dragons4[i].width, dragons4[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragons4[i].x, dragons4[i].y, dragons4[i].width, dragons4[i].height);
		}

		// Calculate health bar sprite index:
		// health 5 -> 1.png, health 4 -> 2.png, etc.
		int spriteIdx = 6 - dragons4[i].health;

		if (spriteIdx >= 1 && spriteIdx <= 5 && dragonHealthBarTextures4[spriteIdx] > 0) {
			// Updated size to 50x8 to match image proportions
			iShowImage(dragons4[i].x + 10, dragons4[i].y + dragons4[i].height + 5, 50, 8, dragonHealthBarTextures4[spriteIdx]);
		}
	}
}

static void checkDragonPlayerCollision4() {
	if (hitCooldown4 > 0) {
		hitCooldown4--;
		return;
	}

	for (int i = 0; i < DRAGON_COUNT; i++) {
		if (!dragons4[i].alive) continue;

		int hx = dragons4[i].x;
		int hy = dragons4[i].y;
		int hw = dragons4[i].width;
		int hh = dragons4[i].height;

		if (dragons4[i].animFrame >= 1 && dragons4[i].animFrame <= 17) {
			hx = dragons4[i].x + (int)(dragons4[i].width * 0.556f);
			hy = dragons4[i].y + (int)(dragons4[i].height * 0.312f);
			hw = (int)(dragons4[i].width * 0.363f);
			hh = (int)(dragons4[i].height * 0.347f);
		}
		else if (dragons4[i].animFrame >= DRAGON_RIGHT_START && dragons4[i].animFrame <= DRAGON_RIGHT_END) {
			hx = dragons4[i].x + (int)(dragons4[i].width * 0.081f);
			hy = dragons4[i].y + (int)(dragons4[i].height * 0.312f);
			hw = (int)(dragons4[i].width * 0.363f);
			hh = (int)(dragons4[i].height * 0.347f);
		}

		bool collideX = (player.x + player.width > hx) && (player.x < hx + hw);
		bool collideY = (player.y + player.height > hy) && (player.y < hy + hh);

		if (collideX && collideY) {
			dragonHitsToPlayer4++;

			// 8 touches to lose 1 zed (life)
			if (dragonHitsToPlayer4 >= 8) {
				playerLives4--;
				dragonHitsToPlayer4 = 0;

				if (playerLives4 <= 0) {
					playerLives4 = 0;
					isGameOver4 = true;
				}
			}

			hitCooldown4 = 30;
			break;
		}
	}
}

static void drawHUD4() {
	drawHUDGeneric(zedsLabelTexture4, zedsIconTexture4, playerLives4, energyFrame4, energyTextures4);
}

static void initLevel4(bool freshStart = true) {
	for (int i = 0; i < LEVEL4_PLATFORM_COUNT; i++) {
		level4_platforms[i] = level4_platformStart[i];
		slabDirection4[i] = (i == SLAB3_PLATFORM) ? -1 : (i == GROUND_PLATFORM ? 0 : 1);
		slabCarry4[i] = 0.0f;
	}

	initPlayer(40, LEVEL4_BOTTOM);
	initDragon4();
	loadSlabTexture4();
	initGoldAndBlueBalls4(freshStart);
	loadPauseTextures4();
	loadZedsTextures4();
	loadEnergyTextures4();
	loadFightingTextures4();
	loadFireballTexture4();
	loadExitTransitionTextures4();
	loadTransitionTextures4();

	for (int i = 0; i < MAX_FIREBALLS4; i++) fireballs4[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES4; i++) trailParticles4[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES4; i++) explosionParticles4[i].active = false;
	fireballHitsToPlayer4 = 0;
	fireballHitCooldown4 = 0;

	level4Complete = false;
	isGameOver4 = false;
	isPaused4 = false;
	hitCooldown4 = 0;
	dragonHitsToPlayer4 = 0;

	showLevel4ExitTransition = false;
	if (level5TransitionTimerId4 >= 0) {
		iPauseTimer(level5TransitionTimerId4);
	}

	isLevel4Transition = true;
	level4TransitionCounter = 0;

	currentPunchCombo4 = 0;
	isFighting4 = false;
	punchDamageDealt4 = false;
	laserResetAll();

	playerLives4 = 5;
	energyFrame4 = 1;   // energy refills (resets) at the start of every level-4 stage
	distanceMoved4 = 0;
}

static void drawLevel4Background() {
	iShowBMP(0, 0, IMG_LV4_BG);
}

static void resolvePlatformCollision4() {
	int oldY = player.y - player.velocityY;
	int oldTopY = oldY + player.height;
	int currentTopY = player.y + player.height;

	if (player.y <= LEVEL4_BOTTOM) {
		player.y = LEVEL4_BOTTOM;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	player.onGround = false;

	for (int i = 1; i < LEVEL4_PLATFORM_COUNT; i++) {
		Platform p = level4_platforms[i];
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

static bool playerStandingOnPlatform4(int index) {
	Platform p = level4_platforms[index];
	bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
	int playerBottom = player.y;
	return withinX && abs(playerBottom - p.y2) <= 3 && player.velocityY == 0;
}

// Returns this frame's integer displacement for slab i, folding in the
// fractional part (2.4 px/frame) that a plain int step would otherwise
// lose every other frame, so the average speed is exactly slabSpeed4[i].
static int slabStep4(int i) {
	float raw = slabSpeed4[i] * slabDirection4[i] + slabCarry4[i];
	int step = (int)raw;
	slabCarry4[i] = raw - step;
	return step;
}

static void updateMovingSlabs4() {
	for (int i = 0; i < LEVEL4_PLATFORM_COUNT; i++) {
		slabDeltaX4[i] = 0;
		slabDeltaY4[i] = 0;
	}

	bool playerOnSlab[LEVEL4_PLATFORM_COUNT] = { false };
	for (int i = 1; i < LEVEL4_PLATFORM_COUNT; i++) {
		playerOnSlab[i] = playerStandingOnPlatform4(i);
	}

	// Slab 1: horizontal, x = 40..300
	{
		int i = SLAB1_PLATFORM;
		int dx = slabStep4(i);
		level4_platforms[i].x1 += dx;
		level4_platforms[i].x2 += dx;

		if (level4_platforms[i].x1 <= 40) {
			dx += 40 - level4_platforms[i].x1;
			level4_platforms[i].x1 = 40;
			level4_platforms[i].x2 = 190;
			slabDirection4[i] = 1;
		}
		else if (level4_platforms[i].x2 >= 300) {
			dx -= level4_platforms[i].x2 - 300;
			level4_platforms[i].x1 = 150;
			level4_platforms[i].x2 = 300;
			slabDirection4[i] = -1;
		}
		slabDeltaX4[i] = dx;
	}

	// Slab 2: vertical, y = 190..330
	{
		int i = SLAB2_PLATFORM;
		int dy = slabStep4(i);
		level4_platforms[i].y1 += dy;
		level4_platforms[i].y2 += dy;

		if (level4_platforms[i].y1 <= 190) {
			dy += 190 - level4_platforms[i].y1;
			level4_platforms[i].y1 = 190;
			level4_platforms[i].y2 = 213;
			slabDirection4[i] = 1;
		}
		else if (level4_platforms[i].y2 >= 330) {
			dy -= level4_platforms[i].y2 - 330;
			level4_platforms[i].y1 = 307;
			level4_platforms[i].y2 = 330;
			slabDirection4[i] = -1;
		}
		slabDeltaY4[i] = dy;
	}

	// Slab 3: horizontal, x = 400..660
	{
		int i = SLAB3_PLATFORM;
		int dx = slabStep4(i);
		level4_platforms[i].x1 += dx;
		level4_platforms[i].x2 += dx;

		if (level4_platforms[i].x1 <= 400) {
			dx += 400 - level4_platforms[i].x1;
			level4_platforms[i].x1 = 400;
			level4_platforms[i].x2 = 550;
			slabDirection4[i] = 1;
		}
		else if (level4_platforms[i].x2 >= 660) {
			dx -= level4_platforms[i].x2 - 660;
			level4_platforms[i].x1 = 510;
			level4_platforms[i].x2 = 660;
			slabDirection4[i] = -1;
		}
		slabDeltaX4[i] = dx;
	}

	// Carry player when standing on moving slab
	for (int i = 1; i < LEVEL4_PLATFORM_COUNT; i++) {
		if (playerOnSlab[i]) {
			player.x += slabDeltaX4[i];
			player.y += slabDeltaY4[i];
		}
	}

	// Move slab-attached gold balls
	for (int i = 0; i < 3; i++) {
		if (!goldBalls4[i].collected) {
			int p = goldBalls4[i].platformIndex;
			goldBalls4[i].x += slabDeltaX4[p];
			goldBalls4[i].y += slabDeltaY4[p];
		}
	}

	// Move slab-attached blue balls
	for (int i = 0; i < 9; i++) {
		if (!blueBalls4[i].collected) {
			int p = blueBalls4[i].platformIndex;
			blueBalls4[i].x += slabDeltaX4[p];
			blueBalls4[i].y += slabDeltaY4[p];
		}
	}
}

static void drawPlatforms4() {
	if (slabTexture4 <= 0) return;

	for (int i = 1; i < LEVEL4_PLATFORM_COUNT; i++) {
		int width = level4_platforms[i].x2 - level4_platforms[i].x1;
		int height = level4_platforms[i].y2 - level4_platforms[i].y1;
		iShowImage(level4_platforms[i].x1, level4_platforms[i].y1, width, height, slabTexture4);
	}
}

static void updateLevel4() {
	if (isLevel4Transition) {
		level4TransitionCounter++;
		if (level4TransitionCounter >= 100) {
			isLevel4Transition = false;
		}
		return;
	}

	if (isGameOver4 || isPaused4 || showLevel4ExitTransition) return;

	updateMovingSlabs4();

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

	if (player.x < LEVEL4_LEFT) player.x = LEVEL4_LEFT;

	if (player.x + player.width > LEVEL4_RIGHT) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level4Complete && atPortalCoordinates)) {
			player.x = LEVEL4_RIGHT - player.width;
		}
	}

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved4 += moveDist;
		while (distanceMoved4 >= 150) {
			distanceMoved4 -= 150;
			if (energyFrame4 < 145) {
				energyFrame4++;
			}
		}
		if (energyFrame4 >= 145) {
			isGameOver4 = true;
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

	resolvePlatformCollision4();

	if (player.y + player.height > LEVEL4_TOP) {
		player.y = LEVEL4_TOP - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateFightingAnimation4();
	updateDragon4();

	// --- FIREBALL & PARTICLE UPDATES ---
	updateFireballs4();
	updateTrailParticles4();
	updateExplosionParticles4();
	checkFireballCollision4();

	checkDragonPlayerCollision4();
	checkGoldBallCollision4();
	checkBlueBallCollision4();
	checkPortalCollision4();
}

static void drawPauseMenu4() {
	drawPauseMenuGeneric(resumeTexture4, pauseResumeBtn4, restartTexture4, pauseRestartBtn4, exitTexture4, pauseExitBtn4);
}

static void drawLevel4() {
	drawLevel4Background();

	if (isLevel4Transition) {
		if (level3CompletedTexture4 > 0) {
			iShowImage(175, 260, 350, 60, level3CompletedTexture4);
		}
		else {
			iSetColor(255, 255, 255);
			iText(265, 305, "Level 3 Completed", GLUT_BITMAP_HELVETICA_18);
		}

		if (level4StartsTexture4 > 0) {
			iShowImage(175, 180, 350, 60, level4StartsTexture4);
		}
		else {
			iSetColor(255, 255, 255);
			iText(290, 195, "Level 4 Starts", GLUT_BITMAP_HELVETICA_18);
		}
		return;
	}

	drawPlatforms4();
	drawDragon4();
	drawFireballs4(); // Draws trails, explosions, and the fireballs themselves
	drawGoldBalls4();
	drawBlueBalls4();
	drawPortal4();

	if (!(isFighting4 && drawLaserFightSprite(fightingTextures4, fightFrame4, player.facingRight))) {
		drawGunPlayerBody();
	}
	drawLaserProjectiles();

	drawHUD4();
	drawGoldBallHUD4();
	drawPointsHUD4();

	if (isPaused4) {
		drawPauseMenu4();
	}
	else if (showLevel4ExitTransition) {
		// Quietly transitioning into the next bonus stage (Sub 1) - the
		// "Level 4 Completed / Level 5 Starts" banner only shows once, after
		// Sub 2 is finished and the game is truly moving on to Level 5.
	}
	else if (isGameOver4) {
		drawTotalPointsBoxGeneric(totalPointsTexture4, playerPoints4, 405);

		if (gameOverTexture4 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTexture4);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTexture4 > 0)
			iShowImage(gameOverRestartBtn4.x1, gameOverRestartBtn4.y1, 220, 50, gameOverRestartTexture4);

		if (gameOverExitTexture4 > 0)
			iShowImage(gameOverExitBtn4.x1, gameOverExitBtn4.y1, 150, 35, gameOverExitTexture4);
	}
}

#endif