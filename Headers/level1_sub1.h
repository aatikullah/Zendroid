#ifndef LEVEL1_L1S1_H
#define LEVEL1_L1S1_H

#include <math.h> 
#include <stdlib.h> // For rand()
#include <stdio.h>  // For sprintf()
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// Background image for this sub level (700x500 BMP)
#ifndef IMG_LV1_L1S1_BG
#define IMG_LV1_L1S1_BG "Images/lv1_sub1bg.bmp"
#endif

// =====================================================================
//  Everything below lives in namespace L1Sub1 so it can be included together
//  with level1.h without any name clashes (same variable / function names
//  as level1.h, just wrapped).  Global wrappers are at the bottom:
//     initLevel1Sub1(), updateLevel1Sub1(), drawLevel1Sub1()
// =====================================================================
namespace L1Sub1 {

	// ===== Level Boundaries & Constants =====
#define L1S1_PLATFORM_COUNT 5
	static const Platform level1_platforms[L1S1_PLATFORM_COUNT] = {
		// { x1 (left), y1 (bottom edge), x2 (right), y2 (top surface) }
		// Coordinates are taken from lv1_sub1bg (700x500, origin bottom-left)
		{ 163, 100, 537, 136 },  // 0: Bottom-Center slab (above the dragon statue)
		{ 20, 221, 252, 244 },  // 1: Middle-Left slab
		{ 395, 282, 677, 309 },  // 2: Middle-Right slab
		{ 519, 374, 677, 409 },  // 3: Top-Right slab
		{ 20, 358, 287, 395 }   // 4: Top-Left slab
	};

#define L1S1_LEVEL_LEFT   20
#define L1S1_LEVEL_RIGHT  680
#define L1S1_LEVEL_BOTTOM 20
#define L1S1_LEVEL_TOP    500

	// ===== Dragon Animation Boundaries =====
#define L1S1_DRAGON_LEFT_START    1
#define L1S1_DRAGON_LEFT_END      17
#define L1S1_DRAGON_RIGHT_START   31
#define L1S1_DRAGON_RIGHT_END     63
#define L1S1_TOTAL_DRAGON_FRAMES  65

#define L1S1_DRAGON_SPEED        2
#define L1S1_DRAGON_ANIM_SPEED   3 

#define L1S1_DRAGON_COUNT        2

	// Jump strength multiplier for this sub level (1.2 = +20%)
#define L1S1_JUMP_MULT           1.2f

	// Dragon Hitbox Bounding Box Multipliers
#define L1S1_DRAGON_HITBOX_X_LEFT_OFFSET   0.556f
#define L1S1_DRAGON_HITBOX_X_RIGHT_OFFSET  0.081f
#define L1S1_DRAGON_HITBOX_Y_OFFSET        0.312f
#define L1S1_DRAGON_HITBOX_WIDTH_SCALE     0.363f
#define L1S1_DRAGON_HITBOX_HEIGHT_SCALE    0.347f

	// ===== Game State Variables =====
	static bool level1Complete = false;   // sub level complete (all gold balls collected)
	static bool isGameOver = false;
	static bool isPaused = false;
	static bool enterLevel2 = false;      // becomes true after the transition -> go to level 2
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
		int platformIndex;   // slab this dragon patrols on
	};

	static PatrolDragon dragons[L1S1_DRAGON_COUNT];
	static int dragonTextures[L1S1_TOTAL_DRAGON_FRAMES];
	static bool texturesLoaded = false;
	static int level2TransitionTimerId = -1;

	// ===== Texture Loading Routines =====
	inline static void loadDragonTextures() {
		if (texturesLoaded) return;

		char path[128];
		for (int i = 1; i <= L1S1_DRAGON_RIGHT_END; i++) {
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

		// Gold Ball 0: Fixed position on Platform 4 (Top-Left slab), left side
		{
			Platform p = level1_platforms[4];
			int rx = p.x1 + 30;
			int ry = p.y2 + 5;

			goldBalls[0] = { rx, ry, 25, 25, false };
			placed[placedCount++] = { rx, ry, 25, 25 };
		}

		// Gold Ball 1: Platform 3 (Top-Right slab) random placement
		{
			bool valid = false;
			int rx = 0, ry = 0;
			int attempts = 0;
			Platform p = level1_platforms[3];

			while (!valid && attempts < 100) {
				attempts++;
				int minX = p.x1 + 30;
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
			if (!valid) { rx = p.x1 + 60; ry = p.y2 + 5; } // Deterministic fallback
			goldBalls[1] = { rx, ry, 25, 25, false };
			placed[placedCount++] = { rx, ry, 25, 25 };
		}

		// Gold Ball 2: Platform 1 (Middle-Left slab) random placement
		{
			bool valid = false;
			int rx = 0, ry = 0;
			int attempts = 0;
			Platform p = level1_platforms[1];

			while (!valid && attempts < 100) {
				attempts++;
				int minX = p.x1 + 30;
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
			if (!valid) { rx = p.x1 + 60; ry = p.y2 + 5; }
			goldBalls[2] = { rx, ry, 25, 25, false };
			placed[placedCount++] = { rx, ry, 25, 25 };
		}

		// Blue Balls Placement (spread over all 5 slabs)
		for (int i = 0; i < 9; i++) {
			bool valid = false;
			int rx = 0, ry = 0;
			int attempts = 0;
			Platform p = level1_platforms[i % L1S1_PLATFORM_COUNT];

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

	// Called by the timer 1 second after "Level 1 Completed / Level 2 Starts" is shown
	inline static void triggerEnterLevel2() {
		enterLevel2 = true;
		showTransitionScreen = false;
		if (level2TransitionTimerId >= 0) {
			iPauseTimer(level2TransitionTimerId);
		}
	}

	// Sub level finished + player walks into the portal -> show transition screen
	inline static void checkPortalCollision() {
		if (level1Complete && !showTransitionScreen) {
			bool collideX = (player.x + player.width >= 700);
			bool collideY = (player.y <= 120 && player.y + player.height >= 20);

			if (collideX && collideY) {
				showTransitionScreen = true;
				loadTransitionTextures();
				if (level2TransitionTimerId < 0) {
					level2TransitionTimerId = iSetTimer(1000, triggerEnterLevel2);
				}
				else {
					iResumeTimer(level2TransitionTimerId);
				}
			}
		}
	}

	inline static void drawGoldBallHUD() {
		drawGoldBallHUDGeneric(goldBallsCollected, goldBallIconTextures);
	}

	inline static void drawPointsHUD() {
		drawPointsHUDGeneric(pointsTexture, playerPoints);
	}

	// ===== Dragons (2 patrol dragons) =====
	inline static void stepNextFrame(PatrolDragon& d) {
		int safetyCounter = 0;
		do {
			d.animFrame++;

			if (d.movingRight) {
				if (d.animFrame > L1S1_DRAGON_RIGHT_END || d.animFrame < L1S1_DRAGON_RIGHT_START)
					d.animFrame = L1S1_DRAGON_RIGHT_START;
			}
			else {
				if (d.animFrame > L1S1_DRAGON_LEFT_END || d.animFrame < L1S1_DRAGON_LEFT_START)
					d.animFrame = L1S1_DRAGON_LEFT_START;
			}

			safetyCounter++;
			if (safetyCounter > L1S1_TOTAL_DRAGON_FRAMES) break;
		} while (dragonTextures[d.animFrame] <= 0);
	}

	// Patrol limits of a dragon (stay inside the visible playfield and on its slab)
	inline static int dragonMinX(const PatrolDragon& d) {
		int minX = level1_platforms[d.platformIndex].x1;
		return (minX < L1S1_LEVEL_LEFT) ? L1S1_LEVEL_LEFT : minX;
	}

	inline static int dragonMaxX(const PatrolDragon& d) {
		int maxX = level1_platforms[d.platformIndex].x2;
		if (maxX > L1S1_LEVEL_RIGHT) maxX = L1S1_LEVEL_RIGHT;
		return maxX - d.width;
	}

	inline static void initOneDragon(PatrolDragon& d, int platformIndex, bool startRight) {
		d.width = 70;
		d.height = 70;
		d.platformIndex = platformIndex;
		d.y = level1_platforms[platformIndex].y2 - 20;
		d.movingRight = startRight;
		d.animTimer = 0;

		if (startRight) {
			d.x = dragonMinX(d) + 10;
			d.animFrame = L1S1_DRAGON_RIGHT_START;
		}
		else {
			d.x = dragonMaxX(d) - 10;
			d.animFrame = L1S1_DRAGON_LEFT_START;
		}

		if (dragonTextures[d.animFrame] <= 0) {
			stepNextFrame(d);
		}
	}

	inline static void initDragons() {
		loadDragonTextures();

		// Dragon 1: Top-Left slab (index 4), starts walking right
		initOneDragon(dragons[0], 4, true);
		// Dragon 2: Middle-Right slab (index 2), starts walking left
		initOneDragon(dragons[1], 2, false);
	}

	inline static void updateOneDragon(PatrolDragon& d) {
		int platformMinX = dragonMinX(d);
		int platformMaxX = dragonMaxX(d);

		if (d.movingRight) {
			d.x += L1S1_DRAGON_SPEED;
			if (d.x >= platformMaxX) {
				d.x = platformMaxX;
				d.movingRight = false;
				d.animFrame = L1S1_DRAGON_LEFT_START;
				if (dragonTextures[d.animFrame] <= 0) stepNextFrame(d);
			}
		}
		else {
			d.x -= L1S1_DRAGON_SPEED;
			if (d.x <= platformMinX) {
				d.x = platformMinX;
				d.movingRight = true;
				d.animFrame = L1S1_DRAGON_RIGHT_START;
				if (dragonTextures[d.animFrame] <= 0) stepNextFrame(d);
			}
		}

		d.animTimer++;
		if (d.animTimer >= L1S1_DRAGON_ANIM_SPEED) {
			d.animTimer = 0;
			stepNextFrame(d);
		}
	}

	inline static void updateDragons() {
		for (int i = 0; i < L1S1_DRAGON_COUNT; i++) updateOneDragon(dragons[i]);
	}

	inline static void drawOneDragon(const PatrolDragon& d) {
		int textureId = dragonTextures[d.animFrame];

		if (textureId > 0) {
			iShowImage(d.x, d.y, d.width, d.height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(d.x, d.y, d.width, d.height);
			iSetColor(255, 255, 255);
			iText(d.x + 5, d.y + 25, "DRAGON", GLUT_BITMAP_HELVETICA_10);
		}
	}

	inline static void drawDragons() {
		for (int i = 0; i < L1S1_DRAGON_COUNT; i++) drawOneDragon(dragons[i]);
	}

	inline static void checkDragonPlayerCollision() {
		if (hitCooldown > 0) {
			hitCooldown--;
			return;
		}

		for (int i = 0; i < L1S1_DRAGON_COUNT; i++) {
			PatrolDragon& d = dragons[i];

			int hx = d.x;
			int hy = d.y;
			int hw = d.width;
			int hh = d.height;

			if (d.animFrame >= 1 && d.animFrame <= 17) {
				hx = d.x + (int)(d.width * L1S1_DRAGON_HITBOX_X_LEFT_OFFSET);
				hy = d.y + (int)(d.height * L1S1_DRAGON_HITBOX_Y_OFFSET);
				hw = (int)(d.width * L1S1_DRAGON_HITBOX_WIDTH_SCALE);
				hh = (int)(d.height * L1S1_DRAGON_HITBOX_HEIGHT_SCALE);
			}
			else if (d.animFrame >= L1S1_DRAGON_RIGHT_START && d.animFrame <= L1S1_DRAGON_RIGHT_END) {
				hx = d.x + (int)(d.width * L1S1_DRAGON_HITBOX_X_RIGHT_OFFSET);
				hy = d.y + (int)(d.height * L1S1_DRAGON_HITBOX_Y_OFFSET);
				hw = (int)(d.width * L1S1_DRAGON_HITBOX_WIDTH_SCALE);
				hh = (int)(d.height * L1S1_DRAGON_HITBOX_HEIGHT_SCALE);
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
				break; // only one hit per cooldown, even if both dragons touch the player
			}
		}
	}

	inline static void drawHUD() {
		drawHUDGeneric(zedsLabelTexture, zedsIconTexture, playerLives, energyFrame, energyTextures);
	}

	inline static void initLevel1Sub1(bool freshStart = false) {
		initPlayer(40, L1S1_LEVEL_BOTTOM - 10);
		initDragons();
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

	inline static void drawLevel1Sub1Background() {
		iShowBMP(0, 0, (char*)IMG_LV1_L1S1_BG);
	}

	inline static void resolvePlatformCollision() {
		int oldY = player.y - player.velocityY;
		int oldTopY = oldY + player.height;
		int currentTopY = player.y + player.height;

		if (player.y <= (L1S1_LEVEL_BOTTOM - 10)) {
			player.y = L1S1_LEVEL_BOTTOM - 10;
			player.velocityY = 0;
			player.onGround = true;
			player.jumping = false;
			return;
		}

		player.onGround = false;

		for (int i = 0; i < L1S1_PLATFORM_COUNT; i++) {
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

	inline static void updateLevel1Sub1() {
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

		if (player.x < L1S1_LEVEL_LEFT) player.x = L1S1_LEVEL_LEFT;

		if (player.x + player.width > L1S1_LEVEL_RIGHT) {
			bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
			if (!(level1Complete && atPortalCoordinates)) {
				player.x = L1S1_LEVEL_RIGHT - player.width;
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
			player.velocityY = JUMP_FORCE * L1S1_JUMP_MULT;
			player.onGround = false;
			player.jumping = true; requestJumpSfx();
		}

		player.velocityY -= GRAVITY;
		if (player.velocityY < -MAX_FALL_SPEED)
			player.velocityY = -MAX_FALL_SPEED;

		player.y += player.velocityY;

		resolvePlatformCollision();

		if (player.y + player.height > L1S1_LEVEL_TOP) {
			player.y = L1S1_LEVEL_TOP - player.height;
			player.velocityY = 0;
		}

		updatePlayerAnimation();
		updateDragons();
		checkDragonPlayerCollision();
		checkGoldBallCollision();
		checkBlueBallCollision();
		checkPortalCollision();
	}

	inline static void drawPauseMenu() {
		drawPauseMenuGeneric(resumeTexture, pauseResumeBtn, restartTexture, pauseRestartBtn, exitTexture, pauseExitBtn);
	}

	inline static void drawLevel1Sub1() {
		drawLevel1Sub1Background();
		drawDragons();
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
			// "Level 1 Completed" then "Level 2 Starts"
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

} // namespace L1Sub1

// =====================================================================
//  Global interface (use these from iMain.cpp, just like level1.h)
// =====================================================================
inline static void initLevel1Sub1(bool freshStart = false) { L1Sub1::initLevel1Sub1(freshStart); }
inline static void updateLevel1Sub1()                      { L1Sub1::updateLevel1Sub1(); }
inline static void drawLevel1Sub1()                        { L1Sub1::drawLevel1Sub1(); }

// State / button references for key & mouse handlers in main.cpp
static bool& l1s1Complete = L1Sub1::level1Complete;
static bool& l1s1IsGameOver = L1Sub1::isGameOver;
static bool& l1s1IsPaused = L1Sub1::isPaused;
static bool& l1s1EnterLevel2 = L1Sub1::enterLevel2;   // true -> switch to Level 2
static bool& l1s1ShowTransition = L1Sub1::showTransitionScreen;
static int&  l1s1PlayerLives = L1Sub1::playerLives;
static Button& l1s1PauseResumeBtn = L1Sub1::pauseResumeBtn;
static Button& l1s1PauseRestartBtn = L1Sub1::pauseRestartBtn;
static Button& l1s1PauseExitBtn = L1Sub1::pauseExitBtn;
static Button& l1s1GameOverRestartBtn = L1Sub1::gameOverRestartBtn;
static Button& l1s1GameOverExitBtn = L1Sub1::gameOverExitBtn;

#endif // LEVEL1_L1S1_H