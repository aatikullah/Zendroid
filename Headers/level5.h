#ifndef LEVEL5_H
#define LEVEL5_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// ============================================================================
// level5.h - LEVEL 5 (main stage)
//
// Architecture, naming and mechanics are taken straight from level4.h. The
// whole level still exposes exactly the same entry points level 4 does, so
// iMain.cpp needs no structural change:
//     initLevel5(bool freshStart), updateLevel5(), drawLevel5(),
//     handleMouseClickLevel5(), isPaused5, isGameOver5, the pause buttons
//     and the game-over buttons.
//
// Level 5 is played as three stages, one file each:
//     level5.h       (this file)  -> level5_sub1.h -> level5_sub2.h
//
// This stage carries level 4's rules unchanged: 3 dragons, killing 1 dragon
// drops 1 golden ball at a random reachable spot, and collecting all 3
// opens the portal on the right edge. Walking into it sets enterLevel5Sub1
// so iMain.cpp can hand over to Sub 1 with points, energy and Zeds intact.
//
// There is NO boss here. Level 5 and both of its sub-levels use the normal
// Images/dragon sprites only - nothing from Images/boss_sprites or
// Images/dragon_boss is loaded anywhere in this stage.
//
// Unchanged from level 4: the fireball breath attack (frames 16 / 54, arc
// physics, trail + explosion particles, 6 fireball hits = 1 Zed), the
// hold-left-mouse 4-punch combo (2 punches = 1 dragon life, 5 lives per
// dragon), blue balls at 10 points, energy drain per 150 px walked, the
// Zeds/energy/points HUD, the pause menu and the game-over screen.
//
// NEW: the five stairs slide LEFT <-> RIGHT as one solid block and two extra
// slabs slide TOP <-> BOTTOM. Every slab is one-way (jump-through), so
// nothing can crush the player.
// (earlier note) the platform layout is a real, moving staircase. slabsfinal.png is
// the slab art (309 x 63, so each slab is drawn 120 x 25 to keep close to
// the source aspect ratio).
// ============================================================================

#define IMG_LV5_BG      "Images/lv5_bg.bmp"
#define IMG_LV5_SLAB    "Images/slabsfinal.png"

// ---------------------------------------------------------------------------
// PLATFORMS
//
// 5 climbing slabs + 2 extra slabs + the fixed ground.
//
// The five stair slabs are ATTACHED. Each step's column starts exactly
// where the previous one ends, and each step's underside sits exactly on
// the previous step's top surface, so the rise between steps IS the slab
// height (21 px). Because slabs are now SOLID on every side, that 21 px
// lip is walked up automatically (see L5_STEP_UP) instead of blocking
// the player, which is what makes the staircase climb like real stairs.
//
// Slabs are 102 x 21. Five in a row span 510 px, leaving a clear strip of
// ground (x 20..140) with nothing overhead - that is where the player
// spawns. Every slab also sits at least 130 px up - comfortably above any
// standing player - so the ground is ALWAYS a free corridor and the player
// can never be walled into a pocket with no way back out.
// ---------------------------------------------------------------------------
#define L5_PLATFORM_COUNT 8

#define L5_GROUND  0
#define L5_STAIR1  1
#define L5_STAIR2  2
#define L5_STAIR3  3
#define L5_STAIR4  4
#define L5_STAIR5  5
#define L5_EXTRA_A 6
#define L5_EXTRA_B 7

#define L5_SLAB_W 102
#define L5_SLAB_H 21

// A lip this low is climbed automatically instead of stopping the player.
#define L5_STEP_UP 22

// Axis each platform travels along.
#define L5_AXIS_NONE 0
#define L5_AXIS_X    1
#define L5_AXIS_Y    2

static Platform level5_platforms[L5_PLATFORM_COUNT] = {
	{ 20, 20, 680, 20 },      // 0 ground (fixed, never drawn)
	{ 122, 110, 224, 131 },   // 1 stair 1   top 131, underside 110: the player
	{ 202, 155, 304, 176 },   // 2 stair 2   top 176   (75 px tall) walks UNDER
	{ 282, 200, 384, 221 },   // 3 stair 3   top 221   the whole staircase.
	{ 362, 245, 464, 266 },   // 4 stair 4   top 266   Each step is 45 px
	{ 442, 290, 544, 311 },   // 5 stair 5   top 311   above the last.
	{ 20, 319, 118, 340 },    // 6 extra A   slides TOP <-> BOTTOM, left column
	{ 588, 199, 680, 220 }    // 7 extra B   slides TOP <-> BOTTOM, right column
};

static const Platform level5_platformStart[L5_PLATFORM_COUNT] = {
	{ 20, 20, 680, 70 },
	{ 122, 110, 224, 131 },
	{ 202, 155, 304, 176 },
	{ 282, 200, 384, 221 },
	{ 362, 245, 464, 266 },
	{ 442, 290, 544, 311 },
	{ 20, 319, 118, 340 },
	{ 588, 199, 680, 220 }
};

// Direction each extra slab starts moving (-1 = down, +1 = up).
static const int L5_START_DIR[L5_PLATFORM_COUNT] = { 0, 0, 0, 0, 0, 0, -1, -1 };

// Range of the extra slabs' TOP surface. Their lowest underside is 105 px,
// so the player can always walk underneath. Each has a column of its own
// (the stairs never reach either column), so nothing overlaps or squeezes.
static const int L5_EXTRA_TOP_MIN[L5_PLATFORM_COUNT] = { 0, 0, 0, 0, 0, 0, 126, 126 };
static const int L5_EXTRA_TOP_MAX[L5_PLATFORM_COUNT] = { 0, 0, 0, 0, 0, 0, 340, 340 };

// ---------------------------------------------------------------------------
// THE RIGID STAIRCASE
//
// Stairs 1..5 are one solid piece that slides left and right. One shared
// offset drives all five, so the staircase shape never changes. The steps
// are spaced with open air between them and every slab is one-way, so the
// staircase can never crush or trap the player.
// ---------------------------------------------------------------------------
#define L5_STAIR_TRAVEL 40       // how far the whole block slides to the right
#define L5_STAIR_SPEED  3.0f     // px per frame
#define L5_EXTRA_SPEED  2.5f     // px per frame

static int stairOffset5 = 0;    // current shift of the block, 0 .. TRAVEL
static int stairDir5 = 1;       // 1 = moving right, -1 = moving left
static float stairCarry5 = 0.0f;

// ---------------------------------------------------------------------------
// Slab speed.
// Level 4 runs at 2 * 1.2 = 2.4 px/frame. Level 5 is a touch quicker at
// 2 * 1.5 = 3.0 px/frame. The float speed + per-slab fractional carry from
// level4.h is kept so the motion stays smooth and the average speed is
// exact rather than being rounded away each frame (see slabStep5()).
// ---------------------------------------------------------------------------
#define SLAB_SPEED_SCALE5 1.5f
static float slabSpeed5[L5_PLATFORM_COUNT] = {
	0.0f,
	2 * SLAB_SPEED_SCALE5, 2 * SLAB_SPEED_SCALE5, 2 * SLAB_SPEED_SCALE5,
	2 * SLAB_SPEED_SCALE5, 2 * SLAB_SPEED_SCALE5, 2 * SLAB_SPEED_SCALE5,
	2 * SLAB_SPEED_SCALE5
};
static float slabCarry5[L5_PLATFORM_COUNT] = { 0.0f };
static int slabDirection5[L5_PLATFORM_COUNT] = { 0 };
static int slabDeltaX5[L5_PLATFORM_COUNT] = { 0 };
static int slabDeltaY5[L5_PLATFORM_COUNT] = { 0 };

static int slabTexture5 = 0;

#define LEVEL5_LEFT   20
#define LEVEL5_RIGHT  680
#define LEVEL5_BOTTOM 25
#define LEVEL5_TOP    480

// ===== Dragons =====
// Exactly level 4's reward rule: 3 dragons, every kill drops 1 golden ball.
#define L5_DRAGON_COUNT   3
#define L5_KILLS_PER_BALL 1

// ===== Dragon Frame Boundaries =====
#define D5_LEFT_START   1
#define D5_LEFT_END     17
#define D5_RIGHT_START  31
#define D5_RIGHT_END    63
#define D5_TOTAL_FRAMES 65

#define D5_SPEED        2
#define D5_ANIM_SPEED   3

// ===== Dragon Health Bar Sprites =====
static int dragonHealthBarTextures5[6] = { 0 }; // 1.png (5 lives) .. 5.png (1 life)

// ===== Fireball Breath Attack (identical to level 4) =====
// Dragons throw a fireball whenever their flight animation reaches frame 16
// (last frame of the left-facing cycle) or frame 54 (mid right-facing
// cycle) - the "mouth open / breathing fire" poses.
#define FIREBALL_ANIM_FRAME_A5 16
#define FIREBALL_ANIM_FRAME_B5 54

#define MAX_FIREBALLS5 10
#define MAX_TRAIL_PARTICLES5 60
#define MAX_EXPLOSION_PARTICLES5 40

struct Fireball5 {
	int x, y;
	int width, height;
	float vx, vy;
	float rotation;
	bool active;
};

struct TrailParticle5 {
	float x, y;
	float radius;
	int alpha;
	bool active;
};

struct ExplosionParticle5 {
	float x, y;
	float vx, vy;
	float radius;
	int life;
	bool active;
};

static Fireball5 fireballs5[MAX_FIREBALLS5];
static TrailParticle5 trailParticles5[MAX_TRAIL_PARTICLES5];
static ExplosionParticle5 explosionParticles5[MAX_EXPLOSION_PARTICLES5];
static int fireballTexture5 = 0;

// Fireball hits needed to drop 1 zed, kept separate from the melee-touch
// counter so the two damage sources never interfere with each other.
static int fireballHitsToPlayer5 = 0;
static int fireballHitCooldown5 = 0;

// ===== Fighting & Combo Variables =====
#define F5_FIGHT_FRAMES     LASER_FRAMES
#define F5_FIGHT_FRAMES_ALL LASER_FRAMES_ALL
#define F5_FIGHT_ANIM_SPEED 7

static int fightingTextures5[F5_FIGHT_FRAMES_ALL + 1] = { 0 };
static bool isFighting5 = false;
static bool isLeftMouseDown5 = false;
static int currentPunchCombo5 = 0;
static int fightFrame5 = 1;
static int fightAnimTimer5 = 0;
static bool punchDamageDealt5 = false;

// ===== Game State Variables =====
static bool level5Complete = false;   // current sub-level's 3 balls collected
static bool isGameOver5 = false;
static bool isPaused5 = false;
static int hitCooldown5 = 0;
static int dragonHitsToPlayer5 = 0;

// ----- Exit transition (Level 5 cleared -> Level 5 Sub 1 starts).
//       Counter driven, no iSetTimer, so the handover can never double-fire
//       or leave a paused timer id behind. -----
static bool enterLevel5Sub1 = false;
static bool showLevel5ExitTransition = false;
static int level5ExitCounter = 0;

// ----- Intro transition (level4 completed -> level5 starts) -----
static bool isLevel5Transition = true;
static int level5TransitionCounter = 0;
static int level4CompletedTexture5 = 0;
static int level5StartsTexture5 = 0;

// ===== Player Lives, Energy & Points Variables =====
static int playerLives5 = 5;
static int& energyFrame5 = gEnergyFrame;
static int distanceMoved5 = 0;
static int energyTextures5[150] = { 0 };
static int& playerPoints5 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTexture5 = 0;
static int zedsIconTexture5 = 0;

// ===== Menu Textures & Bounding Boxes =====
static int resumeTexture5 = 0;
static int restartTexture5 = 0;
static int exitTexture5 = 0;

static Button pauseResumeBtn5 = { 260, 280, 440, 325 };
static Button pauseRestartBtn5 = { 260, 220, 440, 265 };
static Button pauseExitBtn5 = { 260, 160, 440, 205 };

// ===== Game Over Menu Textures & Bounding Boxes =====
static int totalPointsTexture5 = 0;
static int gameOverTexture5 = 0;
static int gameOverRestartTexture5 = 0;
static int gameOverExitTexture5 = 0;

static Button gameOverRestartBtn5 = { 240, 115, 460, 165 };
static Button gameOverExitBtn5 = { 275, 70, 425, 105 };

// ===== Gold Ball Collectibles, Portal & HUD =====
struct GoldBall5 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static GoldBall5 goldBalls5[3];
static int goldBallTexture5 = 0;
static int goldBallIconTextures5[5] = { 0 };
static int goldBallsCollected5 = 0;
static int dispearTexture5 = 0;

// Kill / reward bookkeeping. goldBallsSpawned5 is the single source of
// truth for "how many balls have been handed out this sub-level", so a
// dragon can never pay out twice and a 4th ball can never appear.
static int dragonsKilled5 = 0;
static int goldBallsSpawned5 = 0;

// ===== Blue Ball Collectibles & Points HUD =====
struct BlueBall5 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static BlueBall5 blueBalls5[9];
static int blueBallTexture5 = 0;
static int pointsTexture5 = 0;

struct PatrolDragon5 {
	int x, y;
	int width, height;
	int animFrame;
	int animTimer;
	bool movingRight;
	int health;      // Starts at 5 (5 lives)
	int punchCount;  // 5 laser hits needed per life loss
	bool alive;
	bool rewarded;   // guards against a dead dragon paying out twice
	int platformIndex;
	int minX, maxX;
};

static PatrolDragon5 dragons5[L5_DRAGON_COUNT];

static int dragonTextures5[D5_TOTAL_FRAMES];
static bool texturesLoaded5 = false;

// ---------------------------------------------------------------------------
// Texture loading
// ---------------------------------------------------------------------------
static void loadDragonHealthBarTextures5() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 5; i++) {
		sprintf(path, "Images/Dragonhealthbar/%d.png", i);
		dragonHealthBarTextures5[i] = iLoadImage(path);

		if (dragonHealthBarTextures5[i] <= 0) {
			sprintf(path, "../Images/Dragonhealthbar/%d.png", i);
			dragonHealthBarTextures5[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadDragonTextures5() {
	if (texturesLoaded5) return;

	char path[128];
	for (int i = 1; i <= D5_RIGHT_END; i++) {
		if (i >= 18 && i <= 30) {
			dragonTextures5[i] = 0;
			continue;
		}

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTextures5[i] = iLoadImage(path);

		if (dragonTextures5[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTextures5[i] = iLoadImage(path);
		}
	}

	texturesLoaded5 = true;
}

static void loadFireballTexture5() {
	static bool loaded = false;
	if (loaded) return;
	fireballTexture5 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadFightingTextures5() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= F5_FIGHT_FRAMES_ALL; i++) {
		sprintf(path, "Images/PlayerLaser/shoot/%d.png", i);
		fightingTextures5[i] = iLoadImage(path);

		if (fightingTextures5[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/shoot/%d.png", i);
			fightingTextures5[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

// Laser attack frames: 1-10 face right, 11-20 are their mirrors (facing left).
static int getFightTextureIndex5(int frame, bool facingRight) {
	return laserTextureIndex(frame, facingRight);
}

static void loadTransitionTextures5() {
	static bool loaded = false;
	if (loaded) return;

	level4CompletedTexture5 = loadTex("Images/level4completed.png", "../Images/level4completed.png");
	level5StartsTexture5 = loadTex("Images/level5starts.png", "../Images/level5starts.png");

	loaded = true;
}

static void loadZedsTextures5() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTexture5 = loadTex("Images/zeds.png", "../Images/zeds.png");

	zedsIconTexture5 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture5 <= 0) zedsIconTexture5 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture5 <= 0) zedsIconTexture5 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTextures5() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures5[i] = iLoadImage(path);

		if (energyTextures5[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures5[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadSlabTexture5() {
	static bool loaded = false;
	if (loaded) return;

	slabTexture5 = loadTex(IMG_LV5_SLAB, "../Images/slabsfinal.png");
	if (slabTexture5 <= 0) slabTexture5 = loadTex("Images/slabs.png", "../Images/slabs.png");
	loaded = true;
}

static void loadPauseTextures5() {
	static bool loaded = false;
	if (loaded) return;

	resumeTexture5 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture5 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture5 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTexture5 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTexture5 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture5 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture5 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadGoldBallTextures5() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTexture5 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture5 = loadTex("Images/dispear4.png", "../Images/dispear4.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures5[i] = iLoadImage(path);
		if (goldBallIconTextures5[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures5[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadBlueBallAndPointsTextures5() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTexture5 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture5 = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

// ===========================================================================
// FIREBALL BREATH ATTACK - identical to level4.h, only the suffix differs
// ===========================================================================
static void spawnExplosion5(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES5; j++) {
			if (!explosionParticles5[j].active) {
				explosionParticles5[j].x = x;
				explosionParticles5[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticles5[j].vx = cos(angle) * speed;
				explosionParticles5[j].vy = sin(angle) * speed;
				explosionParticles5[j].radius = 3.0f + (rand() % 3);
				explosionParticles5[j].life = 20 + (rand() % 10);
				explosionParticles5[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticle5(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES5; i++) {
		if (!trailParticles5[i].active) {
			trailParticles5[i].x = x + (rand() % 6 - 3);
			trailParticles5[i].y = y + (rand() % 6 - 3);
			trailParticles5[i].radius = 4.0f + (rand() % 4);
			trailParticles5[i].alpha = 255;
			trailParticles5[i].active = true;
			break;
		}
	}
}

static void spawnFireball5(int dragonIndex) {
	for (int j = 0; j < MAX_FIREBALLS5; j++) {
		if (!fireballs5[j].active) {
			fireballs5[j].x = dragons5[dragonIndex].x + dragons5[dragonIndex].width / 2 - 10;
			fireballs5[j].y = dragons5[dragonIndex].y + dragons5[dragonIndex].height / 2 - 10;
			fireballs5[j].width = 20; fireballs5[j].height = 20;

			float dx = (player.x + player.width / 2.0f) - (fireballs5[j].x + 10);
			float dy = (player.y + player.height / 2.0f) - (fireballs5[j].y + 10);
			float dist = sqrt(dx * dx + dy * dy);
			if (dist == 0) dist = 1;

			float speed = 7.0f;
			fireballs5[j].vx = (dx / dist) * speed;
			// Arc boost for a parabola instead of a flat, unavoidable line shot
			fireballs5[j].vy = (dy / dist) * speed + 4.5f;
			fireballs5[j].rotation = 0.0f;
			fireballs5[j].active = true;
			break;
		}
	}
}

static void updateFireballs5() {
	for (int i = 0; i < MAX_FIREBALLS5; i++) {
		if (!fireballs5[i].active) continue;

		int oldY = fireballs5[i].y;

		// Gravity
		fireballs5[i].vy -= 0.2f;

		fireballs5[i].x += fireballs5[i].vx;
		fireballs5[i].y += fireballs5[i].vy;
		fireballs5[i].rotation += fireballs5[i].vx * 6.0f;

		// Bounce off the ground
		if (fireballs5[i].y <= LEVEL5_BOTTOM) {
			fireballs5[i].y = LEVEL5_BOTTOM;
			fireballs5[i].vy = -fireballs5[i].vy * 0.5f;
			fireballs5[i].vx *= 0.8f;
			spawnExplosion5(fireballs5[i].x + 10, fireballs5[i].y + 10);
			if (fabs(fireballs5[i].vy) < 1.5f) {
				fireballs5[i].active = false;
			}
		}

		// Explode against the floating slabs / ground platform
		if (fireballs5[i].active && fireballs5[i].vy < 0) {
			for (int p = 0; p < L5_PLATFORM_COUNT; p++) {
				Platform plat = level5_platforms[p];

				bool withinX = (fireballs5[i].x + fireballs5[i].width > plat.x1) && (fireballs5[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballs5[i].y <= plat.y2) {
					fireballs5[i].y = plat.y2;
					spawnExplosion5(fireballs5[i].x + 10, fireballs5[i].y + 10);
					fireballs5[i].active = false;
					break;
				}
			}
		}

		if (fireballs5[i].x < -50 || fireballs5[i].x > 750) {
			fireballs5[i].active = false;
		}

		if (fireballs5[i].active)
			spawnTrailParticle5(fireballs5[i].x + 10, fireballs5[i].y + 10);
	}
}

static void updateTrailParticles5() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES5; i++) {
		if (!trailParticles5[i].active) continue;
		trailParticles5[i].alpha -= 15;
		trailParticles5[i].radius -= 0.2f;
		if (trailParticles5[i].alpha <= 0 || trailParticles5[i].radius <= 0) {
			trailParticles5[i].active = false;
		}
	}
}

static void updateExplosionParticles5() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5; i++) {
		if (!explosionParticles5[i].active) continue;
		explosionParticles5[i].x += explosionParticles5[i].vx;
		explosionParticles5[i].y += explosionParticles5[i].vy;
		explosionParticles5[i].vy -= 0.1f;
		explosionParticles5[i].life--;
		explosionParticles5[i].radius -= 0.15f;
		if (explosionParticles5[i].life <= 0 || explosionParticles5[i].radius <= 0) {
			explosionParticles5[i].active = false;
		}
	}
}

// 8 fireball hits to lose 1 zed, independent of the melee-touch counter.
static void checkFireballCollision5() {
	if (fireballHitCooldown5 > 0) {
		fireballHitCooldown5--;
		return;
	}

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS5; i++) {
		if (!fireballs5[i].active) continue;

		bool collideX = (px + pw > fireballs5[i].x) && (px < fireballs5[i].x + fireballs5[i].width);
		bool collideY = (py + ph > fireballs5[i].y) && (py < fireballs5[i].y + fireballs5[i].height);

		if (collideX && collideY) {
			fireballHitsToPlayer5++;

			if (fireballHitsToPlayer5 >= 8) { // 8 fireball hits to lose 1 zed
				playerLives5--;
				fireballHitsToPlayer5 = 0;

				if (playerLives5 <= 0) {
					playerLives5 = 0;
					isGameOver5 = true;
				}
			}

			fireballHitCooldown5 = 30;
			spawnExplosion5(fireballs5[i].x + 10, fireballs5[i].y + 10);
			fireballs5[i].active = false;
			break;
		}
	}
}

static void drawFireballs5() {
	// 1. Trail
	for (int i = 0; i < MAX_TRAIL_PARTICLES5; i++) {
		if (!trailParticles5[i].active) continue;
		int alpha = trailParticles5[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticles5[i].x, trailParticles5[i].y, trailParticles5[i].radius, 20);
	}

	// 2. Explosions
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5; i++) {
		if (!explosionParticles5[i].active) continue;
		if (explosionParticles5[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticles5[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticles5[i].x, explosionParticles5[i].y, explosionParticles5[i].radius, 20);
	}

	// 3. Main fireballs
	for (int i = 0; i < MAX_FIREBALLS5; i++) {
		if (!fireballs5[i].active) continue;
		float cx = fireballs5[i].x + 10;
		float cy = fireballs5[i].y + 10;

		if (fireballTexture5 > 0) {
			iRotate(cx, cy, fireballs5[i].rotation);
			iShowImage(fireballs5[i].x, fireballs5[i].y, fireballs5[i].width, fireballs5[i].height, fireballTexture5);
			iUnRotate();
		}
		else {
			iSetColor(200, 50, 0); iFilledCircle(cx, cy, 14, 50);
			iSetColor(255, 120, 0); iFilledCircle(cx, cy, 10, 50);
			iSetColor(255, 255, 150); iFilledCircle(cx, cy, 5, 50);
		}
	}
}

// ---------------------------------------------------------------------------
// COLLECTIBLES
//
// Gold balls start off-screen exactly like level 4 and only appear when the
// dragons pay out. spawnGoldBall5() always anchors the ball to a real
// platform (a slab top, or the ground) and tags it with that platform's
// index, so it rides the moving slab and can never end up floating in a
// spot the player cannot jump to. The platform and the x offset on it are
// random, so no two runs look the same.
// ---------------------------------------------------------------------------
static void spawnGoldBall5(int ballIndex) {
	if (ballIndex < 0 || ballIndex > 2) return;

	// 1 in 4 balls drops on the ground, the rest on a random slab.
	bool onGround = ((rand() % 4) == 0);

	if (onGround) {
		goldBalls5[ballIndex].x = 60 + (rand() % 540);
		goldBalls5[ballIndex].y = LEVEL5_BOTTOM + 5;
		goldBalls5[ballIndex].platformIndex = L5_GROUND;
		return;
	}

	int p = L5_STAIR1 + (rand() % (L5_PLATFORM_COUNT - 1));
	Platform pl = level5_platforms[p];

	int minX = pl.x1 + 15;
	int maxX = pl.x2 - 40;
	if (maxX <= minX) maxX = minX + 1;

	goldBalls5[ballIndex].x = minX + (rand() % (maxX - minX));
	goldBalls5[ballIndex].y = pl.y2 + 5;
	goldBalls5[ballIndex].platformIndex = p;
}

static void initGoldAndBlueBalls5(bool freshStart) {
	loadGoldBallTextures5();
	loadBlueBallAndPointsTextures5();

	goldBallsCollected5 = 0;
	goldBallsSpawned5 = 0;
	dragonsKilled5 = 0;
	if (freshStart) playerPoints5 = 0;

	// 1. GOLD BALLS - unchanged. Parked off-screen until a dragon pays out.
	for (int i = 0; i < 3; i++) {
		goldBalls5[i].width = 25;
		goldBalls5[i].height = 25;
		goldBalls5[i].x = -1000;
		goldBalls5[i].y = -1000;
		goldBalls5[i].collected = false;
		goldBalls5[i].platformIndex = L5_GROUND;
	}

	// 2. BLUE BALLS ON THE SLABS - placed deliberately now, not scattered
	//    at random. The old random spread kept dropping them inside slabs
	//    or in pockets the player had no route to. One ball now sits in
	//    the middle of every slab, so simply making the climb pays out,
	//    and each ball rides its slab because platformIndex tags it.
	for (int i = L5_STAIR1; i < L5_PLATFORM_COUNT; i++) {
		int b = i - 1;
		blueBalls5[b].width = 18;
		blueBalls5[b].height = 18;
		blueBalls5[b].x = level5_platforms[i].x1 + (L5_SLAB_W / 2) - 9;
		blueBalls5[b].y = level5_platforms[i].y2 + 5;
		blueBalls5[b].collected = false;
		blueBalls5[b].platformIndex = i;
	}

	// 3. THE LAST TWO float over the clear strip of ground beside the
	//    staircase, low enough that one standing jump collects them.
	int airX[2] = { 300, 520 };
	int airY[2] = { 100, 100 };
	for (int i = 0; i < 2; i++) {
		int b = (L5_PLATFORM_COUNT - 1) + i;
		blueBalls5[b].width = 18;
		blueBalls5[b].height = 18;
		blueBalls5[b].x = airX[i];
		blueBalls5[b].y = airY[i];
		blueBalls5[b].collected = false;
		blueBalls5[b].platformIndex = L5_GROUND;
	}
}

static void checkGoldBallCollision5() {
	checkGoldBallCollisionGeneric(goldBalls5, &goldBallsCollected5, &level5Complete);
}

static void checkBlueBallCollision5() {
	checkBlueBallCollisionGeneric(blueBalls5, &playerPoints5);
}

static void drawGoldBalls5() {
	drawGoldBallsGeneric(goldBalls5, goldBallTexture5);
}

static void drawBlueBalls5() {
	drawBlueBallsGeneric(blueBalls5, blueBallTexture5);
}

static void drawGoldBallHUD5() {
	drawGoldBallHUDGeneric(goldBallsCollected5, goldBallIconTextures5);
}

static void drawPointsHUD5() {
	drawPointsHUDGeneric(pointsTexture5, playerPoints5);
}

// ---------------------------------------------------------------------------
// Portal - opens once all 3 golden balls are collected.
// ---------------------------------------------------------------------------
static void drawPortal5() {
	if (level5Complete) {
		if (dispearTexture5 > 0) {
			iShowImage(680, 20, 35, 100, dispearTexture5);
		}
		else {
			iSetColor(255, 255, 0);
			iRectangle(645, 20, 35, 100);
		}
	}
}

// ---------------------------------------------------------------------------
// Dragons
// ---------------------------------------------------------------------------
static void stepNextFrame5(int i) {
	int safetyCounter = 0;
	do {
		dragons5[i].animFrame++;

		if (dragons5[i].movingRight) {
			if (dragons5[i].animFrame > D5_RIGHT_END || dragons5[i].animFrame < D5_RIGHT_START)
				dragons5[i].animFrame = D5_RIGHT_START;
		}
		else {
			if (dragons5[i].animFrame > D5_LEFT_END || dragons5[i].animFrame < D5_LEFT_START)
				dragons5[i].animFrame = D5_LEFT_START;
		}

		safetyCounter++;
		if (safetyCounter > D5_TOTAL_FRAMES) break;
	} while (dragonTextures5[dragons5[i].animFrame] <= 0);
}

static void setupDragon5(int i, int x, int y, int minX, int maxX) {
	dragons5[i].width = 70;
	dragons5[i].height = 70;
	dragons5[i].x = x;
	dragons5[i].y = y;
	dragons5[i].minX = minX;
	dragons5[i].maxX = maxX;
	dragons5[i].movingRight = true;
	dragons5[i].animFrame = D5_RIGHT_START;
	dragons5[i].animTimer = 0;
	dragons5[i].health = 5;
	dragons5[i].punchCount = 0;
	dragons5[i].alive = true;
	dragons5[i].rewarded = false;
	dragons5[i].platformIndex = L5_GROUND;
	if (dragonTextures5[dragons5[i].animFrame] <= 0) stepNextFrame5(i);
}

// Dragon patrol lanes are chosen so a dragon never flies straight through
// the band a staircase slab occupies.
static void initDragon5() {
	loadDragonTextures5();
	loadDragonHealthBarTextures5();

	// One dragon per zone of the climb, so the player meets exactly one at
	// a time instead of all three at once. Every lane sits in air the
	// staircase never sweeps through, so no dragon is ever buried inside a
	// slab, and each one can be reached with a punch from a real standing
	// spot.
	// Every lane sits in air the staircase never sweeps through, so no
	// dragon is ever buried inside a slab, and each one can be reached
	// with a punch from a real standing spot.
	// Every lane sits in air no slab ever sweeps through, so no dragon is
	// ever buried inside one, and each can be punched from a real spot.
	setupDragon5(0, 60, 20, 40, 660);  // ground corridor patrol
	setupDragon5(1, 200, 350, 140, 500);  // mid-high, reachable from the top steps
	setupDragon5(2, 300, 395, 40, 640);   // summit guard above everything
}

static void updateDragon5() {
	for (int i = 0; i < L5_DRAGON_COUNT; i++) {
		if (!dragons5[i].alive) continue;

		int minX = dragons5[i].minX;
		int maxX = dragons5[i].maxX - dragons5[i].width;

		if (dragons5[i].movingRight) {
			dragons5[i].x += D5_SPEED;
			if (dragons5[i].x >= maxX) {
				dragons5[i].x = maxX;
				dragons5[i].movingRight = false;
				dragons5[i].animFrame = D5_LEFT_START;
				if (dragonTextures5[dragons5[i].animFrame] <= 0) stepNextFrame5(i);
			}
		}
		else {
			dragons5[i].x -= D5_SPEED;
			if (dragons5[i].x <= minX) {
				dragons5[i].x = minX;
				dragons5[i].movingRight = true;
				dragons5[i].animFrame = D5_RIGHT_START;
				if (dragonTextures5[dragons5[i].animFrame] <= 0) stepNextFrame5(i);
			}
		}

		dragons5[i].animTimer++;
		if (dragons5[i].animTimer >= D5_ANIM_SPEED) {
			dragons5[i].animTimer = 0;
			stepNextFrame5(i);

			// Breath-attack poses: the instant the animation reaches frame
			// 16 or 54, throw one fireball at the player.
			// 20% fewer fireballs overall: 1-in-5 chance to skip the throw.
			if (dragons5[i].animFrame == FIREBALL_ANIM_FRAME_A5 || dragons5[i].animFrame == FIREBALL_ANIM_FRAME_B5) {
				if (rand() % 5 != 0) spawnFireball5(i);
			}
		}
	}
}

// Called once, and only once, per dragon death: every kill pays out one
// golden ball, and the >= 3 guard means a 4th can never appear.
static void rewardDragonKill5(int i) {
	if (dragons5[i].rewarded) return;
	dragons5[i].rewarded = true;

	dragonsKilled5++;

	if (dragonsKilled5 % L5_KILLS_PER_BALL != 0) return;
	if (goldBallsSpawned5 >= 3) return; // never hand out a 4th ball

	spawnGoldBall5(goldBallsSpawned5);
	goldBallsSpawned5++;
}

// Called by laserUpdateProjectiles() with the laser projectile's rectangle.
// 5 laser hits = 1 dragon life. Returns true if a dragon was hit.
static bool checkPlayerPunchDragonCollision5(int attackX, int attackY, int attackW, int attackH) {
	for (int i = 0; i < L5_DRAGON_COUNT; i++) {
		if (!dragons5[i].alive) continue;

		int dx = dragons5[i].x;
		int dy = dragons5[i].y;
		int dw = dragons5[i].width;
		int dh = dragons5[i].height;

		bool collideX = (attackX + attackW > dx) && (attackX < dx + dw);
		bool collideY = (attackY + attackH > dy) && (attackY < dy + dh);

		if (collideX && collideY) {
			dragons5[i].punchCount++;

			if (dragons5[i].punchCount >= 5) {
				dragons5[i].punchCount = 0;
				dragons5[i].health--;

				if (dragons5[i].health <= 0) {
					dragons5[i].health = 0;
					dragons5[i].alive = false;
					rewardDragonKill5(i);
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

static void updateFightingAnimation5() {
	laserFightStep(isFighting5, isLeftMouseDown5, fightFrame5, fightAnimTimer5, punchDamageDealt5);
	laserUpdateProjectiles(checkPlayerPunchDragonCollision5);   // moves the lasers + hit test
}

static void handleMouseClickLevel5(int button, int state) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			isLeftMouseDown5 = true;
			if (!isFighting5 && !isPaused5 && !isGameOver5 && !showLevel5ExitTransition && !isLevel5Transition) {
				isFighting5 = true;
				fightAnimTimer5 = 0;
				punchDamageDealt5 = false;
				fightFrame5 = 1;
			}
		}
		else if (state == GLUT_UP) {
			isLeftMouseDown5 = false;
		}
	}
}

static void drawDragon5() {
	for (int i = 0; i < L5_DRAGON_COUNT; i++) {
		if (!dragons5[i].alive) continue;

		int textureId = dragonTextures5[dragons5[i].animFrame];

		if (textureId > 0) {
			iShowImage(dragons5[i].x, dragons5[i].y, dragons5[i].width, dragons5[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragons5[i].x, dragons5[i].y, dragons5[i].width, dragons5[i].height);
		}

		// health 5 -> 1.png, health 4 -> 2.png, ...
		int spriteIdx = 6 - dragons5[i].health;

		if (spriteIdx >= 1 && spriteIdx <= 5 && dragonHealthBarTextures5[spriteIdx] > 0) {
			iShowImage(dragons5[i].x + 10, dragons5[i].y + dragons5[i].height + 5, 50, 8, dragonHealthBarTextures5[spriteIdx]);
		}
	}
}

static void checkDragonPlayerCollision5() {
	if (hitCooldown5 > 0) {
		hitCooldown5--;
		return;
	}

	for (int i = 0; i < L5_DRAGON_COUNT; i++) {
		if (!dragons5[i].alive) continue;

		int hx = dragons5[i].x;
		int hy = dragons5[i].y;
		int hw = dragons5[i].width;
		int hh = dragons5[i].height;

		if (dragons5[i].animFrame >= 1 && dragons5[i].animFrame <= 17) {
			hx = dragons5[i].x + (int)(dragons5[i].width * 0.556f);
			hy = dragons5[i].y + (int)(dragons5[i].height * 0.312f);
			hw = (int)(dragons5[i].width * 0.363f);
			hh = (int)(dragons5[i].height * 0.347f);
		}
		else if (dragons5[i].animFrame >= D5_RIGHT_START && dragons5[i].animFrame <= D5_RIGHT_END) {
			hx = dragons5[i].x + (int)(dragons5[i].width * 0.081f);
			hy = dragons5[i].y + (int)(dragons5[i].height * 0.312f);
			hw = (int)(dragons5[i].width * 0.363f);
			hh = (int)(dragons5[i].height * 0.347f);
		}

		bool collideX = (player.x + player.width > hx) && (player.x < hx + hw);
		bool collideY = (player.y + player.height > hy) && (player.y < hy + hh);

		if (collideX && collideY) {
			dragonHitsToPlayer5++;

			// 8 touches to lose 1 zed (life)
			if (dragonHitsToPlayer5 >= 8) {
				playerLives5--;
				dragonHitsToPlayer5 = 0;

				if (playerLives5 <= 0) {
					playerLives5 = 0;
					isGameOver5 = true;
				}
			}

			hitCooldown5 = 30;
			break;
		}
	}
}

static void drawHUD5() {
	drawHUDGeneric(zedsLabelTexture5, zedsIconTexture5, playerLives5, energyFrame5, energyTextures5);
}

// ---------------------------------------------------------------------------
// Stage / level setup
//
// initLevel5Stage() resets the playfield. Zeds, points and energy are
// handled by initLevel5() so a stage reset never silently refills them.
// ---------------------------------------------------------------------------
static void initLevel5Stage(bool freshStart) {
	for (int i = 0; i < L5_PLATFORM_COUNT; i++) {
		level5_platforms[i] = level5_platformStart[i];
		slabDirection5[i] = L5_START_DIR[i];
		slabCarry5[i] = 0.0f;
		slabDeltaX5[i] = 0;
		slabDeltaY5[i] = 0;
	}

	// The staircase always restarts at the bottom of its travel.
	stairOffset5 = 0;
	stairDir5 = 1;
	stairCarry5 = 0.0f;

	initPlayer(40, LEVEL5_BOTTOM);
	initDragon5();
	initGoldAndBlueBalls5(freshStart);

	level5Complete = false;
	hitCooldown5 = 0;
	dragonHitsToPlayer5 = 0;

	currentPunchCombo5 = 0;
	isFighting5 = false;
	punchDamageDealt5 = false;
	laserResetAll();

	for (int i = 0; i < MAX_FIREBALLS5; i++) fireballs5[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES5; i++) trailParticles5[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5; i++) explosionParticles5[i].active = false;
	fireballHitsToPlayer5 = 0;
	fireballHitCooldown5 = 0;

	distanceMoved5 = 0;
}

static void initLevel5(bool freshStart = true) {
	loadSlabTexture5();
	loadPauseTextures5();
	loadZedsTextures5();
	loadEnergyTextures5();
	loadFightingTextures5();
	loadFireballTexture5();
	loadTransitionTextures5();

	initLevel5Stage(freshStart);

	isGameOver5 = false;
	isPaused5 = false;

	enterLevel5Sub1 = false;
	showLevel5ExitTransition = false;
	level5ExitCounter = 0;

	isLevel5Transition = true;
	level5TransitionCounter = 0;

	playerLives5 = 5;
	// Energy and zeds refill at the start of every level.
	energyFrame5 = 1;
}

static void drawLevel5Background() {
	iShowBMP(0, 0, IMG_LV5_BG);
}

// ---------------------------------------------------------------------------
// Collision
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// SOLID SLABS THAT CAN NEVER CRUSH
//
// Every slab is a solid box: the player lands on its top, is stopped by its
// sides and bumps their head on its underside. Slabs and stairs are also
// mounted high enough (underside >= 105 px) for the player to walk under.
// A slab that moves into the player never squeezes them - the player is
// pushed out along the shortest free direction (see pushPlayerOutOfSlabs).
// ---------------------------------------------------------------------------
static bool playerOverlapsSlab5(const Platform& p) {
	return (player.x + player.width > p.x1) && (player.x < p.x2) &&
		(player.y + player.height > p.y1) && (player.y < p.y2);
}

// Vertical: land on tops, bump heads on undersides. Uses where each slab
// WAS before it moved this frame, so a rising slab cannot slip past a
// falling player and a falling slab cannot swallow a rising one.
static void resolvePlatformCollision5() {
	int oldY = player.y - player.velocityY;
	int oldTop = oldY + player.height;

	if (player.y <= LEVEL5_BOTTOM) {
		player.y = LEVEL5_BOTTOM;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	player.onGround = false;

	int bestTop = -100000;
	int bestBottom = 100000;
	for (int i = 1; i < L5_PLATFORM_COUNT; i++) {
		Platform p = level5_platforms[i];
		bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
		if (!withinX) continue;

		int prevTop = p.y2 - slabDeltaY5[i];
		int prevBottom = p.y1 - slabDeltaY5[i];

		if (player.velocityY <= 0) {
			if (oldY >= prevTop - 1 && player.y <= p.y2 && p.y2 > bestTop) bestTop = p.y2;
		}
		else {
			if (oldTop <= prevBottom + 1 && player.y + player.height >= p.y1 && p.y1 < bestBottom) bestBottom = p.y1;
		}
	}

	if (bestTop > -100000) {
		player.y = bestTop;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
	}
	else if (bestBottom < 100000) {
		player.y = bestBottom - player.height;
		player.velocityY = 0;
	}
}

// Horizontal: slab sides are walls.
static void resolveSideCollision5(int oldX) {
	if (player.x == oldX) return;
	bool movingRight = (player.x > oldX);

	for (int i = 1; i < L5_PLATFORM_COUNT; i++) {
		Platform p = level5_platforms[i];
		if (!playerOverlapsSlab5(p)) continue;

		if (movingRight) player.x = p.x1 - player.width;
		else player.x = p.x2;
	}
}

static bool playerStandingOnPlatform5(int index) {
	Platform p = level5_platforms[index];
	bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
	return withinX && player.onGround && player.velocityY <= 0 && abs(player.y - p.y2) <= 2;
}

// A slab that has moved into the player displaces them by the smallest
// distance along a direction that is actually free (never into the floor,
// the walls or the ceiling), so the player is slid out instead of squeezed.
static void pushPlayerOutOfSlabs5() {
	for (int pass = 0; pass < 3; pass++) {
		bool any = false;

		for (int i = 1; i < L5_PLATFORM_COUNT; i++) {
			Platform p = level5_platforms[i];
			if (!playerOverlapsSlab5(p)) continue;
			any = true;

			int up = p.y2 - player.y;
			int down = (player.y + player.height) - p.y1;
			int left = (player.x + player.width) - p.x1;
			int right = p.x2 - player.x;

			if (player.y - down < LEVEL5_BOTTOM) down = 100000;
			if (player.y + up + player.height > LEVEL5_TOP) up = 100000;
			if (player.x - left < LEVEL5_LEFT) left = 100000;
			if (player.x + right + player.width > LEVEL5_RIGHT) right = 100000;

			int best = up;
			int dir = 0;
			if (down < best) { best = down; dir = 1; }
			if (left < best) { best = left; dir = 2; }
			if (right < best) { best = right; dir = 3; }

			if (dir == 0) {
				player.y = p.y2;
				player.velocityY = 0;
				player.onGround = true;
				player.jumping = false;
			}
			else if (dir == 1) {
				player.y = p.y1 - player.height;
				if (player.velocityY > 0) player.velocityY = 0;
			}
			else if (dir == 2) player.x = p.x1 - player.width;
			else player.x = p.x2;
		}

		if (!any) break;
	}
}

// Carries the player and every ball tied to a slab by that slab's movement.
static void applySlabMotion5(const bool* playerOnSlab) {
	for (int i = 1; i < L5_PLATFORM_COUNT; i++) {
		if (playerOnSlab[i]) {
			player.x += slabDeltaX5[i];
			player.y += slabDeltaY5[i];
		}
	}

	// ...and slide them clear if a slab has moved into them.
	pushPlayerOutOfSlabs5();

	for (int i = 0; i < 3; i++) {
		if (!goldBalls5[i].collected) {
			int p = goldBalls5[i].platformIndex;
			goldBalls5[i].x += slabDeltaX5[p];
			goldBalls5[i].y += slabDeltaY5[p];
		}
	}

	for (int i = 0; i < 9; i++) {
		if (!blueBalls5[i].collected) {
			int p = blueBalls5[i].platformIndex;
			blueBalls5[i].x += slabDeltaX5[p];
			blueBalls5[i].y += slabDeltaY5[p];
		}
	}
}


// Horizontal step of the whole staircase this frame.
static int moveStaircase5() {
	float raw = L5_STAIR_SPEED * stairDir5 + stairCarry5;
	int dx = (int)raw;
	stairCarry5 = raw - dx;

	int newOffset = stairOffset5 + dx;
	if (newOffset <= 0) {
		newOffset = 0;
		stairDir5 = 1;
		stairCarry5 = 0.0f;
	}
	else if (newOffset >= L5_STAIR_TRAVEL) {
		newOffset = L5_STAIR_TRAVEL;
		stairDir5 = -1;
		stairCarry5 = 0.0f;
	}

	dx = newOffset - stairOffset5;
	stairOffset5 = newOffset;
	return dx;
}

// Vertical step of an extra slab; bounces between its top and bottom stops.
static int moveExtraSlab5(int i) {
	float raw = L5_EXTRA_SPEED * slabDirection5[i] + slabCarry5[i];
	int dy = (int)raw;
	slabCarry5[i] = raw - dy;

	int h = level5_platforms[i].y2 - level5_platforms[i].y1;
	int newTop = level5_platforms[i].y2 + dy;

	if (newTop <= L5_EXTRA_TOP_MIN[i]) {
		newTop = L5_EXTRA_TOP_MIN[i];
		slabDirection5[i] = 1;
		slabCarry5[i] = 0.0f;
	}
	else if (newTop >= L5_EXTRA_TOP_MAX[i]) {
		newTop = L5_EXTRA_TOP_MAX[i];
		slabDirection5[i] = -1;
		slabCarry5[i] = 0.0f;
	}

	dy = newTop - level5_platforms[i].y2;
	level5_platforms[i].y2 = newTop;
	level5_platforms[i].y1 = newTop - h;
	return dy;
}

static void updateMovingSlabs5() {
	for (int i = 0; i < L5_PLATFORM_COUNT; i++) {
		slabDeltaX5[i] = 0;
		slabDeltaY5[i] = 0;
	}

	bool playerOnSlab[L5_PLATFORM_COUNT] = { false };
	for (int i = 1; i < L5_PLATFORM_COUNT; i++) {
		playerOnSlab[i] = playerStandingOnPlatform5(i);
	}

	// The five stairs slide left <-> right together.
	int stairDx = moveStaircase5();
	for (int i = L5_STAIR1; i <= L5_STAIR5; i++) {
		level5_platforms[i].x1 += stairDx;
		level5_platforms[i].x2 += stairDx;
		slabDeltaX5[i] = stairDx;
	}

	// The two extra slabs slide top <-> bottom.
	for (int i = L5_EXTRA_A; i <= L5_EXTRA_B; i++) {
		slabDeltaY5[i] = moveExtraSlab5(i);
	}

	applySlabMotion5(playerOnSlab);
}

static void drawPlatforms5() {
	if (slabTexture5 <= 0) return;

	for (int i = 1; i < L5_PLATFORM_COUNT; i++) {
		int width = level5_platforms[i].x2 - level5_platforms[i].x1;
		int height = level5_platforms[i].y2 - level5_platforms[i].y1;
		iShowImage(level5_platforms[i].x1, level5_platforms[i].y1, width, height, slabTexture5);
	}
}

// ---------------------------------------------------------------------------
// Level 5 -> Level 5 Sub 1 handover
// ---------------------------------------------------------------------------
static void checkPortalCollision5() {
	if (!level5Complete || showLevel5ExitTransition || enterLevel5Sub1) return;

	bool collideX = (player.x + player.width >= 700);
	bool collideY = (player.y <= 120 && player.y + player.height >= 20);

	if (collideX && collideY) {
		showLevel5ExitTransition = true;
		level5ExitCounter = 0;
	}
}

static void updateLevel5() {
	// Intro banner
	if (isLevel5Transition) {
		level5TransitionCounter++;
		if (level5TransitionCounter >= 100) {
			isLevel5Transition = false;
		}
		return;
	}

	// Exit banner, then hand over to Sub 1. enterLevel5Sub1 latches, so the
	// handover is raised exactly once.
	if (showLevel5ExitTransition) {
		level5ExitCounter++;
		if (level5ExitCounter >= 90) {
			showLevel5ExitTransition = false;
			level5ExitCounter = 0;
			enterLevel5Sub1 = true;
		}
		return;
	}

	if (isGameOver5 || isPaused5 || enterLevel5Sub1) return;

	updateMovingSlabs5();

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

	if (player.x < LEVEL5_LEFT) player.x = LEVEL5_LEFT;

	if (player.x + player.width > LEVEL5_RIGHT) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level5Complete && atPortalCoordinates)) {
			player.x = LEVEL5_RIGHT - player.width;
		}
	}

	// Slabs are solid boxes - stop the player walking through one.
	resolveSideCollision5(oldX);

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved5 += moveDist;
		while (distanceMoved5 >= 150) {
			distanceMoved5 -= 150;
			if (energyFrame5 < 145) {
				energyFrame5++;
			}
		}
		if (energyFrame5 >= 145) {
			isGameOver5 = true;
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

	resolvePlatformCollision5();

	if (player.y + player.height > LEVEL5_TOP) {
		player.y = LEVEL5_TOP - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateFightingAnimation5();
	updateDragon5();
	checkDragonPlayerCollision5();

	// --- FIREBALL & PARTICLE UPDATES (same order as level 4) ---
	updateFireballs5();
	updateTrailParticles5();
	updateExplosionParticles5();
	checkFireballCollision5();

	checkGoldBallCollision5();
	checkBlueBallCollision5();
	checkPortalCollision5();
}

static void drawPauseMenu5() {
	drawPauseMenuGeneric(resumeTexture5, pauseResumeBtn5, restartTexture5, pauseRestartBtn5, exitTexture5, pauseExitBtn5);
}

static void drawLevel5() {
	drawLevel5Background();

	if (isLevel5Transition) {
		if (level4CompletedTexture5 > 0) {
			iShowImage(165, 270, 350, 70, level4CompletedTexture5);
		}
		else {
			iSetColor(255, 255, 255);
			iText(265, 305, "Level 4 Completed", GLUT_BITMAP_HELVETICA_18);
		}

		if (level5StartsTexture5 > 0) {
			iShowImage(190, 190, 300, 60, level5StartsTexture5);
		}
		else {
			iSetColor(255, 255, 255);
			iText(290, 215, "Level 5 Starts", GLUT_BITMAP_HELVETICA_18);
		}
		return;
	}

	drawPlatforms5();
	drawDragon5();
	drawGoldBalls5();
	drawBlueBalls5();
	drawPortal5();

	if (!(isFighting5 && drawLaserFightSprite(fightingTextures5, fightFrame5, player.facingRight))) {
		drawGunPlayerBody();
	}
	drawLaserProjectiles();

	drawFireballs5(); // trails, explosions and the fireballs themselves

	drawHUD5();
	drawGoldBallHUD5();
	drawPointsHUD5();

	if (isPaused5) {
		drawPauseMenu5();
	}
	else if (isGameOver5) {
		drawTotalPointsBoxGeneric(totalPointsTexture5, playerPoints5, 405);

		if (gameOverTexture5 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTexture5);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTexture5 > 0)
			iShowImage(gameOverRestartBtn5.x1, gameOverRestartBtn5.y1, 220, 50, gameOverRestartTexture5);

		if (gameOverExitTexture5 > 0)
			iShowImage(gameOverExitBtn5.x1, gameOverExitBtn5.y1, 150, 35, gameOverExitTexture5);
	}
}

#endif