#ifndef LEVEL3_H
#define LEVEL3_H

#define IMG_LV3_BG "Images/lv3_bg.bmp"

#include <math.h> 
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

#define PLATFORM_COUNT3 11
static Platform level3_platforms[PLATFORM_COUNT3] = {
	{ 106, 129, 129, 129 }, // Step 1
	{ 129, 152, 152, 152 }, // Step 2
	{ 152, 175, 175, 175 }, // Step 3
	{ 175, 198, 198, 198 }, // Step 4
	{ 198, 221, 221, 221 }, // Step 5
	{ 221, 244, 244, 244 }, // Step 6
	{ 244, 267, 267, 267 }, // Step 7
	{ 267, 290, 290, 290 }, // Step 8
	{ 290, 313, 313, 313 }, // Step 9 (Top of stairs)

	// Upper slabs
	{ 395, 357, 422, 380 }, // Left high block (index 9)
	{ 530, 249, 553, 272 }  // Right high block (index 10)
};

#define LEVEL_LEFT   20
#define LEVEL_RIGHT  680
#define LEVEL_BOTTOM 20
#define LEVEL3_TOP    480

// ===== Dragon Frame Boundaries =====
#define DRAGON_LEFT_START    1
#define DRAGON_LEFT_END      17
#define DRAGON_RIGHT_START   31
#define DRAGON_RIGHT_END     63
#define TOTAL_DRAGON_FRAMES 65

#define DRAGON_SPEED         2
#define DRAGON_ANIM_SPEED    3 
#define DRAGON_COUNT         3

// ===== Fireball & Particle Variables =====
#define MAX_FIREBALLS3 10
#define MAX_TRAIL_PARTICLES3 60
#define MAX_EXPLOSION_PARTICLES3 40

struct Fireball3 {
	int x, y;
	int width, height;
	float vx, vy;
	float rotation;
	bool active;
};

struct TrailParticle3 {
	float x, y;
	float radius;
	int alpha;
	bool active;
};

struct ExplosionParticle3 {
	float x, y;
	float vx, vy;
	float radius;
	int life;
	bool active;
};

static Fireball3 fireballs3[MAX_FIREBALLS3];
static TrailParticle3 trailParticles3[MAX_TRAIL_PARTICLES3];
static ExplosionParticle3 explosionParticles3[MAX_EXPLOSION_PARTICLES3];
static int fireballTexture3 = 0;

// ===== Game State Variables =====
static bool level3Complete = false;
static bool isGameOver3 = false;
static bool isPaused3 = false;
static bool enterLevel4 = false;
static int hitCooldown3 = 0;

// ----- Exit transition (level3 completed -> level4 starts) -----
static bool showLevel3ExitTransition = false;
static int level4TransitionTimerId3 = -1;

// ----- Intro transition (level2 completed -> level3 starts) -----
static bool isLevel3Transition = true;
static int level3TransitionCounter = 0;
static int level2CompletedTexture3 = 0;
static int level3StartsTexture3 = 0;

// ===== Player Lives, Energy & Points Variables =====
static int playerLives3 = 5;
static int& energyFrame3 = gEnergyFrame;
static int distanceMoved3 = 0;
static int energyTextures3[150] = { 0 };
static int& playerPoints3 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTexture3 = 0;
static int zedsIconTexture3 = 0;

// ===== Menu Textures & Bounding Boxes =====
static int resumeTexture3 = 0;
static int restartTexture3 = 0;
static int exitTexture3 = 0;

static Button pauseResumeBtn3 = { 260, 280, 440, 325 };
static Button pauseRestartBtn3 = { 260, 220, 440, 265 };
static Button pauseExitBtn3 = { 260, 160, 440, 205 };

// ===== Game Over Menu Textures & Bounding Boxes =====
static int totalPointsTexture3 = 0;
static int gameOverTexture3 = 0;
static int gameOverRestartTexture3 = 0;
static int gameOverExitTexture3 = 0;

static Button gameOverRestartBtn3 = { 240, 115, 460, 165 };
static Button gameOverExitBtn3 = { 275, 70, 425, 105 };

// ===== Gold Ball Collectibles, Portal & HUD =====
struct GoldBall3 {
	int x, y;
	int width, height;
	bool collected;
};

static GoldBall3 goldBalls3[3];
static int goldBallTexture3 = 0;
static int goldBallIconTextures3[5] = { 0 };
static int goldBallsCollected3 = 0;
static int dispearTexture3 = 0;

// ===== Blue Ball Collectibles & Points HUD =====
struct BlueBall3 {
	int x, y;
	int width, height;
	bool collected;
};

static BlueBall3 blueBalls3[9];
static int blueBallTexture3 = 0;
static int pointsTexture3 = 0;

struct PatrolDragon3 {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
	int minX, maxX;
	int shootTimer;
	int shootCooldown;
};

static PatrolDragon3 dragons3[DRAGON_COUNT];

// Preloaded texture array
static int dragonTextures3[TOTAL_DRAGON_FRAMES];
static bool texturesLoaded3 = false;

static void loadDragonTextures3() {
	if (texturesLoaded3) return;

	char path[128];
	for (int i = 1; i <= DRAGON_RIGHT_END; i++) {
		if (i >= 18 && i <= 30) {
			dragonTextures3[i] = 0;
			continue;
		}

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTextures3[i] = iLoadImage(path);

		if (dragonTextures3[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTextures3[i] = iLoadImage(path);
		}
	}

	texturesLoaded3 = true;
}

static void loadFireballTexture3() {
	static bool loaded = false;
	if (loaded) return;
	fireballTexture3 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadTransitionTextures3() {
	static bool loaded = false;
	if (loaded) return;

	level2CompletedTexture3 = loadTex("Images/level2completed.png", "../Images/level2completed.png");
	level3StartsTexture3 = loadTex("Images/level3starts.png", "../Images/level3starts.png");

	loaded = true;
}

static void loadZedsTextures3() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTexture3 = loadTex("Images/zeds.png", "../Images/zeds.png");

	zedsIconTexture3 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture3 <= 0) zedsIconTexture3 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture3 <= 0) zedsIconTexture3 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTextures3() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures3[i] = iLoadImage(path);

		if (energyTextures3[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures3[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadPauseTextures3() {
	static bool loaded = false;
	if (loaded) return;

	resumeTexture3 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture3 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture3 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTexture3 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTexture3 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture3 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture3 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadGoldBallTextures3() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTexture3 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture3 = loadTex("Images/dispear3.png", "../Images/dispear3.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures3[i] = iLoadImage(path);
		if (goldBallIconTextures3[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures3[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadBlueBallAndPointsTextures3() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTexture3 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture3 = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

static void initGoldAndBlueBalls3(bool freshStart) {
	loadGoldBallTextures3();
	loadBlueBallAndPointsTextures3();
	goldBallsCollected3 = 0;
	if (freshStart) playerPoints3 = 0;
	enterLevel4 = false;

	Platform p9 = level3_platforms[9];
	goldBalls3[0].width = 25;
	goldBalls3[0].height = 25;
	goldBalls3[0].x = p9.x1 + 2;
	goldBalls3[0].y = p9.y2 + 2;
	goldBalls3[0].collected = false;

	Platform p10 = level3_platforms[10];
	goldBalls3[1].width = 25;
	goldBalls3[1].height = 25;
	goldBalls3[1].x = p10.x1 + 2;
	goldBalls3[1].y = p10.y2 + 2;
	goldBalls3[1].collected = false;

	goldBalls3[2].width = 25;
	goldBalls3[2].height = 25;
	goldBalls3[2].x = 520;
	goldBalls3[2].y = 25;
	goldBalls3[2].collected = false;

	int stairIndices[3] = { 1, 4, 7 };
	for (int i = 0; i < 3; i++) {
		Platform p = level3_platforms[stairIndices[i]];
		blueBalls3[i].width = 18;
		blueBalls3[i].height = 18;
		blueBalls3[i].x = p.x1 + 2;
		blueBalls3[i].y = p.y2 + 2;
		blueBalls3[i].collected = false;
	}

	struct { int x, y; } scattered[6] = {
		{ 350, 40 },
		{ 600, 40 },
		{ 470, 90 },
		{ 440, 410 },
		{ 560, 300 },
		{ 300, 350 }
	};

	for (int i = 0; i < 6; i++) {
		blueBalls3[3 + i].width = 18;
		blueBalls3[3 + i].height = 18;
		blueBalls3[3 + i].x = scattered[i].x;
		blueBalls3[3 + i].y = scattered[i].y;
		blueBalls3[3 + i].collected = false;
	}
}

static void checkGoldBallCollision3() {
	checkGoldBallCollisionGeneric(goldBalls3, &goldBallsCollected3, &level3Complete);
}

static void checkBlueBallCollision3() {
	checkBlueBallCollisionGeneric(blueBalls3, &playerPoints3);
}

static void drawGoldBalls3() {
	drawGoldBallsGeneric(goldBalls3, goldBallTexture3);
}

static void drawBlueBalls3() {
	drawBlueBallsGeneric(blueBalls3, blueBallTexture3);
}

static void drawPortal3() {
	if (level3Complete) {
		if (dispearTexture3 > 0) {
			iShowImage(680, 20, 35, 100, dispearTexture3);
		}
		else {
			iSetColor(255, 255, 0);
			iRectangle(645, 20, 35, 100);
		}
	}
}

static void triggerEnterLevel4() {
	enterLevel4 = true;
	showLevel3ExitTransition = false;

	if (level4TransitionTimerId3 >= 0) {
		iPauseTimer(level4TransitionTimerId3);
	}
}

static void checkPortalCollision3() {
	if (level3Complete && !showLevel3ExitTransition) {
		bool collideX = (player.x + player.width >= 700);
		bool collideY = (player.y <= 120 && player.y + player.height >= 20);

		if (collideX && collideY) {
			showLevel3ExitTransition = true;

			if (level4TransitionTimerId3 < 0) {
				level4TransitionTimerId3 = iSetTimer(1000, triggerEnterLevel4);
			}
			else {
				iResumeTimer(level4TransitionTimerId3);
			}
		}
	}
}

static void drawGoldBallHUD3() {
	drawGoldBallHUDGeneric(goldBallsCollected3, goldBallIconTextures3);
}

static void drawPointsHUD3() {
	drawPointsHUDGeneric(pointsTexture3, playerPoints3);
}

static void stepNextFrame3(int i) {
	int safetyCounter = 0;
	do {
		dragons3[i].animFrame++;

		if (dragons3[i].movingRight) {
			if (dragons3[i].animFrame > DRAGON_RIGHT_END || dragons3[i].animFrame < DRAGON_RIGHT_START)
				dragons3[i].animFrame = DRAGON_RIGHT_START;
		}
		else {
			if (dragons3[i].animFrame > DRAGON_LEFT_END || dragons3[i].animFrame < DRAGON_LEFT_START)
				dragons3[i].animFrame = DRAGON_LEFT_START;
		}

		safetyCounter++;
		if (safetyCounter > TOTAL_DRAGON_FRAMES) break;
	} while (dragonTextures3[dragons3[i].animFrame] <= 0);
}

static void initDragon3() {
	loadDragonTextures3();

	dragons3[0].width = 60; dragons3[0].height = 60;
	dragons3[0].x = 480; dragons3[0].y = 390;
	dragons3[0].minX = 450; dragons3[0].maxX = 650;
	dragons3[0].movingRight = true;
	dragons3[0].animFrame = DRAGON_RIGHT_START;
	dragons3[0].animTimer = 0;
	dragons3[0].shootTimer = 90 + (rand() % 60);
	dragons3[0].shootCooldown = 150;
	if (dragonTextures3[dragons3[0].animFrame] <= 0) stepNextFrame3(0);

	dragons3[1].width = 60; dragons3[1].height = 60;
	dragons3[1].x = 600; dragons3[1].y = 250;
	dragons3[1].minX = 450; dragons3[1].maxX = 650;
	dragons3[1].movingRight = false;
	dragons3[1].animFrame = DRAGON_LEFT_START;
	dragons3[1].animTimer = 0;
	dragons3[1].shootTimer = 120 + (rand() % 60);
	dragons3[1].shootCooldown = 180;
	if (dragonTextures3[dragons3[1].animFrame] <= 0) stepNextFrame3(1);

	dragons3[2].width = 60; dragons3[2].height = 60;
	dragons3[2].x = 110; dragons3[2].y = 5;
	dragons3[2].minX = 100; dragons3[2].maxX = 630;
	dragons3[2].movingRight = true;
	dragons3[2].animFrame = DRAGON_RIGHT_START;
	dragons3[2].animTimer = 0;
	dragons3[2].shootTimer = 100 + (rand() % 50);
	dragons3[2].shootCooldown = 160;
	if (dragonTextures3[dragons3[2].animFrame] <= 0) stepNextFrame3(2);
}

static void updateDragon3() {
	for (int i = 0; i < DRAGON_COUNT; i++) {
		if (dragons3[i].movingRight) {
			dragons3[i].x += DRAGON_SPEED;
			if (dragons3[i].x >= dragons3[i].maxX) {
				dragons3[i].x = dragons3[i].maxX;
				dragons3[i].movingRight = false;
				dragons3[i].animFrame = DRAGON_LEFT_START;
				if (dragonTextures3[dragons3[i].animFrame] <= 0) stepNextFrame3(i);
			}
		}
		else {
			dragons3[i].x -= DRAGON_SPEED;
			if (dragons3[i].x <= dragons3[i].minX) {
				dragons3[i].x = dragons3[i].minX;
				dragons3[i].movingRight = true;
				dragons3[i].animFrame = DRAGON_RIGHT_START;
				if (dragonTextures3[dragons3[i].animFrame] <= 0) stepNextFrame3(i);
			}
		}

		dragons3[i].animTimer++;
		if (dragons3[i].animTimer >= DRAGON_ANIM_SPEED) {
			dragons3[i].animTimer = 0;
			stepNextFrame3(i);
		}

		// --- FIREBALL SHOOTING LOGIC ---
		dragons3[i].shootTimer--;
		if (dragons3[i].shootTimer <= 0) {
			for (int j = 0; j < MAX_FIREBALLS3; j++) {
				if (!fireballs3[j].active) {
					fireballs3[j].x = dragons3[i].x + dragons3[i].width / 2 - 10;
					fireballs3[j].y = dragons3[i].y + dragons3[i].height / 2 - 10;
					fireballs3[j].width = 20; fireballs3[j].height = 20;

					float dx = (player.x + player.width / 2.0f) - (fireballs3[j].x + 10);
					float dy = (player.y + player.height / 2.0f) - (fireballs3[j].y + 10);
					float dist = sqrt(dx * dx + dy * dy);
					if (dist == 0) dist = 1;

					float speed = 7.0f;
					fireballs3[j].vx = (dx / dist) * speed;
					// Arc boost for parabola
					fireballs3[j].vy = (dy / dist) * speed + 4.5f;
					fireballs3[j].rotation = 0.0f;
					fireballs3[j].active = true;
					break;
				}
			}
			dragons3[i].shootTimer = dragons3[i].shootCooldown;
		}
	}
}

static void drawDragon3() {
	for (int i = 0; i < DRAGON_COUNT; i++) {
		int textureId = dragonTextures3[dragons3[i].animFrame];

		if (textureId > 0) {
			iShowImage(dragons3[i].x, dragons3[i].y, dragons3[i].width, dragons3[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragons3[i].x, dragons3[i].y, dragons3[i].width, dragons3[i].height);
			iSetColor(255, 255, 255);
			iText(dragons3[i].x + 5, dragons3[i].y + 25, "DRAGON", GLUT_BITMAP_HELVETICA_10);
		}
	}
}

// =========================================================
// FIREBALL THEME LOGIC (ARCS, TRAILS, EXPLOSIONS)
// =========================================================
static void spawnExplosion3(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES3; j++) {
			if (!explosionParticles3[j].active) {
				explosionParticles3[j].x = x;
				explosionParticles3[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticles3[j].vx = cos(angle) * speed;
				explosionParticles3[j].vy = sin(angle) * speed;
				explosionParticles3[j].radius = 3.0f + (rand() % 3);
				explosionParticles3[j].life = 20 + (rand() % 10);
				explosionParticles3[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticle3(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES3; i++) {
		if (!trailParticles3[i].active) {
			trailParticles3[i].x = x + (rand() % 6 - 3);
			trailParticles3[i].y = y + (rand() % 6 - 3);
			trailParticles3[i].radius = 4.0f + (rand() % 4);
			trailParticles3[i].alpha = 255;
			trailParticles3[i].active = true;
			break;
		}
	}
}

static void updateFireballs3() {
	for (int i = 0; i < MAX_FIREBALLS3; i++) {
		if (!fireballs3[i].active) continue;

		int oldY = fireballs3[i].y;

		// Apply gravity
		fireballs3[i].vy -= 0.2f;

		fireballs3[i].x += fireballs3[i].vx;
		fireballs3[i].y += fireballs3[i].vy;
		fireballs3[i].rotation += fireballs3[i].vx * 6.0f;

		// Bounce off the ground
		if (fireballs3[i].y <= LEVEL_BOTTOM) {
			fireballs3[i].y = LEVEL_BOTTOM;
			fireballs3[i].vy = -fireballs3[i].vy * 0.5f;
			fireballs3[i].vx *= 0.8f;
			spawnExplosion3(fireballs3[i].x + 10, fireballs3[i].y + 10);
			if (abs(fireballs3[i].vy) < 1.5f) {
				fireballs3[i].active = false;
			}
		}

		// Explode against platforms
		if (fireballs3[i].active && fireballs3[i].vy < 0) {
			for (int p = 0; p < PLATFORM_COUNT3; p++) {
				Platform plat = level3_platforms[p];

				bool withinX = (fireballs3[i].x + fireballs3[i].width > plat.x1) && (fireballs3[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballs3[i].y <= plat.y2) {
					fireballs3[i].y = plat.y2;
					spawnExplosion3(fireballs3[i].x + 10, fireballs3[i].y + 10);
					fireballs3[i].active = false;
					break;
				}
			}
		}

		if (fireballs3[i].x < -50 || fireballs3[i].x > 750) {
			fireballs3[i].active = false;
		}

		// Spawn trail particles
		if (fireballs3[i].active)
			spawnTrailParticle3(fireballs3[i].x + 10, fireballs3[i].y + 10);
	}
}

static void updateTrailParticles3() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES3; i++) {
		if (!trailParticles3[i].active) continue;
		trailParticles3[i].alpha -= 15;
		trailParticles3[i].radius -= 0.2f;
		if (trailParticles3[i].alpha <= 0 || trailParticles3[i].radius <= 0) {
			trailParticles3[i].active = false;
		}
	}
}

static void updateExplosionParticles3() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES3; i++) {
		if (!explosionParticles3[i].active) continue;
		explosionParticles3[i].x += explosionParticles3[i].vx;
		explosionParticles3[i].y += explosionParticles3[i].vy;
		explosionParticles3[i].vy -= 0.1f;
		explosionParticles3[i].life--;
		explosionParticles3[i].radius -= 0.15f;
		if (explosionParticles3[i].life <= 0 || explosionParticles3[i].radius <= 0) {
			explosionParticles3[i].active = false;
		}
	}
}

static void checkFireballCollision3() {
	if (hitCooldown3 > 0) return;

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS3; i++) {
		if (!fireballs3[i].active) continue;

		bool collideX = (px + pw > fireballs3[i].x) && (px < fireballs3[i].x + fireballs3[i].width);
		bool collideY = (py + ph > fireballs3[i].y) && (py < fireballs3[i].y + fireballs3[i].height);

		if (collideX && collideY) {
			playerLives3--;
			if (playerLives3 <= 0) { playerLives3 = 0; isGameOver3 = true; }
			hitCooldown3 = 60;
			spawnExplosion3(fireballs3[i].x + 10, fireballs3[i].y + 10);
			fireballs3[i].active = false;
			break;
		}
	}
}

static void drawFireballs3() {
	// 1. Draw Trail
	for (int i = 0; i < MAX_TRAIL_PARTICLES3; i++) {
		if (!trailParticles3[i].active) continue;
		int alpha = trailParticles3[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticles3[i].x, trailParticles3[i].y, trailParticles3[i].radius, 20);
	}

	// 2. Draw Explosions
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES3; i++) {
		if (!explosionParticles3[i].active) continue;
		if (explosionParticles3[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticles3[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticles3[i].x, explosionParticles3[i].y, explosionParticles3[i].radius, 20);
	}

	// 3. Draw Main Fireballs
	for (int i = 0; i < MAX_FIREBALLS3; i++) {
		if (!fireballs3[i].active) continue;
		float cx = fireballs3[i].x + 10;
		float cy = fireballs3[i].y + 10;

		if (fireballTexture3 > 0) {
			iRotate(cx, cy, fireballs3[i].rotation);
			iShowImage(fireballs3[i].x, fireballs3[i].y, fireballs3[i].width, fireballs3[i].height, fireballTexture3);
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

static void checkDragonPlayerCollision3() {
	if (hitCooldown3 > 0) {
		hitCooldown3--;
		return;
	}

	for (int i = 0; i < DRAGON_COUNT; i++) {
		int hx = dragons3[i].x;
		int hy = dragons3[i].y;
		int hw = dragons3[i].width;
		int hh = dragons3[i].height;

		if (dragons3[i].animFrame >= 1 && dragons3[i].animFrame <= 17) {
			hx = dragons3[i].x + (int)(dragons3[i].width * 0.556f);
			hy = dragons3[i].y + (int)(dragons3[i].height * 0.312f);
			hw = (int)(dragons3[i].width * 0.363f);
			hh = (int)(dragons3[i].height * 0.347f);
		}
		else if (dragons3[i].animFrame >= DRAGON_RIGHT_START && dragons3[i].animFrame <= DRAGON_RIGHT_END) {
			hx = dragons3[i].x + (int)(dragons3[i].width * 0.081f);
			hy = dragons3[i].y + (int)(dragons3[i].height * 0.312f);
			hw = (int)(dragons3[i].width * 0.363f);
			hh = (int)(dragons3[i].height * 0.347f);
		}

		bool collideX = (player.x + player.width > hx) && (player.x < hx + hw);
		bool collideY = (player.y + player.height > hy) && (player.y < hy + hh);

		if (collideX && collideY) {
			playerLives3--;
			if (playerLives3 <= 0) {
				playerLives3 = 0;
				isGameOver3 = true;
			}
			hitCooldown3 = 30;
			break;
		}
	}
}

static void drawHUD3() {
	drawHUDGeneric(zedsLabelTexture3, zedsIconTexture3, playerLives3, energyFrame3, energyTextures3);
}

static void initLevel3(bool freshStart = true) {
	initPlayer(40, LEVEL_BOTTOM - 10);
	initDragon3();

	loadFireballTexture3();
	for (int i = 0; i < MAX_FIREBALLS3; i++) fireballs3[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES3; i++) trailParticles3[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES3; i++) explosionParticles3[i].active = false;

	initGoldAndBlueBalls3(freshStart);
	loadPauseTextures3();
	loadZedsTextures3();
	loadEnergyTextures3();
	loadTransitionTextures3();

	level3Complete = false;
	isGameOver3 = false;
	isPaused3 = false;
	hitCooldown3 = 0;

	showLevel3ExitTransition = false;
	if (level4TransitionTimerId3 >= 0) {
		iPauseTimer(level4TransitionTimerId3);
	}

	isLevel3Transition = true;
	level3TransitionCounter = 0;

	// Zeds + energy start full every time Level 3 begins (and refill again at the
	// start of each bonus stage, sub 1 / sub 2).
	playerLives3 = 5;
	energyFrame3 = 1;
	distanceMoved3 = 0;
}

static void drawLevel3Background() {
	iShowBMP(0, 0, IMG_LV3_BG);
}

// ===========================================================================
// SOLID BLOCKS - stairs and slabs can't be walked through from any side.
// ===========================================================================
// Standing height fine-tune (pixels). 0 = the bottom of the player's hitbox
// (getPlayerHitbox) rests exactly on the block top. Raise it if the character
// sinks into the blocks, lower it (negative) if it floats above them.
#define FEET_TWEAK_L3 0

// Small rises the player may simply walk up (like a real stair). Anything taller
// is a wall. 0 = every block is a wall.
#define STEP_UP_L3 24

// The 9 castle steps are treated as one solid block per step (as thick as the
// step rise); the two upper slabs use their own thickness.
#define STAIR_THICK3 23
static void getSolid3(int i, int &x1, int &y1, int &x2, int &y2) {
	Platform p = level3_platforms[i];
	x1 = p.x1; x2 = p.x2; y2 = p.y2;
	y1 = (i < 9) ? (p.y2 - STAIR_THICK3) : p.y1;
}

// HORIZONTAL pass - call right after player.x has been changed, BEFORE gravity
// moves player.y. Blocks are solid on their sides: the player is pushed back so
// the hitbox just touches the block, or (for a tiny rise) walks up onto it.
static void resolveSolidsX3(int oldX) {
	for (int iter = 0; iter < 4; iter++) {
		int px, py, pw, ph;
		getPlayerHitbox(px, py, pw, ph);
		int hx = px - player.x;                       // hitbox left offset
		int feetOff = (py - player.y) + FEET_TWEAK_L3;
		int foot = player.y + feetOff;
		int head = foot + ph;

		bool acted = false;
		for (int i = 0; i < PLATFORM_COUNT3; i++) {
			int x1, y1, x2, y2;
			getSolid3(i, x1, y1, x2, y2);
			if (!((px + pw > x1) && (px < x2) && (foot < y2) && (head > y1))) continue;

			int rise = y2 - foot;
			if (rise > 0 && rise <= STEP_UP_L3) {
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
static void resolvePlatformCollision3() {
	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	int feetOff = (py - player.y) + FEET_TWEAK_L3;   // sprite bottom -> feet
	int oldY = player.y - player.velocityY;
	int footNow = player.y + feetOff;
	int footOld = oldY + feetOff;
	int headNow = footNow + ph;
	int headOld = footOld + ph;

	if (player.y <= (LEVEL_BOTTOM - 10)) {
		player.y = LEVEL_BOTTOM - 10;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	player.onGround = false;

	if (player.velocityY <= 0) {
		int bestTop = -100000;
		for (int i = 0; i < PLATFORM_COUNT3; i++) {
			int x1, y1, x2, y2;
			getSolid3(i, x1, y1, x2, y2);
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
		for (int i = 0; i < PLATFORM_COUNT3; i++) {
			int x1, y1, x2, y2;
			getSolid3(i, x1, y1, x2, y2);
			if (!((px + pw > x1) && (px < x2))) continue;
			if (headOld <= y1 && headNow >= y1 && y1 < bestBottom) bestBottom = y1;
		}
		if (bestBottom < 100000) {
			player.y = bestBottom - feetOff - ph;
			player.velocityY = 0;
		}
	}
}

static void updateLevel3() {
	if (isLevel3Transition) {
		level3TransitionCounter++;
		if (level3TransitionCounter >= 100) {
			isLevel3Transition = false;
		}
		return;
	}

	if (isGameOver3 || isPaused3 || showLevel3ExitTransition) return;

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

	if (player.x < LEVEL_LEFT) player.x = LEVEL_LEFT;

	if (player.x + player.width > LEVEL_RIGHT) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level3Complete && atPortalCoordinates)) {
			player.x = LEVEL_RIGHT - player.width;
		}
	}

	// Stairs and slabs are solid on their sides too.
	resolveSolidsX3(oldX);

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved3 += moveDist;
		while (distanceMoved3 >= 150) {
			distanceMoved3 -= 150;
			if (energyFrame3 < 145) {
				energyFrame3++;
			}
		}
		if (energyFrame3 >= 145) {
			isGameOver3 = true;
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

	resolvePlatformCollision3();

	if (player.y + player.height > LEVEL3_TOP) {
		player.y = LEVEL3_TOP - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateDragon3();

	// --- FIREBALL & PARTICLE UPDATES ---
	updateFireballs3();
	updateTrailParticles3();
	updateExplosionParticles3();
	checkFireballCollision3();

	checkDragonPlayerCollision3();
	checkGoldBallCollision3();
	checkBlueBallCollision3();
	checkPortalCollision3();
}

static void drawPauseMenu3() {
	drawPauseMenuGeneric(resumeTexture3, pauseResumeBtn3, restartTexture3, pauseRestartBtn3, exitTexture3, pauseExitBtn3);
}

static void drawLevel3() {
	drawLevel3Background();

	if (isLevel3Transition) {
		if (level2CompletedTexture3 > 0) {
			iShowImage(175, 260, 350, 60, level2CompletedTexture3);
		}
		else {
			iSetColor(255, 255, 255);
			iText(265, 305, "Level 2 Completed", GLUT_BITMAP_HELVETICA_18);
		}

		if (level3StartsTexture3 > 0) {
			iShowImage(175, 180, 350, 60, level3StartsTexture3);
		}
		else {
			iSetColor(255, 255, 255);
			iText(290, 195, "Level 3 Starts", GLUT_BITMAP_HELVETICA_18);
		}
		return;
	}

	drawDragon3();
	drawFireballs3(); // Draws trails, explosions, and fireballs
	drawGoldBalls3();
	drawBlueBalls3();
	drawPortal3();
	drawPlayer();

	drawHUD3();
	drawGoldBallHUD3();
	drawPointsHUD3();

	if (isPaused3) {
		drawPauseMenu3();
	}
	else if (showLevel3ExitTransition) {
		// No banner here: Level 3 continues straight into sub 1. The
		// "Level 3 Completed / Level 4 Starts" banner is shown only at the end of sub 2.
	}
	else if (isGameOver3) {
		drawTotalPointsBoxGeneric(totalPointsTexture3, playerPoints3, 405);

		if (gameOverTexture3 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTexture3);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTexture3 > 0)
			iShowImage(gameOverRestartBtn3.x1, gameOverRestartBtn3.y1, 220, 50, gameOverRestartTexture3);

		if (gameOverExitTexture3 > 0)
			iShowImage(gameOverExitBtn3.x1, gameOverExitBtn3.y1, 150, 35, gameOverExitTexture3);
	}
}

#endif