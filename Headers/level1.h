#ifndef LEVEL1_H
#define LEVEL1_H

#include <math.h> 
#include <stdlib.h> // For rand()
#include <stdio.h>  // For sprintf()
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// ===== Level Boundaries & Constants =====
#define PLATFORM_COUNT 3
static const Platform level1_platforms[PLATFORM_COUNT] = {
	// { x1 (left), y1 (bottom edge), x2 (right), y2 (top surface) }
	{ 20, 95, 440, 125 },  // 1st slab (Bottom-Left)
	{ 240, 210, 660, 248 },  // 2nd slab (Middle-Right)
	{ 20, 328, 440, 366 }   // 3rd slab (Top-Left)
};

#define LEVEL_LEFT   20
#define LEVEL_RIGHT  680
#define LEVEL_BOTTOM 20
#define LEVEL_TOP    500

// ===== Dragon Animation Boundaries =====
#define DRAGON_LEFT_START    1
#define DRAGON_LEFT_END      17
#define DRAGON_RIGHT_START   31
#define DRAGON_RIGHT_END     63
#define TOTAL_DRAGON_FRAMES  65

#define DRAGON_SPEED        2
#define DRAGON_ANIM_SPEED   3 

// Dragon Hitbox Bounding Box Multipliers
#define DRAGON_HITBOX_X_LEFT_OFFSET   0.556f
#define DRAGON_HITBOX_X_RIGHT_OFFSET  0.081f
#define DRAGON_HITBOX_Y_OFFSET        0.312f
#define DRAGON_HITBOX_WIDTH_SCALE     0.363f
#define DRAGON_HITBOX_HEIGHT_SCALE    0.347f

// ===== Game State Variables =====
static bool level1Complete = false;
static bool isGameOver = false;
static bool isPaused = false;
static bool enterLevel2 = false;
static int hitCooldown = 0;

static bool showTransitionScreen = false;
static int level1CompletedTexture = 0;
static int level2StartsTexture = 0;

// ===== Player Persistent References =====
static int playerLives = 5;
static int& energyFrame = gEnergyFrame;
static int distanceMoved = 0;
static int energyTextures[150] = { 0 };
static int& playerPoints = gPlayerPoints;

// ===== HUD & Menu Textures =====
static int zedsLabelTexture = 0;
static int zedsIconTexture = 0;

static int resumeTexture = 0;
static int restartTexture = 0;
static int exitTexture = 0;

static Button pauseResumeBtn = { 260, 280, 440, 325 };
static Button pauseRestartBtn = { 260, 220, 440, 265 };
static Button pauseExitBtn = { 260, 160, 440, 205 };

static int totalPointsTexture = 0;
static int gameOverTexture = 0;
static int gameOverRestartTexture = 0;
static int gameOverExitTexture = 0;

static Button gameOverRestartBtn = { 240, 140, 460, 190 };
static Button gameOverExitBtn = { 275, 95, 425, 130 };

// ===== Collectibles & Entities =====
struct GoldBall {
	int x, y;
	int width, height;
	bool collected;
};

static GoldBall goldBalls[3];
static int goldBallTexture = 0;
static int goldBallIconTextures[5] = { 0 };
static int goldBallsCollected = 0;
static int dispearTexture = 0;

struct BlueBall {
	int x, y;
	int width, height;
	bool collected;
};

static BlueBall blueBalls[9];
static int blueBallTexture = 0;
static int pointsTexture = 0;

struct PatrolDragon {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
};

static PatrolDragon dragon;
static int dragonTextures[TOTAL_DRAGON_FRAMES];
static bool texturesLoaded = false;
static int level2TransitionTimerId = -1;

// ===== Texture Loading Routines =====
inline static void loadDragonTextures() {
	if (texturesLoaded) return;

	char path[128];
	for (int i = 1; i <= DRAGON_RIGHT_END; i++) {
		if (i >= 18 && i <= 30) {
			dragonTextures[i] = 0;
			continue;
		}

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTextures[i] = iLoadImage(path);

		if (dragonTextures[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTextures[i] = iLoadImage(path);
		}
	}
	texturesLoaded = true;
}

inline static void loadZedsTextures() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTexture = loadTex("Images/zeds.png", "../Images/zeds.png");
	zedsIconTexture = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture <= 0) zedsIconTexture = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture <= 0) zedsIconTexture = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

inline static void loadEnergyTextures() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures[i] = iLoadImage(path);

		if (energyTextures[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

inline static void loadPauseTextures() {
	static bool loaded = false;
	if (loaded) return;

	totalPointsTexture = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	resumeTexture = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture = loadTex("Images/exit.png", "../Images/exit.png");
	gameOverTexture = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

inline static void loadTransitionTextures() {
	static bool loaded = false;
	if (loaded) return;

	level1CompletedTexture = loadTex("Images/level1completed.png", "../Images/level1completed.png");
	level2StartsTexture = loadTex("Images/level2starts.png", "../Images/level2starts.png");

	loaded = true;
}

inline static void loadGoldBallTextures() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTexture = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture = loadTex("Images/dispear.png", "../Images/dispear.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures[i] = iLoadImage(path);
		if (goldBallIconTextures[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

inline static void loadBlueBallAndPointsTextures() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTexture = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

// ===== Item Placement Logic =====
inline static void initGoldAndBlueBalls(bool freshStart) {
	loadGoldBallTextures();
	loadBlueBallAndPointsTextures();
	goldBallsCollected = 0;
	if (freshStart) playerPoints = 0;
	enterLevel2 = false;

	struct ItemRect { int x, y, w, h; };
	ItemRect placed[12];
	int placedCount = 0;

	// Gold Ball 0: Fixed position on Platform 2 left side
	{
		Platform p = level1_platforms[2];
		int rx = p.x1 + 20;
		int ry = p.y2 + 5;

		goldBalls[0] = { rx, ry, 25, 25, false };
		placed[placedCount++] = { rx, ry, 25, 25 };
	}

	// Gold Ball 1: Platform 2 random placement
	{
		bool valid = false;
		int rx = 0, ry = 0;
		int attempts = 0;
		Platform p = level1_platforms[2];

		while (!valid && attempts < 100) {
			attempts++;
			int minX = p.x1 + 60;
			int maxX = p.x2 - 45;
			if (maxX <= minX) maxX = minX + 1;
			rx = minX + (rand() % (maxX - minX));
			ry = p.y2 + 5;

			valid = true;
			for (int j = 0; j < placedCount; j++) {
				if (rx < placed[j].x + placed[j].w + 35 && rx + 60 > placed[j].x &&
					ry < placed[j].y + placed[j].h + 35 && ry + 60 > placed[j].y) {
					valid = false;
					break;
				}
			}
		}
		if (!valid) { rx = p.x1 + 100; ry = p.y2 + 5; } // Deterministic fallback
		goldBalls[1] = { rx, ry, 25, 25, false };
		placed[placedCount++] = { rx, ry, 25, 25 };
	}

	// Gold Ball 2: Platform 1 random placement
	{
		bool valid = false;
		int rx = 0, ry = 0;
		int attempts = 0;
		Platform p = level1_platforms[1];

		while (!valid && attempts < 100) {
			attempts++;
			int minX = p.x1 + 20;
			int maxX = p.x2 - 45;
			if (maxX <= minX) maxX = minX + 1;
			rx = minX + (rand() % (maxX - minX));
			ry = p.y2 + 5;

			valid = true;
			for (int j = 0; j < placedCount; j++) {
				if (rx < placed[j].x + placed[j].w + 35 && rx + 60 > placed[j].x &&
					ry < placed[j].y + placed[j].h + 35 && ry + 60 > placed[j].y) {
					valid = false;
					break;
				}
			}
		}
		if (!valid) { rx = p.x1 + 40; ry = p.y2 + 5; }
		goldBalls[2] = { rx, ry, 25, 25, false };
		placed[placedCount++] = { rx, ry, 25, 25 };
	}

	// Blue Balls Placement
	for (int i = 0; i < 9; i++) {
		bool valid = false;
		int rx = 0, ry = 0;
		int attempts = 0;
		Platform p = level1_platforms[i % PLATFORM_COUNT];

		while (!valid && attempts < 100) {
			attempts++;
			int minX = p.x1 + 20;
			int maxX = p.x2 - 38;
			if (maxX <= minX) maxX = minX + 1;
			rx = minX + (rand() % (maxX - minX));
			ry = p.y2 + 5;

			valid = true;
			for (int j = 0; j < placedCount; j++) {
				if (rx < placed[j].x + placed[j].w + 30 && rx + 48 > placed[j].x &&
					ry < placed[j].y + placed[j].h + 30 && ry + 48 > placed[j].y) {
					valid = false;
					break;
				}
			}
		}
		if (!valid) { rx = p.x1 + 20 + (i * 25); ry = p.y2 + 5; }
		blueBalls[i] = { rx, ry, 18, 18, false };
		placed[placedCount++] = { rx, ry, 18, 18 };
	}
}

// ===== Collision & Gameplay Routines =====
inline static void checkGoldBallCollision() {
	checkGoldBallCollisionGeneric(goldBalls, &goldBallsCollected, &level1Complete);
}

inline static void checkBlueBallCollision() {
	checkBlueBallCollisionGeneric(blueBalls, &playerPoints);
}

inline static void drawGoldBalls() {
	drawGoldBallsGeneric(goldBalls, goldBallTexture);
}

inline static void drawBlueBalls() {
	drawBlueBallsGeneric(blueBalls, blueBallTexture);
}

inline static void drawPortal() {
	if (level1Complete) {
		if (dispearTexture > 0) {
			iShowImage(680, 20, 35, 100, dispearTexture);
		}
		else {
			iSetColor(255, 255, 0);
			iRectangle(645, 20, 35, 100);
		}
	}
}

inline static void triggerEnterLevel2() {
	enterLevel2 = true;
	showTransitionScreen = false;
	if (level2TransitionTimerId >= 0) {
		iPauseTimer(level2TransitionTimerId);
	}
}

inline static void checkPortalCollision() {
	if (level1Complete && !showTransitionScreen) {
		bool collideX = (player.x + player.width >= 700);
		bool collideY = (player.y <= 120 && player.y + player.height >= 20);

		if (collideX && collideY) {
			// Level 1 main stage done: go straight to the Level 1 sub level.
			// (The "Level 1 Completed / Level 2 Starts" screen is now shown
			// at the end of level1_sub1.h instead.)
			enterLevel2 = true;
		}
	}
}

inline static void drawGoldBallHUD() {
	drawGoldBallHUDGeneric(goldBallsCollected, goldBallIconTextures);
}

inline static void drawPointsHUD() {
	drawPointsHUDGeneric(pointsTexture, playerPoints);
}

inline static void stepNextFrame() {
	int safetyCounter = 0;
	do {
		dragon.animFrame++;

		if (dragon.movingRight) {
			if (dragon.animFrame > DRAGON_RIGHT_END || dragon.animFrame < DRAGON_RIGHT_START)
				dragon.animFrame = DRAGON_RIGHT_START;
		}
		else {
			if (dragon.animFrame > DRAGON_LEFT_END || dragon.animFrame < DRAGON_LEFT_START)
				dragon.animFrame = DRAGON_LEFT_START;
		}

		safetyCounter++;
		if (safetyCounter > TOTAL_DRAGON_FRAMES) break;
	} while (dragonTextures[dragon.animFrame] <= 0);
}

inline static void initDragon() {
	loadDragonTextures();

	dragon.width = 70;
	dragon.height = 70;
	dragon.x = level1_platforms[2].x1 + 10;
	dragon.y = level1_platforms[2].y2 - 20;
	dragon.movingRight = true;
	dragon.animFrame = DRAGON_RIGHT_START;
	dragon.animTimer = 0;

	if (dragonTextures[dragon.animFrame] <= 0) {
		stepNextFrame();
	}
}

inline static void updateDragon() {
	int platformMinX = level1_platforms[2].x1;
	int platformMaxX = level1_platforms[2].x2 - dragon.width;

	if (dragon.movingRight) {
		dragon.x += DRAGON_SPEED;
		if (dragon.x >= platformMaxX) {
			dragon.x = platformMaxX;
			dragon.movingRight = false;
			dragon.animFrame = DRAGON_LEFT_START;
			if (dragonTextures[dragon.animFrame] <= 0) stepNextFrame();
		}
	}
	else {
		dragon.x -= DRAGON_SPEED;
		if (dragon.x <= platformMinX) {
			dragon.x = platformMinX;
			dragon.movingRight = true;
			dragon.animFrame = DRAGON_RIGHT_START;
			if (dragonTextures[dragon.animFrame] <= 0) stepNextFrame();
		}
	}

	dragon.animTimer++;
	if (dragon.animTimer >= DRAGON_ANIM_SPEED) {
		dragon.animTimer = 0;
		stepNextFrame();
	}
}

inline static void drawDragon() {
	int textureId = dragonTextures[dragon.animFrame];

	if (textureId > 0) {
		iShowImage(dragon.x, dragon.y, dragon.width, dragon.height, textureId);
	}
	else {
		iSetColor(255, 0, 0);
		iFilledRectangle(dragon.x, dragon.y, dragon.width, dragon.height);
		iSetColor(255, 255, 255);
		iText(dragon.x + 5, dragon.y + 25, "DRAGON", GLUT_BITMAP_HELVETICA_10);
	}
}

inline static void checkDragonPlayerCollision() {
	if (hitCooldown > 0) {
		hitCooldown--;
		return;
	}

	int hx = dragon.x;
	int hy = dragon.y;
	int hw = dragon.width;
	int hh = dragon.height;

	if (dragon.animFrame >= 1 && dragon.animFrame <= 17) {
		hx = dragon.x + (int)(dragon.width * DRAGON_HITBOX_X_LEFT_OFFSET);
		hy = dragon.y + (int)(dragon.height * DRAGON_HITBOX_Y_OFFSET);
		hw = (int)(dragon.width * DRAGON_HITBOX_WIDTH_SCALE);
		hh = (int)(dragon.height * DRAGON_HITBOX_HEIGHT_SCALE);
	}
	else if (dragon.animFrame >= DRAGON_RIGHT_START && dragon.animFrame <= DRAGON_RIGHT_END) {
		hx = dragon.x + (int)(dragon.width * DRAGON_HITBOX_X_RIGHT_OFFSET);
		hy = dragon.y + (int)(dragon.height * DRAGON_HITBOX_Y_OFFSET);
		hw = (int)(dragon.width * DRAGON_HITBOX_WIDTH_SCALE);
		hh = (int)(dragon.height * DRAGON_HITBOX_HEIGHT_SCALE);
	}

	bool collideX = (player.x + player.width > hx) && (player.x < hx + hw);
	bool collideY = (player.y + player.height > hy) && (player.y < hy + hh);

	if (collideX && collideY) {
		playerLives--;
		if (playerLives <= 0) {
			playerLives = 0;
			isGameOver = true;
		}
		hitCooldown = 30;
	}
}

inline static void drawHUD() {
	drawHUDGeneric(zedsLabelTexture, zedsIconTexture, playerLives, energyFrame, energyTextures);
}

inline static void initLevel1(bool freshStart = true) {
	initPlayer(40, LEVEL_BOTTOM - 10);
	initDragon();
	initGoldAndBlueBalls(freshStart);
	loadPauseTextures();
	loadZedsTextures();
	loadEnergyTextures();

	level1Complete = false;
	isGameOver = false;
	isPaused = false;
	showTransitionScreen = false;
	hitCooldown = 0;

	if (level2TransitionTimerId >= 0) {
		iPauseTimer(level2TransitionTimerId);
	}

	playerLives = 5;
	if (freshStart) energyFrame = 1;
	distanceMoved = 0;
}

inline static void drawLevel1Background() {
	iShowBMP(0, 0, IMG_LV1_BG);
}

inline static void resolvePlatformCollision() {
	int oldY = player.y - player.velocityY;
	int oldTopY = oldY + player.height;
	int currentTopY = player.y + player.height;

	if (player.y <= (LEVEL_BOTTOM - 10)) {
		player.y = LEVEL_BOTTOM - 10;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	player.onGround = false;

	for (int i = 0; i < PLATFORM_COUNT; i++) {
		Platform p = level1_platforms[i];
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
		else if (player.velocityY > 0) {
			int platformBottom = p.y1;
			if (oldTopY <= platformBottom && currentTopY >= platformBottom) {
				player.y = platformBottom - player.height;
				player.velocityY = 0;
				break;
			}
		}
	}
}

inline static void updateLevel1() {
	if (isGameOver || isPaused || showTransitionScreen) return;

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
		if (!(level1Complete && atPortalCoordinates)) {
			player.x = LEVEL_RIGHT - player.width;
		}
	}

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved += moveDist;
		while (distanceMoved >= 150) {
			distanceMoved -= 150;
			if (energyFrame < 145) {
				energyFrame++;
			}
		}
		if (energyFrame >= 145) {
			isGameOver = true;
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

	resolvePlatformCollision();

	if (player.y + player.height > LEVEL_TOP) {
		player.y = LEVEL_TOP - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateDragon();
	checkDragonPlayerCollision();
	checkGoldBallCollision();
	checkBlueBallCollision();
	checkPortalCollision();
}

inline static void drawPauseMenu() {
	drawPauseMenuGeneric(resumeTexture, pauseResumeBtn, restartTexture, pauseRestartBtn, exitTexture, pauseExitBtn);
}

inline static void drawLevel1() {
	drawLevel1Background();
	drawDragon();
	drawGoldBalls();
	drawBlueBalls();
	drawPortal();
	drawPlayer();
	drawHUD();
	drawGoldBallHUD();
	drawPointsHUD();

	if (isPaused) {
		drawPauseMenu();
	}
	else if (showTransitionScreen) {
		if (level1CompletedTexture > 0) {
			iShowImage(165, 270, 350, 70, level1CompletedTexture);
		}
		else {
			iSetColor(255, 255, 255);
			iText(265, 305, "Level 1 Completed", GLUT_BITMAP_HELVETICA_18);
		}

		if (level2StartsTexture > 0) {
			iShowImage(190, 190, 300, 60, level2StartsTexture);
		}
		else {
			iSetColor(255, 255, 255);
			iText(290, 215, "Level 2 Starts", GLUT_BITMAP_HELVETICA_18);
		}
	}
	else if (isGameOver) {
		drawTotalPointsBoxGeneric(totalPointsTexture, playerPoints, 360);

		if (gameOverTexture > 0) {
			iShowImage(175, 185, 350, 160, gameOverTexture);
		}

		if (gameOverRestartTexture > 0)
			iShowImage(gameOverRestartBtn.x1, gameOverRestartBtn.y1, 220, 50, gameOverRestartTexture);

		if (gameOverExitTexture > 0)
			iShowImage(gameOverExitBtn.x1, gameOverExitBtn.y1, 150, 35, gameOverExitTexture);
	}
}

#endif // LEVEL1_H