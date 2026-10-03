#ifndef LEVEL2_H
#define LEVEL2_H
#define IMG_LV2_BG "Images/lv2_bg.bmp"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

#define PLATFORM_COUNT_2 9
static Platform level2_platforms[PLATFORM_COUNT_2] = {
	{ 611, 326, 680, 326 },  // top-right short slab
	{ 381, 392, 488, 392 },  // top-mid L slab
	{ 71, 383, 175, 383 },   // top-left L slab
	{ 278, 279, 348, 279 },  // middle isolated slab
	{ 476, 239, 565, 239 },  // right-mid top cap slab
	{ 551, 148, 680, 148 },  // right-mid bottom foot slab
	{ 20, 246, 97, 246 },    // left-mid slab
	{ 354, 153, 443, 153 },  // bottom-mid slab
	{ 170, 153, 276, 153 }   // bottom-left L slab top
};

// Vertical wall segments
#define WALL_COUNT_2 4
static Platform level2_walls[WALL_COUNT_2] = {
	{ 154, 388, 175, 480 },
	{ 382, 397, 399, 480 },
	{ 549, 153, 565, 230 },
	{ 170, 20, 187, 153 }
};

#define LEVEL_LEFT_2   5
#define LEVEL_RIGHT_2  680
#define LEVEL_BOTTOM_2 20
#define LEVEL_TOP_2    480

// =========================================================
// DRAGON FRAME BOUNDARIES
// =========================================================
#define DRAGON_LEFT_START_2   1
#define DRAGON_LEFT_END_2     17
#define DRAGON_RIGHT_START_2  31
#define DRAGON_RIGHT_END_2    63
#define TOTAL_DRAGON_FRAMES_2 65
#define DRAGON_SPEED_2        2
#define DRAGON_ANIM_SPEED_2   3
#define DRAGON_COUNT_2        2

// =========================================================
// GAME STATE
// =========================================================
static bool level2Complete = false;
static bool isGameOver2 = false;
static bool isPaused2 = false;
static bool enterLevel3 = false;
static int hitCooldown2 = 0;
static bool isLevel2Transition = true;
static int level2TransitionCounter = 0;
static int level1CompletedTexture2 = 0;
static int level2StartsTexture2 = 0;

// ----- Exit transition (level2 completed -> level3 starts) -----
static bool showLevel2ExitTransition = false;
static int level2CompletedTexture2 = 0;
static int level3StartsTexture2 = 0;
static int level3TransitionTimerId2 = -1;

// =========================================================
// PLAYER LIVES, ENERGY & POINTS
// =========================================================
static int playerLives2 = 5;
static int& energyFrame2 = gEnergyFrame;
static int distanceMoved2 = 0;
static int energyTextures2[150] = { 0 };
static int& playerPoints2 = gPlayerPoints;

// =========================================================
// ZEDS TEXTURES
// =========================================================
static int zedsLabelTexture2 = 0;
static int zedsIconTexture2 = 0;

// =========================================================
// PAUSE MENU
// =========================================================
static int resumeTexture2 = 0;
static int restartTexture2 = 0;
static int exitTexture2 = 0;
static Button pauseResumeBtn2 = { 260, 280, 440, 325 };
static Button pauseRestartBtn2 = { 260, 220, 440, 265 };
static Button pauseExitBtn2 = { 260, 160, 440, 205 };

// =========================================================
// GAME OVER MENU
// =========================================================
static int totalPointsTexture2 = 0;
static int gameOverTexture2 = 0;
static int gameOverRestartTexture2 = 0;
static int gameOverExitTexture2 = 0;
static Button gameOverRestartBtn2 = { 240, 115, 460, 165 };
static Button gameOverExitBtn2 = { 275, 70, 425, 105 };

// =========================================================
// GOLD BALLS
// =========================================================
struct GoldBall2 {
	int x, y;
	int width, height;
	bool collected;
};
static GoldBall2 goldBalls2[3];
static int goldBallTexture2 = 0;
static int goldBallIconTextures2[5] = { 0 };
static int goldBallsCollected2 = 0;
static int dispearTexture2 = 0;

// =========================================================
// BLUE BALLS
// =========================================================
struct BlueBall2 {
	int x, y;
	int width, height;
	bool collected;
};
static BlueBall2 blueBalls2[9];
static int blueBallTexture2 = 0;
static int pointsTexture2 = 0;

// =========================================================
// DRAGONS
// =========================================================
struct PatrolDragon2 {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
	int minX, maxX;
	int shootTimer;
	int shootCooldown;
};
static PatrolDragon2 dragons2[DRAGON_COUNT_2];
static int dragonTextures2[TOTAL_DRAGON_FRAMES_2];
static bool texturesLoaded2 = false;

// =========================================================
// FIREBALL & PARTICLE VARIABLES (THEME UPGRADE)
// =========================================================
#define MAX_FIREBALLS2 10
#define MAX_TRAIL_PARTICLES2 60
#define MAX_EXPLOSION_PARTICLES2 40

struct Fireball2 {
	int x, y;
	int width, height;
	float vx, vy;
	float rotation;
	bool active;
};

struct TrailParticle2 {
	float x, y;
	float radius;
	int alpha;
	bool active;
};

struct ExplosionParticle2 {
	float x, y;
	float vx, vy;
	float radius;
	int life;
	bool active;
};

static Fireball2 fireballs2[MAX_FIREBALLS2];
static TrailParticle2 trailParticles2[MAX_TRAIL_PARTICLES2];
static ExplosionParticle2 explosionParticles2[MAX_EXPLOSION_PARTICLES2];
static int fireballTexture2 = 0;

// =========================================================
// LOAD DRAGON TEXTURES
// =========================================================
static void loadDragonTextures2() {
	if (texturesLoaded2) return;
	char path[128];
	for (int i = 1; i <= DRAGON_RIGHT_END_2; i++) {
		if (i >= 18 && i <= 30) {
			dragonTextures2[i] = 0;
			continue;
		}
		sprintf(path, "Images/dragon/%d.png", i);
		dragonTextures2[i] = iLoadImage(path);
		if (dragonTextures2[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTextures2[i] = iLoadImage(path);
		}
	}
	texturesLoaded2 = true;
}

// =========================================================
// LOAD FIREBALL TEXTURE
// =========================================================
static void loadFireballTexture2() {
	static bool loaded = false;
	if (loaded) return;
	fireballTexture2 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

// =========================================================
// TRANSITION TEXTURES
// =========================================================
static void loadTransitionTextures2() {
	static bool loaded = false;
	if (loaded) return;
	level1CompletedTexture2 = loadTex("Images/level1completed.png", "../Images/level1completed.png");
	level2StartsTexture2 = loadTex("Images/level2starts.png", "../Images/level2starts.png");
	loaded = true;
}

static void loadExitTransitionTextures2() {
	static bool loaded = false;
	if (loaded) return;
	level2CompletedTexture2 = loadTex("Images/level2completed.png", "../Images/level2completed.png");
	level3StartsTexture2 = loadTex("Images/level3starts.png", "../Images/level3starts.png");
	loaded = true;
}

static void loadZedsTextures2() {
	static bool loaded = false;
	if (loaded) return;
	zedsLabelTexture2 = loadTex("Images/zeds.png", "../Images/zeds.png");
	zedsIconTexture2 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture2 <= 0) zedsIconTexture2 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture2 <= 0) zedsIconTexture2 = iLoadImage("../Images/zedsicon.png");
	loaded = true;
}

static void loadEnergyTextures2() {
	static bool loaded = false;
	if (loaded) return;
	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures2[i] = iLoadImage(path);
		if (energyTextures2[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures2[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadPauseTextures2() {
	static bool loaded = false;
	if (loaded) return;
	resumeTexture2 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture2 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture2 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTexture2 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTexture2 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture2 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture2 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");
	loaded = true;
}

static void loadGoldBallTextures2() {
	static bool loaded = false;
	if (loaded) return;
	goldBallTexture2 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture2 = loadTex("Images/dispear.png", "../Images/dispear.png");
	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures2[i] = iLoadImage(path);
		if (goldBallIconTextures2[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures2[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadBlueBallAndPointsTextures2() {
	static bool loaded = false;
	if (loaded) return;
	blueBallTexture2 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture2 = loadTex("Images/points.png", "../Images/points.png");
	loaded = true;
}

// =========================================================
// INITIALIZE GOLD & BLUE BALLS
// =========================================================
static void initGoldAndBlueBalls2(bool freshStart) {
	loadGoldBallTextures2();
	loadBlueBallAndPointsTextures2();
	goldBallsCollected2 = 0;
	if (freshStart) playerPoints2 = 0;
	enterLevel3 = false;
	struct ItemRect { int x, y, w, h; };
	ItemRect placed[15];
	int placedCount = 0;

	goldBalls2[0].width = 25; goldBalls2[0].height = 25;
	goldBalls2[0].x = 620; goldBalls2[0].y = 351; goldBalls2[0].collected = false;
	placed[placedCount++] = { goldBalls2[0].x, goldBalls2[0].y, 25, 25 };

	goldBalls2[1].width = 25; goldBalls2[1].height = 25;
	goldBalls2[1].x = 115; goldBalls2[1].y = 408; goldBalls2[1].collected = false;
	placed[placedCount++] = { goldBalls2[1].x, goldBalls2[1].y, 25, 25 };

	goldBalls2[2].width = 25; goldBalls2[2].height = 25;
	goldBalls2[2].x = 600; goldBalls2[2].y = 173; goldBalls2[2].collected = false;
	placed[placedCount++] = { goldBalls2[2].x, goldBalls2[2].y, 25, 25 };

	blueBalls2[0].width = 18; blueBalls2[0].height = 18;
	blueBalls2[0].x = 420; blueBalls2[0].y = 397; blueBalls2[0].collected = false;
	placed[placedCount++] = { blueBalls2[0].x, blueBalls2[0].y, 18, 18 };

	blueBalls2[1].width = 18; blueBalls2[1].height = 18;
	blueBalls2[1].x = 580; blueBalls2[1].y = 153; blueBalls2[1].collected = false;
	placed[placedCount++] = { blueBalls2[1].x, blueBalls2[1].y, 18, 18 };

	blueBalls2[2].width = 18; blueBalls2[2].height = 18;
	blueBalls2[2].x = 306; blueBalls2[2].y = 284; blueBalls2[2].collected = false;
	placed[placedCount++] = { blueBalls2[2].x, blueBalls2[2].y, 18, 18 };

	for (int i = 3; i < 9; i++) {
		bool valid = false;
		int rx = 0, ry = 0, attempts = 0;
		while (!valid && attempts < 200) {
			attempts++;
			int pIdx = rand() % PLATFORM_COUNT_2;
			Platform p = level2_platforms[pIdx];
			int minX = p.x1 + 8;
			int maxX = p.x2 - 8 - 18;
			if (maxX <= minX) maxX = minX + 1;
			rx = minX + (rand() % (maxX - minX));
			ry = p.y2 + 5;
			valid = true;
			for (int j = 0; j < placedCount; j++) {
				if (rx < placed[j].x + placed[j].w + 30 && rx + 18 + 30 > placed[j].x &&
					ry < placed[j].y + placed[j].h + 30 && ry + 18 + 30 > placed[j].y) {
					valid = false; break;
				}
			}
		}
		blueBalls2[i].width = 18; blueBalls2[i].height = 18;
		blueBalls2[i].x = rx; blueBalls2[i].y = ry; blueBalls2[i].collected = false;
		placed[placedCount++] = { rx, ry, 18, 18 };
	}
}

static void checkGoldBallCollision2() { checkGoldBallCollisionGeneric(goldBalls2, &goldBallsCollected2, &level2Complete); }
static void checkBlueBallCollision2() { checkBlueBallCollisionGeneric(blueBalls2, &playerPoints2); }
static void drawGoldBalls2() { drawGoldBallsGeneric(goldBalls2, goldBallTexture2); }
static void drawBlueBalls2() { drawBlueBallsGeneric(blueBalls2, blueBallTexture2); }

// =========================================================
// PORTAL
// =========================================================
static void drawPortal2() {
	if (level2Complete) {
		if (dispearTexture2 > 0) iShowImage(680, 20, 35, 100, dispearTexture2);
		else { iSetColor(255, 255, 0); iRectangle(645, 20, 35, 100); }
	}
}

static void triggerEnterLevel3() {
	enterLevel3 = true;
	showLevel2ExitTransition = false;
	if (level3TransitionTimerId2 >= 0) iPauseTimer(level3TransitionTimerId2);
}

static void checkPortalCollision2() {
	if (level2Complete && !showLevel2ExitTransition) {
		bool collideX = (player.x + player.width >= 700);
		bool collideY = (player.y <= 120 && player.y + player.height >= 20);
		if (collideX && collideY) {
			// Level 2 main stage done: go straight to the Level 2 sub levels.
			// ("Level 2 Completed / Level 3 Starts" is now shown at the end of level2_sub2.h)
			enterLevel3 = true;
		}
	}
}

static void drawGoldBallHUD2() { drawGoldBallHUDGeneric(goldBallsCollected2, goldBallIconTextures2); }
static void drawPointsHUD2() { drawPointsHUDGeneric(pointsTexture2, playerPoints2); }

// =========================================================
// DRAGON LOGIC
// =========================================================
static void stepNextFrame2(int i) {
	int safetyCounter = 0;
	do {
		dragons2[i].animFrame++;
		if (dragons2[i].movingRight) {
			if (dragons2[i].animFrame > DRAGON_RIGHT_END_2 || dragons2[i].animFrame < DRAGON_RIGHT_START_2)
				dragons2[i].animFrame = DRAGON_RIGHT_START_2;
		}
		else {
			if (dragons2[i].animFrame > DRAGON_LEFT_END_2 || dragons2[i].animFrame < DRAGON_LEFT_START_2)
				dragons2[i].animFrame = DRAGON_LEFT_START_2;
		}
		safetyCounter++;
		if (safetyCounter > TOTAL_DRAGON_FRAMES_2) break;
	} while (dragonTextures2[dragons2[i].animFrame] <= 0);
}

static void initDragon2() {
	loadDragonTextures2();
	dragons2[0].width = 70; dragons2[0].height = 70;
	dragons2[0].x = 400; dragons2[0].y = 20;
	dragons2[0].minX = 350; dragons2[0].maxX = 610;
	dragons2[0].movingRight = true; dragons2[0].animFrame = DRAGON_RIGHT_START_2; dragons2[0].animTimer = 0;
	dragons2[0].shootTimer = 90 + (rand() % 60); dragons2[0].shootCooldown = 150;
	if (dragonTextures2[dragons2[0].animFrame] <= 0) stepNextFrame2(0);

	dragons2[1].width = 70; dragons2[1].height = 70;
	dragons2[1].x = 270; dragons2[1].y = 265;
	dragons2[1].minX = 150; dragons2[1].maxX = 480;
	dragons2[1].movingRight = true; dragons2[1].animFrame = DRAGON_RIGHT_START_2; dragons2[1].animTimer = 0;
	dragons2[1].shootTimer = 120 + (rand() % 60); dragons2[1].shootCooldown = 180;
	if (dragonTextures2[dragons2[1].animFrame] <= 0) stepNextFrame2(1);
}

static void updateDragon2() {
	for (int i = 0; i < DRAGON_COUNT_2; i++) {
		if (dragons2[i].movingRight) {
			dragons2[i].x += DRAGON_SPEED_2;
			if (dragons2[i].x >= dragons2[i].maxX) {
				dragons2[i].x = dragons2[i].maxX; dragons2[i].movingRight = false;
				dragons2[i].animFrame = DRAGON_LEFT_START_2;
				if (dragonTextures2[dragons2[i].animFrame] <= 0) stepNextFrame2(i);
			}
		}
		else {
			dragons2[i].x -= DRAGON_SPEED_2;
			if (dragons2[i].x <= dragons2[i].minX) {
				dragons2[i].x = dragons2[i].minX; dragons2[i].movingRight = true;
				dragons2[i].animFrame = DRAGON_RIGHT_START_2;
				if (dragonTextures2[dragons2[i].animFrame] <= 0) stepNextFrame2(i);
			}
		}
		dragons2[i].animTimer++;
		if (dragons2[i].animTimer >= DRAGON_ANIM_SPEED_2) {
			dragons2[i].animTimer = 0; stepNextFrame2(i);
		}

		// --- FIREBALL SHOOTING LOGIC ---
		dragons2[i].shootTimer--;
		if (dragons2[i].shootTimer <= 0) {
			for (int j = 0; j < MAX_FIREBALLS2; j++) {
				if (!fireballs2[j].active) {
					fireballs2[j].x = dragons2[i].x + dragons2[i].width / 2 - 10;
					fireballs2[j].y = dragons2[i].y + dragons2[i].height / 2 - 10;
					fireballs2[j].width = 20; fireballs2[j].height = 20;

					float dx = (player.x + player.width / 2.0f) - (fireballs2[j].x + 10);
					float dy = (player.y + player.height / 2.0f) - (fireballs2[j].y + 10);
					float dist = sqrt(dx * dx + dy * dy);
					if (dist == 0) dist = 1;

					float speed = 7.0f;
					fireballs2[j].vx = (dx / dist) * speed;
					// Arc boost: adds upward velocity so it flies in a parabola
					fireballs2[j].vy = (dy / dist) * speed + 4.5f;
					fireballs2[j].rotation = 0.0f;
					fireballs2[j].active = true;
					break;
				}
			}
			dragons2[i].shootTimer = dragons2[i].shootCooldown;
		}
	}
}

static void drawDragon2() {
	for (int i = 0; i < DRAGON_COUNT_2; i++) {
		int textureId = dragonTextures2[dragons2[i].animFrame];
		if (textureId > 0) {
			iShowImage(dragons2[i].x, dragons2[i].y, dragons2[i].width, dragons2[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragons2[i].x, dragons2[i].y, dragons2[i].width, dragons2[i].height);
			iSetColor(255, 255, 255);
			iText(dragons2[i].x + 5, dragons2[i].y + 25, "DRAGON", GLUT_BITMAP_HELVETICA_10);
		}
	}
}

// =========================================================
// FIREBALL THEME LOGIC (ARCS, TRAILS, EXPLOSIONS)
// =========================================================
static void spawnExplosion2(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES2; j++) {
			if (!explosionParticles2[j].active) {
				explosionParticles2[j].x = x;
				explosionParticles2[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticles2[j].vx = cos(angle) * speed;
				explosionParticles2[j].vy = sin(angle) * speed;
				explosionParticles2[j].radius = 3.0f + (rand() % 3);
				explosionParticles2[j].life = 20 + (rand() % 10);
				explosionParticles2[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticle2(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES2; i++) {
		if (!trailParticles2[i].active) {
			trailParticles2[i].x = x + (rand() % 6 - 3);
			trailParticles2[i].y = y + (rand() % 6 - 3);
			trailParticles2[i].radius = 4.0f + (rand() % 4);
			trailParticles2[i].alpha = 255;
			trailParticles2[i].active = true;
			break;
		}
	}
}

static void updateFireballs2() {
	for (int i = 0; i < MAX_FIREBALLS2; i++) {
		if (!fireballs2[i].active) continue;

		int oldY = fireballs2[i].y;

		// Apply gravity for an arc effect
		fireballs2[i].vy -= 0.2f;

		fireballs2[i].x += fireballs2[i].vx;
		fireballs2[i].y += fireballs2[i].vy;
		fireballs2[i].rotation += fireballs2[i].vx * 6.0f; // tumble as it flies

		// Bounce off the ground
		if (fireballs2[i].y <= LEVEL_BOTTOM_2) {
			fireballs2[i].y = LEVEL_BOTTOM_2;
			fireballs2[i].vy = -fireballs2[i].vy * 0.5f; // Lose energy on bounce
			fireballs2[i].vx *= 0.8f;
			spawnExplosion2(fireballs2[i].x + 10, fireballs2[i].y + 10);
			if (abs(fireballs2[i].vy) < 1.5f) {
				fireballs2[i].active = false;
			}
		}

		// Explode against a platform ledge instead of flying straight through it
		if (fireballs2[i].active && fireballs2[i].vy < 0) {
			for (int p = 0; p < PLATFORM_COUNT_2; p++) {
				Platform plat = level2_platforms[p];

				bool withinX = (fireballs2[i].x + fireballs2[i].width > plat.x1) && (fireballs2[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballs2[i].y <= plat.y2) {
					fireballs2[i].y = plat.y2;
					spawnExplosion2(fireballs2[i].x + 10, fireballs2[i].y + 10);
					fireballs2[i].active = false;
					break;
				}
			}
		}

		if (fireballs2[i].x < -50 || fireballs2[i].x > 750) {
			fireballs2[i].active = false;
		}

		// Spawn trail particles
		if (fireballs2[i].active)
			spawnTrailParticle2(fireballs2[i].x + 10, fireballs2[i].y + 10);
	}
}

static void updateTrailParticles2() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES2; i++) {
		if (!trailParticles2[i].active) continue;
		trailParticles2[i].alpha -= 15;
		trailParticles2[i].radius -= 0.2f;
		if (trailParticles2[i].alpha <= 0 || trailParticles2[i].radius <= 0) {
			trailParticles2[i].active = false;
		}
	}
}

static void updateExplosionParticles2() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES2; i++) {
		if (!explosionParticles2[i].active) continue;
		explosionParticles2[i].x += explosionParticles2[i].vx;
		explosionParticles2[i].y += explosionParticles2[i].vy;
		explosionParticles2[i].vy -= 0.1f;
		explosionParticles2[i].life--;
		explosionParticles2[i].radius -= 0.15f;
		if (explosionParticles2[i].life <= 0 || explosionParticles2[i].radius <= 0) {
			explosionParticles2[i].active = false;
		}
	}
}

static void checkFireballCollision2() {
	if (hitCooldown2 > 0) return;

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS2; i++) {
		if (!fireballs2[i].active) continue;

		bool collideX = (px + pw > fireballs2[i].x) && (px < fireballs2[i].x + fireballs2[i].width);
		bool collideY = (py + ph > fireballs2[i].y) && (py < fireballs2[i].y + fireballs2[i].height);

		if (collideX && collideY) {
			playerLives2--;
			if (playerLives2 <= 0) { playerLives2 = 0; isGameOver2 = true; }
			hitCooldown2 = 60;
			spawnExplosion2(fireballs2[i].x + 10, fireballs2[i].y + 10); // Explode on hit
			fireballs2[i].active = false;
			break;
		}
	}
}

static void drawFireballs2() {
	// 1. Draw Trail
	for (int i = 0; i < MAX_TRAIL_PARTICLES2; i++) {
		if (!trailParticles2[i].active) continue;
		int alpha = trailParticles2[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticles2[i].x, trailParticles2[i].y, trailParticles2[i].radius, 20);
	}

	// 2. Draw Explosions
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES2; i++) {
		if (!explosionParticles2[i].active) continue;
		if (explosionParticles2[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticles2[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticles2[i].x, explosionParticles2[i].y, explosionParticles2[i].radius, 20);
	}

	// 3. Draw Main Fireballs
	for (int i = 0; i < MAX_FIREBALLS2; i++) {
		if (!fireballs2[i].active) continue;
		float cx = fireballs2[i].x + 10;
		float cy = fireballs2[i].y + 10;

		if (fireballTexture2 > 0) {
			iRotate(cx, cy, fireballs2[i].rotation);
			iShowImage(fireballs2[i].x, fireballs2[i].y, fireballs2[i].width, fireballs2[i].height, fireballTexture2);
			iUnRotate();
		}
		else {
			// Layered glowing orb fallback
			iSetColor(200, 50, 0); iFilledCircle(cx, cy, 14, 50);
			iSetColor(255, 120, 0); iFilledCircle(cx, cy, 10, 50);
			iSetColor(255, 255, 150); iFilledCircle(cx, cy, 5, 50);
		}
	}
}

// =========================================================
// COLLISION & HUD
// =========================================================
static void checkDragonPlayerCollision2() {
	if (hitCooldown2 > 0) { hitCooldown2--; return; }
	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);
	for (int i = 0; i < DRAGON_COUNT_2; i++) {
		int hx = dragons2[i].x, hy = dragons2[i].y, hw = dragons2[i].width, hh = dragons2[i].height;
		if (dragons2[i].animFrame >= 1 && dragons2[i].animFrame <= 17) {
			hx = dragons2[i].x + (int)(dragons2[i].width * 0.556f);
			hy = dragons2[i].y + (int)(dragons2[i].height * 0.312f);
			hw = (int)(dragons2[i].width * 0.363f); hh = (int)(dragons2[i].height * 0.347f);
		}
		else if (dragons2[i].animFrame >= DRAGON_RIGHT_START_2 && dragons2[i].animFrame <= DRAGON_RIGHT_END_2) {
			hx = dragons2[i].x + (int)(dragons2[i].width * 0.081f);
			hy = dragons2[i].y + (int)(dragons2[i].height * 0.312f);
			hw = (int)(dragons2[i].width * 0.363f); hh = (int)(dragons2[i].height * 0.347f);
		}
		bool collideX = (px + pw > hx) && (px < hx + hw);
		bool collideY = (py + ph > hy) && (py < hy + hh);
		if (collideX && collideY) {
			playerLives2--;
			if (playerLives2 <= 0) { playerLives2 = 0; isGameOver2 = true; }
			hitCooldown2 = 30; break;
		}
	}
}

static void drawHUD2() { drawHUDGeneric(zedsLabelTexture2, zedsIconTexture2, playerLives2, energyFrame2, energyTextures2); }

// =========================================================
// INIT & UPDATE LEVEL 2
// =========================================================
static void initLevel2(bool freshStart = true) {
	initPlayer(40, LEVEL_BOTTOM_2 - 10);
	initDragon2();

	loadFireballTexture2();
	for (int i = 0; i < MAX_FIREBALLS2; i++) fireballs2[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES2; i++) trailParticles2[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES2; i++) explosionParticles2[i].active = false;

	initGoldAndBlueBalls2(freshStart);
	loadPauseTextures2(); loadZedsTextures2(); loadEnergyTextures2(); loadTransitionTextures2();

	level2Complete = false; isGameOver2 = false; isPaused2 = false; hitCooldown2 = 0;
	isLevel2Transition = true; level2TransitionCounter = 0; showLevel2ExitTransition = false;
	if (level3TransitionTimerId2 >= 0) iPauseTimer(level3TransitionTimerId2);
	playerLives2 = 5;
	energyFrame2 = 1;
	distanceMoved2 = 0;
}

static void drawLevel2Background() { iShowBMP(0, 0, IMG_LV2_BG); }

static void resolvePlatformCollision2() {
	int oldY = player.y - player.velocityY;
	int oldTopY = oldY + player.height;
	int currentTopY = player.y + player.height;
	if (player.y <= (LEVEL_BOTTOM_2 - 10)) {
		player.y = LEVEL_BOTTOM_2 - 10; player.velocityY = 0; player.onGround = true; player.jumping = false; return;
	}
	player.onGround = false;
	for (int i = 0; i < PLATFORM_COUNT_2; i++) {
		Platform p = level2_platforms[i];
		bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
		if (!withinX) continue;
		if (player.velocityY <= 0) {
			int platformTop = p.y2;
			if (oldY >= platformTop && player.y <= platformTop) {
				player.y = platformTop; player.velocityY = 0; player.onGround = true; player.jumping = false; break;
			}
		}
		else if (player.velocityY > 0) {
			int platformBottom = p.y1;
			if (oldTopY <= platformBottom && currentTopY >= platformBottom) {
				player.y = platformBottom - player.height; player.velocityY = 0; break;
			}
		}
	}
}

static void resolveWallCollision2() {
	for (int i = 0; i < WALL_COUNT_2; i++) {
		Platform w = level2_walls[i];
		bool withinY = (player.y + player.height > w.y1) && (player.y < w.y2);
		if (!withinY) continue;
		bool collideX = (player.x + player.width > w.x1) && (player.x < w.x2);
		if (!collideX) continue;
		int playerCenter = player.x + player.width / 2;
		int wallCenter = (w.x1 + w.x2) / 2;
		if (playerCenter < wallCenter) player.x = w.x1 - player.width;
		else player.x = w.x2;
	}
}

static void updateLevel2() {
	if (isLevel2Transition) {
		level2TransitionCounter++;
		if (level2TransitionCounter >= 100) isLevel2Transition = false;
		return;
	}
	if (isGameOver2 || isPaused2 || showLevel2ExitTransition) return;

	int oldX = player.x;
	player.isMoving = false;
	if (isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT)) { player.x -= MOVE_SPEED; player.facingRight = false; player.isMoving = true; }
	if (isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT)) { player.x += MOVE_SPEED; player.facingRight = true; player.isMoving = true; }

	if (player.x < LEVEL_LEFT_2) player.x = LEVEL_LEFT_2;
	if (player.x + player.width > LEVEL_RIGHT_2) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level2Complete && atPortalCoordinates)) player.x = LEVEL_RIGHT_2 - player.width;
	}
	resolveWallCollision2();

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved2 += moveDist;
		while (distanceMoved2 >= 150) {
			distanceMoved2 -= 150;
			if (energyFrame2 < 145) energyFrame2++;
		}
		if (energyFrame2 >= 145) isGameOver2 = true;
	}

	if ((isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP)) && player.onGround) {
		player.velocityY = JUMP_FORCE; player.onGround = false; player.jumping = true; requestJumpSfx();
	}
	player.velocityY -= GRAVITY;
	if (player.velocityY < -MAX_FALL_SPEED) player.velocityY = -MAX_FALL_SPEED;
	player.y += player.velocityY;
	resolvePlatformCollision2();
	if (player.y + player.height > LEVEL_TOP_2) { player.y = LEVEL_TOP_2 - player.height; player.velocityY = 0; }

	updatePlayerAnimation();
	updateDragon2();

	// --- FIREBALL & PARTICLE UPDATES ---
	updateFireballs2();
	updateTrailParticles2();
	updateExplosionParticles2();
	checkFireballCollision2();

	checkDragonPlayerCollision2();
	checkGoldBallCollision2();
	checkBlueBallCollision2();
	checkPortalCollision2();
}

static void drawPauseMenu2() { drawPauseMenuGeneric(resumeTexture2, pauseResumeBtn2, restartTexture2, pauseRestartBtn2, exitTexture2, pauseExitBtn2); }

static void drawLevel2() {
	drawLevel2Background();
	if (isLevel2Transition) {
		if (level1CompletedTexture2 > 0) iShowImage(175, 260, 350, 60, level1CompletedTexture2);
		if (level2StartsTexture2 > 0) iShowImage(175, 180, 350, 60, level2StartsTexture2);
		return;
	}

	drawDragon2();
	drawFireballs2(); // Draws trails, explosions, and fireballs
	drawGoldBalls2();
	drawBlueBalls2();
	drawPortal2();
	drawPlayer();

	drawHUD2();
	drawGoldBallHUD2();
	drawPointsHUD2();

	if (isPaused2) {
		drawPauseMenu2();
	}
	else if (showLevel2ExitTransition) {
		if (level2CompletedTexture2 > 0) iShowImage(175, 260, 350, 60, level2CompletedTexture2);
		else { iSetColor(255, 255, 255); iText(265, 305, "Level 2 Completed", GLUT_BITMAP_HELVETICA_18); }
		if (level3StartsTexture2 > 0) iShowImage(175, 180, 350, 60, level3StartsTexture2);
		else { iSetColor(255, 255, 255); iText(290, 195, "Level 3 Starts", GLUT_BITMAP_HELVETICA_18); }
	}
	else if (isGameOver2) {
		drawTotalPointsBoxGeneric(totalPointsTexture2, playerPoints2, 405);
		if (gameOverTexture2 > 0) iShowImage(175, 175, 350, 220, gameOverTexture2);
		else { iSetColor(255, 0, 0); iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24); }
		if (gameOverRestartTexture2 > 0) iShowImage(gameOverRestartBtn2.x1, gameOverRestartBtn2.y1, 220, 50, gameOverRestartTexture2);
		if (gameOverExitTexture2 > 0) iShowImage(gameOverExitBtn2.x1, gameOverExitBtn2.y1, 150, 35, gameOverExitTexture2);
	}
}

#endif