#ifndef LEVEL5S2_H
#define LEVEL5S2_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// ============================================================================
// level5S2_sub2.h - LEVEL 5, SUB LEVEL 2 (FINAL ROUND)
//
// Architecture, naming and mechanics are taken straight from level4.h. The
// whole level still exposes exactly the same entry points level 4 does, so
// iMain.cpp needs no structural change:
//     initLevel5S2(bool freshStart), updateLevel5S2(), drawLevel5S2(),
//     handleMouseClickLevel5S2(), isPaused5S2, isGameOver5S2, the pause buttons
//     and the game-over buttons.
//
// Level 5 is played as three stages, one file each:
//     level5S2.h -> level5S2_sub1.h -> level5S2_sub2.h  (this file)
//
// THE FINAL ROUND. Exactly 3 normal dragons, ALL ON THE FIELD AT ONCE (sprites in
// Images/dragon/). The RED DRAGON is reserved for the final boss. Every normal
// dragon drops one golden ball at a random
// reachable spot when it dies (like level 4):
//
//     Dragon 1 dead -> ball 1,  Dragon 2 dead -> ball 2,  Dragon 3 dead -> ball 3
//
// The dragons walk on the ground, claw when you are close and breathe fire
// when you are further away. Lasers only hit them from ground level. There is no portal and NO level-complete screen after
// the third ball: instead the BOSS DRAGON walks in from the isRight edge (see
// the BOSS DRAGON section) and the fight begins.
//
// The player wins ONLY when the boss is dead: its collapse animation
// finishes, gameWon5S2 latches and win.png is shown once. No fourth ball can
// ever be handed out.
//
// The staircase is a centre-peak layout, different from both level5S2.h and
// level5S2_sub1.h, with three extra slabs in their own free bands.
//
// The boss uses the sprites in Images/BossDragon/ (built from the 28-frame
// fire-breath sheet, which is kept there as source_sheet_28frames.png).
//
// Unchanged from level 4: the fireball breath attack (frames 16 / 54, arc
// physics, trail + explosion particles, 6 fireball hits = 1 Zed), the
// hold-left-mouse 4-punch combo (2 punches = 1 dragon life, 5 lives per
// dragon), blue balls at 10 points, energy drain per 150 px walked, the
// Zeds/energy/points HUD, the pause menu and the game-over screen.
//
// NEW: the platform layout is a real, moving staircase. slabsfinal.png is
// the slab art (309 x 63, so each slab is drawn 120 x 25 to keep close to
// the source aspect ratio).
// ============================================================================

#define IMG_LV5S2_BG      "Images/lv5_bg.bmp"
#define IMG_LV5S2_SLAB    "Images/slabsfinal.png"
#define IMG_LV5S2_WIN     "Images/win.png"

// ---------------------------------------------------------------------------
// PLATFORMS
//
// 5 climbing slabs + 2 extra slabs + the fixed ground.
//
// The five stair slabs are ATTACHED. Each step's column starts exactly
// where the previous one ends, and each step's underside sits exactly on
// the previous step's top surface, so the rise between steps IS the slab
// height (21 px). Because slabs are now SOLID on every side, that 21 px
// lip is walked up automatically (see L5S2_STEP_UP) instead of blocking
// the player, which is what makes the staircase climb like real stairs.
//
// Slabs are 102 x 21. Five in a row span 510 px, leaving a clear strip of
// ground (x 20..140) with nothing overhead - that is where the player
// spawns. Every slab also sits at least 130 px up - comfortably above any
// standing player - so the ground is ALWAYS a free corridor and the player
// can never be walled into a pocket with no way back out.
// ---------------------------------------------------------------------------
#define L5S2_PLATFORM_COUNT 8

#define L5S2_GROUND  0
#define L5S2_STAIR1  1
#define L5S2_STAIR2  2
#define L5S2_STAIR3  3
#define L5S2_STAIR4  4
#define L5S2_STAIR5  5
#define L5S2_EXTRA_A 6
#define L5S2_EXTRA_B 7

#define L5S2_SLAB_W 102
#define L5S2_SLAB_H 21

// A lip this low is climbed automatically instead of stopping the player.
#define L5S2_STEP_UP 22

// Axis each platform travels along.
#define L5S2_AXIS_NONE 0
#define L5S2_AXIS_X    1
#define L5S2_AXIS_Y    2

static Platform level5S2_platforms[L5S2_PLATFORM_COUNT] = {
	{ 20, 20, 680, 20 },      // 0 ground (fixed, never drawn)
	{ 30, 110, 132, 131 },    // 1..4 conveyor stairs, 120 px apart, diagonal loop
	{ 150, 152, 252, 173 },   //      exactly as in sub 1 (set by resetPath5S2 /
	{ 270, 194, 372, 215 },   //      updateMovingSlabs5S2)
	{ 390, 236, 492, 257 },
	{ 558, 375, 660, 396 },   // 5..7 top-lane slabs, glide isRight -> left
	{ 379, 375, 481, 396 },
	{ 200, 375, 302, 396 }
};

static const Platform level5S2_platformStart[L5S2_PLATFORM_COUNT] = {
	{ 20, 20, 680, 70 },
	{ 30, 110, 132, 131 },
	{ 150, 152, 252, 173 },
	{ 270, 194, 372, 215 },
	{ 390, 236, 492, 257 },
	{ 558, 375, 660, 396 },
	{ 379, 375, 481, 396 },
	{ 200, 375, 302, 396 }
};

static const int L5S2_START_DIR[L5S2_PLATFORM_COUNT] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// ---------------------------------------------------------------------------
// THE LOOPING STAIRCASE (same as sub 1)
//
// Four slabs, 120 px apart with a clear gap between them, ride a diagonal from the bottom-left up towards the isRight and
// loop back to the start. A slab someone stands on waits at the end until
// they step off.
// ---------------------------------------------------------------------------
#define L5S2_PATH_X0    30
#define L5S2_PATH_Y0    131
#define L5S2_PATH_LEN   480
#define L5S2_PATH_SPEED 2.0f
#define L5S2_PATH_SLABS 4

static float pathPos5S2[L5S2_PATH_SLABS] = { 0, 120, 240, 360 };

static void placePathSlab5S2(int slab, float s) {
	int i = slab + 1;
	int x1 = L5S2_PATH_X0 + (int)s;
	int top = L5S2_PATH_Y0 + (int)(s * 0.35f);
	level5S2_platforms[i].x1 = x1;
	level5S2_platforms[i].x2 = x1 + L5S2_SLAB_W;
	level5S2_platforms[i].y2 = top;
	level5S2_platforms[i].y1 = top - L5S2_SLAB_H;
}

static void resetPath5S2() {
	for (int k = 0; k < L5S2_PATH_SLABS; k++) {
		pathPos5S2[k] = k * (L5S2_PATH_LEN / (float)L5S2_PATH_SLABS);
		placePathSlab5S2(k, pathPos5S2[k]);
	}
}

// ---------------------------------------------------------------------------
// THE TOP LANE
//
// Three slabs appear at the top-isRight corner, glide left along the top of the
// screen as moving platforms, and are recycled to the top-isRight corner once
// they reach the top-left corner. The lane sits high enough that the player
// standing on the top of the staircase fits under it, and the slabs are
// spread wide so there is always a gap to jump up through.
// ---------------------------------------------------------------------------
#define L5S2_LANE_TOP    396     // top surface of every lane slab
#define L5S2_LANE_START  558     // left edge of a slab at the top-isRight corner
#define L5S2_LANE_END    20      // left edge at the top-left corner
#define L5S2_LANE_LEN    (L5S2_LANE_START - L5S2_LANE_END)
#define L5S2_LANE_SPEED  2.0f
#define L5S2_LANE_SLABS  3
#define L5S2_LANE_FIRST  5       // platform index of the first lane slab

static float lanePos5S2[L5S2_LANE_SLABS] = { 0, 179, 358 };

static void placeLaneSlab5S2(int slab, float s) {
	int i = L5S2_LANE_FIRST + slab;
	int x1 = L5S2_LANE_START - (int)s;
	level5S2_platforms[i].x1 = x1;
	level5S2_platforms[i].x2 = x1 + L5S2_SLAB_W;
	level5S2_platforms[i].y2 = L5S2_LANE_TOP;
	level5S2_platforms[i].y1 = L5S2_LANE_TOP - L5S2_SLAB_H;
}

static void resetLane5S2() {
	for (int k = 0; k < L5S2_LANE_SLABS; k++) {
		lanePos5S2[k] = k * (L5S2_LANE_LEN / (float)L5S2_LANE_SLABS);
		placeLaneSlab5S2(k, lanePos5S2[k]);
	}
}

// ---------------------------------------------------------------------------
// Slab speed.
// Level 4 runs at 2 * 1.2 = 2.4 px/frame. Level 5 is a touch quicker at
// 2 * 1.5 = 3.0 px/frame. The float speed + per-slab fractional carry from
// level4.h is kept so the motion stays smooth and the average speed is
// exact rather than being rounded away each frame (see slabStep5S2()).
// ---------------------------------------------------------------------------
#define SLAB_SPEED_SCALE5S2 1.5f
static float slabSpeed5S2[L5S2_PLATFORM_COUNT] = {
	0.0f,
	2 * SLAB_SPEED_SCALE5S2, 2 * SLAB_SPEED_SCALE5S2, 2 * SLAB_SPEED_SCALE5S2,
	2 * SLAB_SPEED_SCALE5S2, 2 * SLAB_SPEED_SCALE5S2, 2 * SLAB_SPEED_SCALE5S2,
	2 * SLAB_SPEED_SCALE5S2
};
static float slabCarry5S2[L5S2_PLATFORM_COUNT] = { 0.0f };
static int slabDirection5S2[L5S2_PLATFORM_COUNT] = { 0 };
static int slabDeltaX5S2[L5S2_PLATFORM_COUNT] = { 0 };
static int slabDeltaY5S2[L5S2_PLATFORM_COUNT] = { 0 };

static int slabTexture5S2 = 0;

#define LEVEL5S2_LEFT   20
#define LEVEL5S2_RIGHT  680
#define LEVEL5S2_BOTTOM 25
#define LEVEL5S2_TOP    480

// ===== Dragons =====
// Final round: 3 normal dragons, each kill pays out one golden ball, then the
// boss dragon arrives.
#define L5S2_DRAGON_COUNT   3
#define L5S2_KILLS_PER_BALL 1

// ===== NORMAL DRAGONS (sprites: Images/dragon/<n>.png) =====
// These are the normal flying dragons used by level 5 / level 5 sub-level 1.
// They are NOT the red boss. All 3 patrol the level and breathe fire.
// Frames 1..17 face left; frames 31..63 face right. Frames 18..30 are absent.
#define RD_LEFT_START   1
#define RD_LEFT_END     17
#define RD_RIGHT_START  31
#define RD_RIGHT_END    63
#define RD_TOTAL_FRAMES 65
#define RD_WALK_SPEED   2.0f
#define RD_ANIM_SPEED   3
#define RD_DRAW_W       90
#define RD_DRAW_H       70
#define RD_HALF_W       28
#define RD_HIT_H        55
#define RD_TOUCH_HALF_W 24
#define RD_MOUTH_DX     32
#define RD_MOUTH_Y      42
#define RD_MIN_CX       (LEVEL5S2_LEFT + 45)
#define RD_MAX_CX       (LEVEL5S2_RIGHT - 45)
#define RD_FIREBALL_UNITS 3
#define RD_FIREBALL_SPEED 5.5f

// ===== Dragon Health Bar Sprites =====
static int dragonHealthBarTextures5S2[6] = { 0 }; // 1.png (5 lives) .. 5.png (1 life)

// ===== Fireball Breath Attack (identical to level 4) =====
// Dragons throw a fireball whenever their flight animation reaches frame 16
// (last frame of the left-facing cycle) or frame 54 (mid isRight-facing
// cycle) - the "mouth open / breathing fire" poses.
#define FIREBALL_ANIM_FRAME_A5S2 16
#define FIREBALL_ANIM_FRAME_B5S2 54

#define MAX_FIREBALLS5S2 10
#define MAX_TRAIL_PARTICLES5S2 60
#define MAX_EXPLOSION_PARTICLES5S2 40

struct Fireball5S2 {
	int x, y;
	int width, height;
	float vx, vy;
	float rotation;
	bool active;
};

struct TrailParticle5S2 {
	float x, y;
	float radius;
	int alpha;
	bool active;
};

struct ExplosionParticle5S2 {
	float x, y;
	float vx, vy;
	float radius;
	int life;
	bool active;
};

static Fireball5S2 fireballs5S2[MAX_FIREBALLS5S2];
static TrailParticle5S2 trailParticles5S2[MAX_TRAIL_PARTICLES5S2];
static ExplosionParticle5S2 explosionParticles5S2[MAX_EXPLOSION_PARTICLES5S2];
static int fireballTexture5S2 = 0;

// Fireball hits needed to drop 1 zed, kept separate from the melee-touch
// counter so the two damage sources never interfere with each other.
static int fireballHitsToPlayer5S2 = 0;
static int fireballHitCooldown5S2 = 0;

// ===== Fighting & Combo Variables =====
#define F5S2_FIGHT_FRAMES     LASER_FRAMES
#define F5S2_FIGHT_FRAMES_ALL LASER_FRAMES_ALL
#define F5S2_FIGHT_ANIM_SPEED 7

static int fightingTextures5S2[F5S2_FIGHT_FRAMES_ALL + 1] = { 0 };
static bool isFighting5S2 = false;
static bool isLeftMouseDown5S2 = false;
static int currentPunchCombo5S2 = 0;
static int fightFrame5S2 = 1;
static int fightAnimTimer5S2 = 0;
static bool punchDamageDealt5S2 = false;

// ===== Game State Variables =====
static bool level5S2Complete = false;   // current sub-level's 3 balls collected
static bool isGameOver5S2 = false;
static bool isPaused5S2 = false;
static int hitCooldown5S2 = 0;
static int dragonHitsToPlayer5S2 = 0;

// ----- Win state. gameWon5S2 latches the instant the final round is
//       cleared, so the win screen triggers exactly once. -----
static bool gameWon5S2 = false;
static int winTexture5S2 = 0;

// ----- Intro transition (level4 completed -> level5S2 starts) -----
static bool isLevel5S2Transition = true;
static int level5S2TransitionCounter = 0;
static int level4CompletedTexture5S2 = 0;
static int level5S2StartsTexture5 = 0;

// ===== Player Lives, Energy & Points Variables =====
static int playerLives5S2 = 5;
static int& energyFrame5S2 = gEnergyFrame;
static int distanceMoved5S2 = 0;
static int energyTextures5S2[150] = { 0 };
static int& playerPoints5S2 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTexture5S2 = 0;
static int zedsIconTexture5S2 = 0;

// ===== Menu Textures & Bounding Boxes =====
static int resumeTexture5S2 = 0;
static int restartTexture5S2 = 0;
static int exitTexture5S2 = 0;

static Button pauseResumeBtn5S2 = { 260, 280, 440, 325 };
static Button pauseRestartBtn5S2 = { 260, 220, 440, 265 };
static Button pauseExitBtn5S2 = { 260, 160, 440, 205 };

// ===== Game Over Menu Textures & Bounding Boxes =====
static int totalPointsTexture5S2 = 0;
static int gameOverTexture5S2 = 0;
static int gameOverRestartTexture5S2 = 0;
static int gameOverExitTexture5S2 = 0;

static Button gameOverRestartBtn5S2 = { 240, 115, 460, 165 };
static Button gameOverExitBtn5S2 = { 275, 70, 425, 105 };

// ===== Win Screen Exit Button (reuses exitTexture5S2 / Images/exit.png) =====
static Button winExitBtn5S2 = { 260, 60, 440, 105 };

// ===== Gold Ball Collectibles, Portal & HUD =====
struct GoldBall5S2 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static GoldBall5S2 goldBalls5S2[3];
static int goldBallTexture5S2 = 0;
static int goldBallIconTextures5S2[5] = { 0 };
static int goldBallsCollected5S2 = 0;
static int dispearTexture5S2 = 0;

// Kill / reward bookkeeping. goldBallsSpawned5S2 is the single source of
// truth for "how many balls have been handed out this sub-level", so a
// dragon can never pay out twice and a 4th ball can never appear.
static int dragonsKilled5S2 = 0;
static int goldBallsSpawned5S2 = 0;

// ===== Blue Ball Collectibles & Points HUD =====
struct BlueBall5S2 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static BlueBall5S2 blueBalls5S2[9];
static int blueBallTexture5S2 = 0;
static int pointsTexture5S2 = 0;

struct PatrolDragon5S2 {
	int x, y;
	int width, height;
	float cx;
	bool facingRight;
	bool alive;
	bool rewarded;
	int health;
	int punchCount;
	int animFrame;
	int animTimer;
	int moveTimer;
	int moveDir;
	int fireCooldown;
	int flash;
	int minX, maxX;   // this dragon's own patrol lane, so the 3 dragons stay in 3 separate places
};

static PatrolDragon5S2 dragons5S2[L5S2_DRAGON_COUNT];
static int dragonTex5S2[RD_TOTAL_FRAMES] = { 0 };
static bool texturesLoaded5S2 = false;

// ---------------------------------------------------------------------------
// Texture loading
// ---------------------------------------------------------------------------
static void loadDragonHealthBarTextures5S2() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 5; i++) {
		sprintf(path, "Images/Dragonhealthbar/%d.png", i);
		dragonHealthBarTextures5S2[i] = iLoadImage(path);

		if (dragonHealthBarTextures5S2[i] <= 0) {
			sprintf(path, "../Images/Dragonhealthbar/%d.png", i);
			dragonHealthBarTextures5S2[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadFireballTexture5S2() {
	static bool loaded = false;
	if (loaded) return;
	fireballTexture5S2 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadFightingTextures5S2() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= F5S2_FIGHT_FRAMES_ALL; i++) {
		sprintf(path, "Images/PlayerLaser/shoot/%d.png", i);
		fightingTextures5S2[i] = iLoadImage(path);

		if (fightingTextures5S2[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/shoot/%d.png", i);
			fightingTextures5S2[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

// Laser attack frames: 1-10 face isRight, 11-20 are their mirrors (facing left).
static int getFightTextureIndex5S2(int frame, bool facingRight) {
	return laserTextureIndex(frame, facingRight);
}

static void loadTransitionTextures5S2() {
	static bool loaded = false;
	if (loaded) return;

	level4CompletedTexture5S2 = loadTex("Images/level4completed.png", "../Images/level4completed.png");
	level5S2StartsTexture5 = loadTex("Images/level5starts.png", "../Images/level5starts.png");

	loaded = true;
}

static void loadWinTexture5S2() {
	static bool loaded = false;
	if (loaded) return;

	winTexture5S2 = loadTex(IMG_LV5S2_WIN, "../Images/win.png");
	loaded = true;
}

static void loadZedsTextures5S2() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTexture5S2 = loadTex("Images/zeds.png", "../Images/zeds.png");

	zedsIconTexture5S2 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture5S2 <= 0) zedsIconTexture5S2 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture5S2 <= 0) zedsIconTexture5S2 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTextures5S2() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures5S2[i] = iLoadImage(path);

		if (energyTextures5S2[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures5S2[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadSlabTexture5S2() {
	static bool loaded = false;
	if (loaded) return;

	slabTexture5S2 = loadTex(IMG_LV5S2_SLAB, "../Images/slabsfinal.png");
	if (slabTexture5S2 <= 0) slabTexture5S2 = loadTex("Images/slabs.png", "../Images/slabs.png");
	loaded = true;
}

static void loadPauseTextures5S2() {
	static bool loaded = false;
	if (loaded) return;

	resumeTexture5S2 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture5S2 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture5S2 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTexture5S2 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTexture5S2 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture5S2 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture5S2 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadGoldBallTextures5S2() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTexture5S2 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture5S2 = loadTex("Images/dispear4.png", "../Images/dispear4.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures5S2[i] = iLoadImage(path);
		if (goldBallIconTextures5S2[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures5S2[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadBlueBallAndPointsTextures5S2() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTexture5S2 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture5S2 = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

// ===========================================================================
// FIREBALL BREATH ATTACK - identical to level4.h, only the suffix differs
// ===========================================================================
static void spawnExplosion5S2(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES5S2; j++) {
			if (!explosionParticles5S2[j].active) {
				explosionParticles5S2[j].x = x;
				explosionParticles5S2[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticles5S2[j].vx = cos(angle) * speed;
				explosionParticles5S2[j].vy = sin(angle) * speed;
				explosionParticles5S2[j].radius = 3.0f + (rand() % 3);
				explosionParticles5S2[j].life = 20 + (rand() % 10);
				explosionParticles5S2[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticle5S2(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES5S2; i++) {
		if (!trailParticles5S2[i].active) {
			trailParticles5S2[i].x = x + (rand() % 6 - 3);
			trailParticles5S2[i].y = y + (rand() % 6 - 3);
			trailParticles5S2[i].radius = 4.0f + (rand() % 4);
			trailParticles5S2[i].alpha = 255;
			trailParticles5S2[i].active = true;
			break;
		}
	}
}

static void spawnFireball5S2(int dragonIndex) {
	for (int j = 0; j < MAX_FIREBALLS5S2; j++) {
		if (!fireballs5S2[j].active) {
			fireballs5S2[j].x = dragons5S2[dragonIndex].x + dragons5S2[dragonIndex].width / 2 - 10;
			fireballs5S2[j].y = dragons5S2[dragonIndex].y + dragons5S2[dragonIndex].height / 2 - 10;
			fireballs5S2[j].width = 20; fireballs5S2[j].height = 20;

			float dx = (player.x + player.width / 2.0f) - (fireballs5S2[j].x + 10);
			float dy = (player.y + player.height / 2.0f) - (fireballs5S2[j].y + 10);
			float dist = sqrt(dx * dx + dy * dy);
			if (dist == 0) dist = 1;

			float speed = 7.0f;
			fireballs5S2[j].vx = (dx / dist) * speed;
			// Arc boost for a parabola instead of a flat, unavoidable line shot
			fireballs5S2[j].vy = (dy / dist) * speed + 4.5f;
			fireballs5S2[j].rotation = 0.0f;
			fireballs5S2[j].active = true;
			break;
		}
	}
}

static void updateFireballs5S2() {
	for (int i = 0; i < MAX_FIREBALLS5S2; i++) {
		if (!fireballs5S2[i].active) continue;

		int oldY = fireballs5S2[i].y;

		// Gravity
		fireballs5S2[i].vy -= 0.2f;

		fireballs5S2[i].x += fireballs5S2[i].vx;
		fireballs5S2[i].y += fireballs5S2[i].vy;
		fireballs5S2[i].rotation += fireballs5S2[i].vx * 6.0f;

		// Bounce off the ground
		if (fireballs5S2[i].y <= LEVEL5S2_BOTTOM) {
			fireballs5S2[i].y = LEVEL5S2_BOTTOM;
			fireballs5S2[i].vy = -fireballs5S2[i].vy * 0.5f;
			fireballs5S2[i].vx *= 0.8f;
			spawnExplosion5S2(fireballs5S2[i].x + 10, fireballs5S2[i].y + 10);
			if (fabs(fireballs5S2[i].vy) < 1.5f) {
				fireballs5S2[i].active = false;
			}
		}

		// Explode against the floating slabs / ground platform
		if (fireballs5S2[i].active && fireballs5S2[i].vy < 0) {
			for (int p = 0; p < L5S2_PLATFORM_COUNT; p++) {
				Platform plat = level5S2_platforms[p];

				bool withinX = (fireballs5S2[i].x + fireballs5S2[i].width > plat.x1) && (fireballs5S2[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballs5S2[i].y <= plat.y2) {
					fireballs5S2[i].y = plat.y2;
					spawnExplosion5S2(fireballs5S2[i].x + 10, fireballs5S2[i].y + 10);
					fireballs5S2[i].active = false;
					break;
				}
			}
		}

		if (fireballs5S2[i].x < -50 || fireballs5S2[i].x > 750) {
			fireballs5S2[i].active = false;
		}

		if (fireballs5S2[i].active)
			spawnTrailParticle5S2(fireballs5S2[i].x + 10, fireballs5S2[i].y + 10);
	}
}

static void updateTrailParticles5S2() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES5S2; i++) {
		if (!trailParticles5S2[i].active) continue;
		trailParticles5S2[i].alpha -= 15;
		trailParticles5S2[i].radius -= 0.2f;
		if (trailParticles5S2[i].alpha <= 0 || trailParticles5S2[i].radius <= 0) {
			trailParticles5S2[i].active = false;
		}
	}
}

static void updateExplosionParticles5S2() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5S2; i++) {
		if (!explosionParticles5S2[i].active) continue;
		explosionParticles5S2[i].x += explosionParticles5S2[i].vx;
		explosionParticles5S2[i].y += explosionParticles5S2[i].vy;
		explosionParticles5S2[i].vy -= 0.1f;
		explosionParticles5S2[i].life--;
		explosionParticles5S2[i].radius -= 0.15f;
		if (explosionParticles5S2[i].life <= 0 || explosionParticles5S2[i].radius <= 0) {
			explosionParticles5S2[i].active = false;
		}
	}
}

// 8 fireball hits to lose 1 zed, independent of the melee-touch counter.
static void checkFireballCollision5S2() {
	if (fireballHitCooldown5S2 > 0) {
		fireballHitCooldown5S2--;
		return;
	}

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS5S2; i++) {
		if (!fireballs5S2[i].active) continue;

		bool collideX = (px + pw > fireballs5S2[i].x) && (px < fireballs5S2[i].x + fireballs5S2[i].width);
		bool collideY = (py + ph > fireballs5S2[i].y) && (py < fireballs5S2[i].y + fireballs5S2[i].height);

		if (collideX && collideY) {
			fireballHitsToPlayer5S2++;

			if (fireballHitsToPlayer5S2 >= 8) {  // 8 fireball hits = 1 zed
				playerLives5S2--;
				fireballHitsToPlayer5S2 = 0;

				if (playerLives5S2 <= 0) {
					playerLives5S2 = 0;
					isGameOver5S2 = true;
				}
			}

			fireballHitCooldown5S2 = 30;
			spawnExplosion5S2(fireballs5S2[i].x + 10, fireballs5S2[i].y + 10);
			fireballs5S2[i].active = false;
			break;
		}
	}
}

static void drawFireballs5S2() {
	// 1. Trail
	for (int i = 0; i < MAX_TRAIL_PARTICLES5S2; i++) {
		if (!trailParticles5S2[i].active) continue;
		int alpha = trailParticles5S2[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticles5S2[i].x, trailParticles5S2[i].y, trailParticles5S2[i].radius, 20);
	}

	// 2. Explosions
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5S2; i++) {
		if (!explosionParticles5S2[i].active) continue;
		if (explosionParticles5S2[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticles5S2[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticles5S2[i].x, explosionParticles5S2[i].y, explosionParticles5S2[i].radius, 20);
	}

	// 3. Main fireballs
	for (int i = 0; i < MAX_FIREBALLS5S2; i++) {
		if (!fireballs5S2[i].active) continue;
		float cx = fireballs5S2[i].x + 10;
		float cy = fireballs5S2[i].y + 10;

		if (fireballTexture5S2 > 0) {
			iRotate(cx, cy, fireballs5S2[i].rotation);
			iShowImage(fireballs5S2[i].x, fireballs5S2[i].y, fireballs5S2[i].width, fireballs5S2[i].height, fireballTexture5S2);
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
// dragons pay out. spawnGoldBall5S2() always anchors the ball to a real
// platform (a slab top, or the ground) and tags it with that platform's
// index, so it rides the moving slab and can never end up floating in a
// spot the player cannot jump to. The platform and the x offset on it are
// random, so no two runs look the same.
// ---------------------------------------------------------------------------
static void spawnGoldBall5S2(int ballIndex) {
	if (ballIndex < 0 || ballIndex > 2) return;

	// 1 in 4 balls drops on the ground, the rest on a random slab.
	bool onGround = ((rand() % 4) == 0);

	if (onGround) {
		goldBalls5S2[ballIndex].x = 60 + (rand() % 540);
		goldBalls5S2[ballIndex].y = LEVEL5S2_BOTTOM + 5;
		goldBalls5S2[ballIndex].platformIndex = L5S2_GROUND;
		return;
	}

	int p = 1 + (rand() % 4);   // a staircase slab
	Platform pl = level5S2_platforms[p];

	int minX = pl.x1 + 15;
	int maxX = pl.x2 - 40;
	if (maxX <= minX) maxX = minX + 1;

	goldBalls5S2[ballIndex].x = minX + (rand() % (maxX - minX));
	goldBalls5S2[ballIndex].y = pl.y2 + 5;
	goldBalls5S2[ballIndex].platformIndex = p;
}

static void initGoldAndBlueBalls5S2(bool freshStart) {
	loadGoldBallTextures5S2();
	loadBlueBallAndPointsTextures5S2();

	goldBallsCollected5S2 = 0;
	goldBallsSpawned5S2 = 0;
	dragonsKilled5S2 = 0;
	if (freshStart) playerPoints5S2 = 0;

	// 1. GOLD BALLS - unchanged. Parked off-screen until a dragon pays out.
	for (int i = 0; i < 3; i++) {
		goldBalls5S2[i].width = 25;
		goldBalls5S2[i].height = 25;
		goldBalls5S2[i].x = -1000;
		goldBalls5S2[i].y = -1000;
		goldBalls5S2[i].collected = false;
		goldBalls5S2[i].platformIndex = L5S2_GROUND;
	}

	// 2. BLUE BALLS - one on every stair, two on lane slabs.
	int ballPlat[7] = { 1, 2, 3, 4, 5, 6, 7 };
	for (int b = 0; b < 7; b++) {
		int i = ballPlat[b];
		blueBalls5S2[b].width = 18;
		blueBalls5S2[b].height = 18;
		blueBalls5S2[b].x = level5S2_platforms[i].x1 + (L5S2_SLAB_W / 2) - 9;
		blueBalls5S2[b].y = level5S2_platforms[i].y2 + 5;
		blueBalls5S2[b].collected = false;
		blueBalls5S2[b].platformIndex = i;
	}

	// 3. THE LAST TWO float low over the ground, one standing jump away.
	int airX[2] = { 560, 620 };
	int airY[2] = { 100, 100 };
	for (int i = 0; i < 2; i++) {
		int b = 7 + i;
		blueBalls5S2[b].width = 18;
		blueBalls5S2[b].height = 18;
		blueBalls5S2[b].x = airX[i];
		blueBalls5S2[b].y = airY[i];
		blueBalls5S2[b].collected = false;
		blueBalls5S2[b].platformIndex = L5S2_GROUND;
	}
}

static void checkGoldBallCollision5S2() {
	checkGoldBallCollisionGeneric(goldBalls5S2, &goldBallsCollected5S2, &level5S2Complete);
}

static void checkBlueBallCollision5S2() {
	checkBlueBallCollisionGeneric(blueBalls5S2, &playerPoints5S2);
}

static void drawGoldBalls5S2() {
	drawGoldBallsGeneric(goldBalls5S2, goldBallTexture5S2);
}

static void drawBlueBalls5S2() {
	drawBlueBallsGeneric(blueBalls5S2, blueBallTexture5S2);
}

static void drawGoldBallHUD5S2() {
	drawGoldBallHUDGeneric(goldBallsCollected5S2, goldBallIconTextures5S2);
}

static void drawPointsHUD5S2() {
	drawPointsHUDGeneric(pointsTexture5S2, playerPoints5S2);
}

// ---------------------------------------------------------------------------
// No portal in the final round - the level ends in the win screen, so
// there is nothing to walk into and no isRight-edge opening.
// ---------------------------------------------------------------------------
static void drawPortal5S2() {
}

// ===========================================================================
// BOSS DRAGON  (final fight of level 5 / sub level 2)
//
// After the 3 normal dragons are dead and their 3 golden balls are in hand
// the Boss Dragon walks in from the isRight edge. It ALWAYS re-targets the
// player: it turns to face them, walks towards / away from them along the
// ground, then attacks in the player's direction:
//
//   BOSS_ARRIVE  -> walks in from the isRight edge
//   BOSS_IDLE    -> breathing, picks the next action
//   BOSS_MOVE    -> walks left/isRight (faces the player while walking)
//   BOSS_PREPARE -> attack wind-up   (sheet frames 5-7,  mouth starts to glow)
//   BOSS_ATTACK  -> fire breath      (sheet frames 8-19)
//                     far  : 2-3 aimed fireball projectiles leave the mouth
//                     close: a flame-cone hitbox in front of the mouth that
//                            hurts ONCE per attack, only on the big-flame frames
//   BOSS_RECOVER -> flame dies out   (sheet frames 22-28) then a cooldown
//   BOSS_HIT     -> short stagger when shot while walking / idle
//   BOSS_DEAD    -> collapse animation, then gameWon5S2 (final win)
//
// Sprites: Images/BossDragon/{idle,attack,hit,death,projectile}/  (see
// loadBossTextures5S2). Every file has a _r (facing isRight) and _l (facing
// left, mirrored) version on the SAME 320x200 canvas, so switching frames or
// turning around never changes size or position (no jitter).
//
// Tuning: everything is in the #defines below.
// ===========================================================================
#define BOSS5S2_MAX_HP          50      // laser hits to kill the boss (dragon = 10)
#define BOSS5S2_WALK_SPEED      1.6f    // px per tick
#define BOSS5S2_ARRIVE_SPEED    3.0f
#define BOSS5S2_COOLDOWN_MIN    50      // ticks between attacks (16 ms each)
#define BOSS5S2_COOLDOWN_RAND   50
#define BOSS5S2_MELEE_RANGE     170     // closer than this -> flame-cone attack
#define BOSS5S2_FIREBALL_SPEED  6.5f
#define BOSS5S2_FIREBALL_UNITS  4       // fireball hit = 4 of the 12 "hits per zed"
#define BOSS5S2_FLAME_UNITS     5       // flame cone  = 5 of the 12
#define BOSS5S2_TOUCH_UNITS     1       // body touch = 1 of the 8 needed to lose a zed

// Draw geometry. Source canvas is 320x200; the body's centre sits at x=124
// (facing isRight) and the feet 8 px above the canvas bottom.
#define BOSS5S2_DRAW_W          340
#define BOSS5S2_DRAW_H          100
#define BOSS5S2_BODY_CX_R       100     // red boss body centre from canvas left, facing right
#define BOSS5S2_BODY_CX_L       240     // red boss body centre from canvas left, facing left
#define BOSS5S2_FEET_PAD        4       // red boss canvas bottom is this far below the feet
#define BOSS5S2_MOUTH_DX        62      // mouth distance in front of body centre
#define BOSS5S2_MOUTH_Y         58      // mouth height above the canvas bottom
#define BOSS5S2_HALF_W          55      // body hitbox half width
#define BOSS5S2_HIT_H           78      // body hitbox height above the ground
#define BOSS5S2_MIN_CX          (LEVEL5S2_LEFT + 62)
#define BOSS5S2_MAX_CX          (LEVEL5S2_RIGHT - 62)

enum BossState5S2 {
	BOSS_NONE = 0, BOSS_ARRIVE, BOSS_IDLE, BOSS_MOVE, BOSS_PREPARE,
	BOSS_ATTACK, BOSS_RECOVER, BOSS_HIT, BOSS_DEAD
};

// frame counts / ticks per frame
#define BOSS_IDLE_FRAMES     4
#define BOSS_PREP_FRAMES     3
#define BOSS_FIRE_FRAMES     12
#define BOSS_RECOVER_FRAMES  7
#define BOSS_HIT_FRAMES      2
#define BOSS_DEATH_FRAMES    6
#define BOSS_PREP_TICKS      6
#define BOSS_FIRE_TICKS      3
#define BOSS_RECOVER_TICKS   4
#define BOSS_DEATH_TICKS     10
#define BOSS_HIT_STAGGER     12
#define BOSS_FLASH_TICKS     8

// Attack timeline inside BOSS_ATTACK (fire frame numbers 1..12)
#define BOSS_FLAME_FIRST     3       // flame-cone hitbox active on frames 3..8
#define BOSS_FLAME_LAST      8
#define BOSS_SHOT_FRAME_A    4       // fireballs leave the mouth on these frames
#define BOSS_SHOT_FRAME_B    7
#define BOSS_SHOT_FRAME_C    10

struct BossFire5S2 {
	float x, y;          // centre of the fireball head
	float vx, vy;
	int age;
	int animTimer, animFrame;
	int units;           // damage units (of 12 per zed)
	bool isSmall;          // dragon fireballs are drawn smaller than the boss'
	bool active;
};

struct Boss5S2 {
	float cx;            // world x of the body centre
	bool facingRight;
	BossState5S2 state;
	int hp;
	int frame, timer;    // animation frame / ticks spent on it
	int cooldown;        // ticks until the next attack is allowed
	int attackType;      // 0 = fireballs, 1 = flame cone
	bool coneHit;        // this attack has already hurt the player
	int shotsFired;
	int moveDir;         // -1 / 0 / +1 while BOSS_MOVE
	int moveTimer;
	int flash;           // hit-flash ticks left
	int staggerTimer;
	int deathHold;
	int bob;             // walk-bob phase
	bool active;         // boss has arrived in the level
};

#define MAX_BOSS_FIRE5S2 14
static Boss5S2 boss5S2;
static BossFire5S2 bossFire5S2[MAX_BOSS_FIRE5S2];
static int bossTex5S2[2][40] = { { 0 } };   // [facing 0=isRight 1=left][sprite id]
static int bossFireTex5S2[2][2] = { { 0 } };
static int bossBannerTimer5S2 = 0;

// sprite ids inside bossTex5S2
#define BT_IDLE   0                                  // 4
#define BT_PREP   (BT_IDLE + BOSS_IDLE_FRAMES)       // 3
#define BT_FIRE   (BT_PREP + BOSS_PREP_FRAMES)       // 12
#define BT_RECOV  (BT_FIRE + BOSS_FIRE_FRAMES)       // 7
#define BT_HIT    (BT_RECOV + BOSS_RECOVER_FRAMES)   // 2
#define BT_DEATH  (BT_HIT + BOSS_HIT_FRAMES)         // 6   -> 34 total

static int loadBossImage5S2(const char* folder, const char* name, int n, char side) {
	char path[160];
	sprintf(path, "Images/BossDragon/%s/%s_%d_%c.png", folder, name, n, side);
	int t = iLoadImage(path);
	if (t <= 0) {
		sprintf(path, "../Images/BossDragon/%s/%s_%d_%c.png", folder, name, n, side);
		t = iLoadImage(path);
	}
	return t;
}

static int loadRedBossImage5S2(const char* anim, int n, char side) {
	char path[160];
	sprintf(path, "Images/RedDragon/%s/%s_%d_%c.png", anim, anim, n, side);
	int t = iLoadImage(path);
	if (t <= 0) {
		sprintf(path, "../Images/RedDragon/%s/%s_%d_%c.png", anim, anim, n, side);
		t = iLoadImage(path);
	}
	return t;
}

static void loadBossTextures5S2() {
	static bool loaded = false;
	if (loaded) return;
	for (int s = 0; s < 2; s++) {
		char side = (s == 0) ? 'r' : 'l';
		// The RED DRAGON is the boss. Reuse its dedicated animations while
		// preserving the existing boss state machine.
		for (int i = 1; i <= BOSS_IDLE_FRAMES; i++)
			bossTex5S2[s][BT_IDLE + i - 1] = loadRedBossImage5S2("idle", i, side);
		for (int i = 1; i <= BOSS_PREP_FRAMES; i++)
			bossTex5S2[s][BT_PREP + i - 1] = loadRedBossImage5S2("claw", i, side);
		for (int i = 1; i <= BOSS_FIRE_FRAMES; i++) {
			int src = ((i - 1) % 8) + 1;
			bossTex5S2[s][BT_FIRE + i - 1] = loadRedBossImage5S2("fire", src, side);
		}
		for (int i = 1; i <= BOSS_RECOVER_FRAMES; i++) {
			int src = ((i - 1) % 6) + 1;
			bossTex5S2[s][BT_RECOV + i - 1] = loadRedBossImage5S2("idle", src, side);
		}
		for (int i = 1; i <= BOSS_HIT_FRAMES; i++)
			bossTex5S2[s][BT_HIT + i - 1] = loadRedBossImage5S2("hit", i, side);
		for (int i = 1; i <= BOSS_DEATH_FRAMES; i++)
			bossTex5S2[s][BT_DEATH + i - 1] = loadRedBossImage5S2("death", i, side);
		// Keep the existing fireball projectile sprites.
		for (int i = 1; i <= 2; i++)
			bossFireTex5S2[s][i - 1] = loadBossImage5S2("projectile", "fireball", i, side);
	}
	loaded = true;
}

static void resetBoss5S2() {
	memset(&boss5S2, 0, sizeof(boss5S2));
	boss5S2.state = BOSS_NONE;
	boss5S2.hp = BOSS5S2_MAX_HP;
	boss5S2.facingRight = false;
	for (int i = 0; i < MAX_BOSS_FIRE5S2; i++) bossFire5S2[i].active = false;
	bossBannerTimer5S2 = 0;
}

static float bossPlayerCX5S2() { return player.x + player.width / 2.0f; }

static void bossSetState5S2(BossState5S2 s) {
	boss5S2.state = s;
	boss5S2.frame = 0;
	boss5S2.timer = 0;
}

// The RED DRAGON boss enters from the right edge and moves RIGHT -> LEFT
// before the final fight begins.
static void spawnBoss5S2() {
	loadBossTextures5S2();
	resetBoss5S2();
	boss5S2.active = true;
	boss5S2.cx = (float)(LEVEL5S2_RIGHT + 170);
	boss5S2.facingRight = false; // faces left while entering from right to left
	boss5S2.cooldown = 60;
	bossSetState5S2(BOSS_ARRIVE);
	bossBannerTimer5S2 = 130;
}

static bool bossAlive5S2() {
	return boss5S2.active && boss5S2.state != BOSS_DEAD && boss5S2.state != BOSS_NONE;
}

// Body hitbox (game coordinates).
static void bossBodyBox5S2(int& x, int& y, int& w, int& h) {
	x = (int)(boss5S2.cx - BOSS5S2_HALF_W);
	w = BOSS5S2_HALF_W * 2;
	y = LEVEL5S2_BOTTOM;
	h = BOSS5S2_HIT_H;
}

static void bossMouth5S2(float& mx, float& my) {
	mx = boss5S2.cx + (boss5S2.facingRight ? BOSS5S2_MOUTH_DX : -BOSS5S2_MOUTH_DX);
	my = (float)(LEVEL5S2_BOTTOM - BOSS5S2_FEET_PAD + BOSS5S2_MOUTH_Y);
}

// Flame cone in front of the mouth (only used while the cone is "live").
static void bossConeBox5S2(int& x, int& y, int& w, int& h) {
	float mx, my; bossMouth5S2(mx, my);
	w = 105; h = 64;
	x = boss5S2.facingRight ? (int)mx - 15 : (int)mx - w + 15;
	y = (int)my - h / 2;
}

static void damagePlayerUnits5S2(int units) {
	if (fireballHitCooldown5S2 > 0) return;   // invulnerability window shared with the dragon fireballs
	fireballHitsToPlayer5S2 += units;
	while (fireballHitsToPlayer5S2 >= 8) {
		fireballHitsToPlayer5S2 -= 8;
		playerLives5S2--;
		if (playerLives5S2 <= 0) {
			playerLives5S2 = 0;
			isGameOver5S2 = true;
			break;
		}
	}
	fireballHitCooldown5S2 = 30;
}

// Fires one projectile from (mx,my) towards the player's CURRENT position.
static void spawnBossFireAt5S2(float mx, float my, bool isRight, int units, float speed, bool isSmall) {
	for (int i = 0; i < MAX_BOSS_FIRE5S2; i++) {
		if (bossFire5S2[i].active) continue;
		float tx = bossPlayerCX5S2();
		float ty = player.y + player.height * 0.5f;
		float dx = tx - mx, dy = ty - my;
		float dir = isRight ? 1.0f : -1.0f;
		// never shoot backwards: keep a minimum forward component
		if (dx * dir < 60.0f) dx = dir * 60.0f;
		float d = sqrt(dx * dx + dy * dy);
		if (d < 1.0f) d = 1.0f;
		bossFire5S2[i].x = mx;
		bossFire5S2[i].y = my;
		bossFire5S2[i].vx = dx / d * speed;
		bossFire5S2[i].vy = dy / d * speed;
		bossFire5S2[i].age = 0;
		bossFire5S2[i].animTimer = 0;
		bossFire5S2[i].animFrame = 0;
		bossFire5S2[i].units = units;
		bossFire5S2[i].isSmall = isSmall;
		bossFire5S2[i].active = true;
		return;
	}
}

static void bossShootFireball5S2() {
	float mx, my; bossMouth5S2(mx, my);
	spawnBossFireAt5S2(mx, my, boss5S2.facingRight, BOSS5S2_FIREBALL_UNITS, BOSS5S2_FIREBALL_SPEED, false);
}

static void updateBossFire5S2() {
	for (int i = 0; i < MAX_BOSS_FIRE5S2; i++) {
		BossFire5S2& f = bossFire5S2[i];
		if (!f.active) continue;

		f.x += f.vx;
		f.y += f.vy;
		f.age++;
		if (++f.animTimer >= 4) { f.animTimer = 0; f.animFrame ^= 1; }
		if ((f.age & 1) == 0) spawnTrailParticle5S2(f.x, f.y);

		// outside the arena / into the ground
		if (f.x < -80 || f.x > 780 || f.y > 520 || f.y < LEVEL5S2_BOTTOM - 5) {
			if (f.y < LEVEL5S2_BOTTOM + 10) spawnExplosion5S2(f.x, LEVEL5S2_BOTTOM + 5);
			f.active = false;
			continue;
		}

		// slabs are cover (skip while the ball is still leaving the mouth)
		if (f.age > 10) {
			bool hitSlab = false;
			for (int p = 1; p < L5S2_PLATFORM_COUNT; p++) {
				Platform pl = level5S2_platforms[p];
				if (f.x > pl.x1 && f.x < pl.x2 && f.y > pl.y1 && f.y < pl.y2) { hitSlab = true; break; }
			}
			if (hitSlab) { spawnExplosion5S2(f.x, f.y); f.active = false; continue; }
		}

		// player hit
		int px, py, pw, ph;
		getPlayerHitbox(px, py, pw, ph);
		bool hx = (f.x + 16 > px) && (f.x - 16 < px + pw);
		bool hy = (f.y + 13 > py) && (f.y - 13 < py + ph);
		if (hx && hy) {
			spawnExplosion5S2(f.x, f.y);
			f.active = false;
			damagePlayerUnits5S2(f.units);
		}
	}
}

static void bossStartAttack5S2() {
	float dist = fabsf(bossPlayerCX5S2() - boss5S2.cx);
	// close -> flame cone (melee), otherwise mostly fireballs
	if (dist <= BOSS5S2_MELEE_RANGE) boss5S2.attackType = (rand() % 100 < 75) ? 1 : 0;
	else                             boss5S2.attackType = (rand() % 100 < 15) ? 1 : 0;
	boss5S2.coneHit = false;
	boss5S2.shotsFired = 0;
	bossSetState5S2(BOSS_PREPARE);
}

static void bossFaceTarget5S2(float hysteresis) {
	float dx = bossPlayerCX5S2() - boss5S2.cx;
	if (dx > hysteresis) boss5S2.facingRight = true;
	else if (dx < -hysteresis) boss5S2.facingRight = false;
}

static void bossPickNextAction5S2() {
	float dist = fabsf(bossPlayerCX5S2() - boss5S2.cx);

	if (boss5S2.cooldown <= 0) { bossStartAttack5S2(); return; }

	// walk towards a far player, back away from a close one, otherwise pace
	int dir = 0;
	if (dist > 220) dir = (bossPlayerCX5S2() > boss5S2.cx) ? 1 : -1;
	else if (dist < 120) dir = (bossPlayerCX5S2() > boss5S2.cx) ? -1 : 1;
	else dir = (rand() % 2) ? 1 : -1;

	boss5S2.moveDir = dir;
	boss5S2.moveTimer = 25 + rand() % 35;
	bossSetState5S2(BOSS_MOVE);
}

static void updateBoss5S2() {
	updateBossFire5S2();
	if (!boss5S2.active) return;

	if (boss5S2.flash > 0) boss5S2.flash--;
	if (boss5S2.cooldown > 0) boss5S2.cooldown--;
	if (boss5S2.staggerTimer > 0) boss5S2.staggerTimer--;
	if (bossBannerTimer5S2 > 0) bossBannerTimer5S2--;

	switch (boss5S2.state) {

	case BOSS_ARRIVE: {
						  boss5S2.facingRight = false;
						  boss5S2.cx -= BOSS5S2_ARRIVE_SPEED;
						  if (++boss5S2.timer >= 6) { boss5S2.timer = 0; boss5S2.frame = (boss5S2.frame + 1) % 6; }
						  if (boss5S2.cx <= LEVEL5S2_RIGHT - 150) {
							  boss5S2.cx = (float)(LEVEL5S2_RIGHT - 150);
							  bossSetState5S2(BOSS_IDLE);
						  }
						  break;
	}

	case BOSS_IDLE: {
						bossFaceTarget5S2(8.0f);
						if (boss5S2.cooldown <= 0) { bossStartAttack5S2(); break; }   // ready -> attack at once
						if (++boss5S2.timer >= 8) { boss5S2.timer = 0; boss5S2.frame = (boss5S2.frame + 1) % 6; }
						if (++boss5S2.moveTimer >= 18) { boss5S2.moveTimer = 0; bossPickNextAction5S2(); }
						break;
	}

	case BOSS_MOVE: {
						bossFaceTarget5S2(14.0f);   // keeps looking at the player while it walks
						boss5S2.cx += boss5S2.moveDir * BOSS5S2_WALK_SPEED;
						bool wall = false;
						if (boss5S2.cx < BOSS5S2_MIN_CX) { boss5S2.cx = (float)BOSS5S2_MIN_CX; wall = true; }
						if (boss5S2.cx > BOSS5S2_MAX_CX) { boss5S2.cx = (float)BOSS5S2_MAX_CX; wall = true; }
						if (++boss5S2.timer >= 5) { boss5S2.timer = 0; boss5S2.frame = (boss5S2.frame + 1) % 6; }
						boss5S2.bob++;
						if (--boss5S2.moveTimer <= 0 || wall) {
							boss5S2.moveTimer = 0;
							if (boss5S2.cooldown <= 0) bossStartAttack5S2();
							else bossSetState5S2(BOSS_IDLE);
						}
						else if (boss5S2.cooldown <= 0) {
							bossStartAttack5S2();   // cooldown over -> attack straight from the walk
						}
						break;
	}

	case BOSS_PREPARE: {
						   bossFaceTarget5S2(0.0f);    // lock on to the player's side
						   if (++boss5S2.timer >= BOSS_PREP_TICKS) {
							   boss5S2.timer = 0;
							   if (++boss5S2.frame >= BOSS_PREP_FRAMES) {
								   bossFaceTarget5S2(0.0f);   // one last look before firing
								   bossSetState5S2(BOSS_ATTACK);
							   }
						   }
						   break;
	}

	case BOSS_ATTACK: {
						  int fr = boss5S2.frame + 1;   // 1..12

						  if (boss5S2.timer == 0) {     // the first tick of every frame
							  if (boss5S2.attackType == 0) {
								  if (fr == BOSS_SHOT_FRAME_A || fr == BOSS_SHOT_FRAME_B || fr == BOSS_SHOT_FRAME_C) {
									  // third shot only when the boss is angry (below half health)
									  if (fr != BOSS_SHOT_FRAME_C || boss5S2.hp < BOSS5S2_MAX_HP / 2) {
										  bossShootFireball5S2();
										  boss5S2.shotsFired++;
									  }
								  }
							  }
						  }

						  // flame cone: hurts once, only while the big flame frames are showing
						  if (boss5S2.attackType == 1 && !boss5S2.coneHit && fr >= BOSS_FLAME_FIRST && fr <= BOSS_FLAME_LAST) {
							  int cx, cy, cw, ch, px, py, pw, ph;
							  bossConeBox5S2(cx, cy, cw, ch);
							  getPlayerHitbox(px, py, pw, ph);
							  if ((px + pw > cx) && (px < cx + cw) && (py + ph > cy) && (py < cy + ch)) {
								  if (fireballHitCooldown5S2 <= 0) {
									  boss5S2.coneHit = true;
									  damagePlayerUnits5S2(BOSS5S2_FLAME_UNITS);
								  }
							  }
						  }

						  if (++boss5S2.timer >= BOSS_FIRE_TICKS) {
							  boss5S2.timer = 0;
							  if (++boss5S2.frame >= BOSS_FIRE_FRAMES) bossSetState5S2(BOSS_RECOVER);
						  }
						  break;
	}

	case BOSS_RECOVER: {
						   if (++boss5S2.timer >= BOSS_RECOVER_TICKS) {
							   boss5S2.timer = 0;
							   if (++boss5S2.frame >= BOSS_RECOVER_FRAMES) {
								   int cd = BOSS5S2_COOLDOWN_MIN + rand() % BOSS5S2_COOLDOWN_RAND;
								   if (boss5S2.hp < BOSS5S2_MAX_HP / 2) cd = cd * 7 / 10;   // enraged
								   boss5S2.cooldown = cd;
								   boss5S2.moveTimer = 0;
								   bossSetState5S2(BOSS_IDLE);
							   }
						   }
						   break;
	}

	case BOSS_HIT: {
					   // isSmall knock-back away from the player, then back to work
					   boss5S2.cx += (bossPlayerCX5S2() > boss5S2.cx ? -1.5f : 1.5f);
					   if (boss5S2.cx < BOSS5S2_MIN_CX) boss5S2.cx = (float)BOSS5S2_MIN_CX;
					   if (boss5S2.cx > BOSS5S2_MAX_CX) boss5S2.cx = (float)BOSS5S2_MAX_CX;
					   if (++boss5S2.timer >= BOSS_HIT_STAGGER) {
						   boss5S2.moveTimer = 0;
						   bossSetState5S2(BOSS_IDLE);
					   }
					   break;
	}

	case BOSS_DEAD: {
						if (boss5S2.frame < BOSS_DEATH_FRAMES - 1) {
							if (++boss5S2.timer >= BOSS_DEATH_TICKS) { boss5S2.timer = 0; boss5S2.frame++; }
						}
						else {
							boss5S2.deathHold++;
						}
						break;
	}

	default: break;
	}

	// body touch (uses the same cooldown / counter the normal dragons use)
	if (bossAlive5S2() && boss5S2.state != BOSS_ARRIVE && hitCooldown5S2 <= 0) {
		int bx, by, bw, bh;
		bossBodyBox5S2(bx, by, bw, bh);
		int px, py, pw, ph;
		getPlayerHitbox(px, py, pw, ph);
		if ((px + pw > bx) && (px < bx + bw) && (py + ph > by) && (py < by + bh)) {
			dragonHitsToPlayer5S2 += BOSS5S2_TOUCH_UNITS;
			if (dragonHitsToPlayer5S2 >= 8) {
				dragonHitsToPlayer5S2 = 0;
				playerLives5S2--;
				if (playerLives5S2 <= 0) { playerLives5S2 = 0; isGameOver5S2 = true; }
			}
			hitCooldown5S2 = 30;
		}
	}
}

// Called by the player's laser (through checkPlayerPunchDragonCollision5S2).
// One laser = one hit: the caller removes the laser when this returns true,
// so a single shot can never hit twice.
static bool bossHitByLaser5S2(int ax, int ay, int aw, int ah) {
	if (!boss5S2.active) return false;
	if (boss5S2.state == BOSS_DEAD || boss5S2.state == BOSS_NONE || boss5S2.state == BOSS_ARRIVE) return false;

	int bx, by, bw, bh;
	bossBodyBox5S2(bx, by, bw, bh);
	bool cx = (ax + aw > bx) && (ax < bx + bw);
	bool cy = (ay + ah > by) && (ay < by + bh);
	if (!(cx && cy)) return false;

	laserHitCenterX = bx + bw / 2;
	laserHitCenterY = by + bh / 2;
	laserHitCenterSet = true;

	boss5S2.hp--;
	boss5S2.flash = BOSS_FLASH_TICKS;

	if (boss5S2.hp <= 0) {
		boss5S2.hp = 0;
		for (int i = 0; i < MAX_BOSS_FIRE5S2; i++) bossFire5S2[i].active = false;
		bossSetState5S2(BOSS_DEAD);
		boss5S2.deathHold = 0;
		return true;
	}

	// stagger only when it isn't in the middle of an attack
	// (and never twice in a row, so a stream of shots can't stun-lock it)
	if ((boss5S2.state == BOSS_IDLE || boss5S2.state == BOSS_MOVE) && boss5S2.staggerTimer <= 0) {
		boss5S2.staggerTimer = 50;
		bossSetState5S2(BOSS_HIT);
	}
	return true;
}

static bool bossDeathFinished5S2() {
	return boss5S2.active && boss5S2.state == BOSS_DEAD &&
		boss5S2.frame >= BOSS_DEATH_FRAMES - 1 && boss5S2.deathHold >= 60;
}

static int bossCurrentSprite5S2() {
	switch (boss5S2.state) {
	case BOSS_ARRIVE:
	case BOSS_IDLE:
	case BOSS_MOVE: {
						static const int pingPong[6] = { 0, 1, 2, 3, 2, 1 };
						return BT_IDLE + pingPong[boss5S2.frame % 6];
	}
	case BOSS_PREPARE:  return BT_PREP + boss5S2.frame;
	case BOSS_ATTACK:   return BT_FIRE + boss5S2.frame;
	case BOSS_RECOVER:  return BT_RECOV + boss5S2.frame;
	case BOSS_HIT:      return BT_HIT + ((boss5S2.timer < BOSS_HIT_STAGGER / 2) ? 0 : 1);
	case BOSS_DEAD:     return BT_DEATH + boss5S2.frame;
	default:            return BT_IDLE;
	}
}

static void drawBoss5S2() {
	if (!boss5S2.active) return;
	int side = boss5S2.facingRight ? 0 : 1;
	int spr = bossCurrentSprite5S2();

	// Hit reaction: the body flashes white while idle/walking/recovering,
	// and every state gets a tiny shake so a hit is always visible (the
	// attack frames keep their flames, so they are not swapped out).
	int shake = 0;
	if (boss5S2.flash > 0 && boss5S2.state != BOSS_DEAD) {
		shake = (boss5S2.flash & 1) ? 2 : -2;
		if (boss5S2.state == BOSS_IDLE || boss5S2.state == BOSS_MOVE || boss5S2.state == BOSS_RECOVER)
			spr = BT_HIT + ((boss5S2.flash > BOSS_FLASH_TICKS / 2) ? 0 : 1);
	}

	int bob = 0;
	if (boss5S2.state == BOSS_MOVE) { static const int b[4] = { 0, 1, 2, 1 }; bob = b[(boss5S2.bob / 4) & 3]; }

	int drawX = (int)(boss5S2.cx - (boss5S2.facingRight ? BOSS5S2_BODY_CX_R : BOSS5S2_BODY_CX_L)) + shake;
	int drawY = LEVEL5S2_BOTTOM - BOSS5S2_FEET_PAD + bob;

	int tex = bossTex5S2[side][spr];
	if (tex > 0) iShowImage(drawX, drawY, BOSS5S2_DRAW_W, BOSS5S2_DRAW_H, tex);
	else {
		iSetColor(160, 20, 20);
		iFilledRectangle((int)boss5S2.cx - BOSS5S2_HALF_W, LEVEL5S2_BOTTOM, BOSS5S2_HALF_W * 2, BOSS5S2_HIT_H);
	}

	// Health bar - the SAME Dragonhealthbar sprites the normal dragons use
	// (1.png = full ... 5.png = nearly dead), just drawn larger.
	if (boss5S2.state != BOSS_DEAD && boss5S2.state != BOSS_ARRIVE) {
		int idx = 1 + (int)((BOSS5S2_MAX_HP - boss5S2.hp) * 5 / BOSS5S2_MAX_HP);
		if (idx < 1) idx = 1;
		if (idx > 5) idx = 5;
		int bx = (int)boss5S2.cx - 55;
		int by = LEVEL5S2_BOTTOM - BOSS5S2_FEET_PAD + BOSS5S2_DRAW_H + 2;
		if (dragonHealthBarTextures5S2[idx] > 0)
			iShowImage(bx, by, 110, 16, dragonHealthBarTextures5S2[idx]);
	}
}

static void drawBossFire5S2() {
	for (int i = 0; i < MAX_BOSS_FIRE5S2; i++) {
		BossFire5S2& f = bossFire5S2[i];
		if (!f.active) continue;
		bool isRight = f.vx >= 0;
		int tex = bossFireTex5S2[isRight ? 0 : 1][f.animFrame];
		const int w = f.isSmall ? 70 : 92, h = f.isSmall ? 38 : 50;
		if (tex > 0) {
			// the fireball head is at the leading end of the sprite
			int x = (int)(isRight ? f.x - w * 0.80f : f.x - w * 0.20f);
			iShowImage(x, (int)f.y - h / 2, w, h, tex);
		}
		else {
			iSetColor(255, 120, 0);  iFilledCircle(f.x, f.y, 12, 30);
			iSetColor(255, 240, 150); iFilledCircle(f.x, f.y, 6, 30);
		}
	}
}

static void drawBossBanner5S2() {
	// Text intentionally suppressed: the red dragon's entrance speaks for itself.
	return;
}

// ---------------------------------------------------------------------------
// Dragons
// ---------------------------------------------------------------------------
static void loadDragonTextures5S2() {
	if (texturesLoaded5S2) return;
	char path[160];
	for (int i = 1; i <= RD_RIGHT_END; i++) {
		if (i >= 18 && i <= 30) { dragonTex5S2[i] = 0; continue; }
		sprintf(path, "Images/dragon/%d.png", i);
		dragonTex5S2[i] = iLoadImage(path);
		if (dragonTex5S2[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTex5S2[i] = iLoadImage(path);
		}
	}
	texturesLoaded5S2 = true;
}

static void dragonSync5S2(int i) {
	PatrolDragon5S2& d = dragons5S2[i];
	d.width = RD_DRAW_W;
	d.height = RD_DRAW_H;
	d.x = (int)(d.cx - RD_DRAW_W * 0.5f);
	// d.y is set once in setupDragon5S2 to the 3 distinct heights (95/235/365)
	// and each dragon only patrols horizontally, so it must not be touched
	// here - forcing it back to LEVEL5S2_BOTTOM+35 every frame is what was
	// collapsing all 3 dragons onto the same y position.
}

static void setupDragon5S2(int i, int cx, int y, int minX, int maxX) {
	PatrolDragon5S2& d = dragons5S2[i];
	memset(&d, 0, sizeof(d));
	d.cx = (float)cx;
	d.y = y;
	d.facingRight = false;
	d.alive = true;
	d.rewarded = false;
	d.health = 5;
	d.punchCount = 0;
	d.animFrame = RD_LEFT_START;
	d.animTimer = 0;
	d.moveTimer = 0;
	d.moveDir = -1;
	d.fireCooldown = 80 + rand() % 80;
	d.minX = minX;
	d.maxX = maxX;
	dragonSync5S2(i);
}

static void initDragon5S2() {
	loadDragonTextures5S2();
	loadDragonHealthBarTextures5S2();
	// Use the original non-red dragon sprites from Images/dragon/.
	// The three patrol lanes are spread across the sub-level, non-overlapping,
	// so the 3 dragons always appear in 3 distinct places on the ground.
	setupDragon5S2(0, 150, 95, LEVEL5S2_LEFT, 280);
	setupDragon5S2(1, 380, 235, 300, 460);
	setupDragon5S2(2, 560, 365, 480, LEVEL5S2_RIGHT);
}

static void stepDragonFrame5S2(int i) {
	PatrolDragon5S2& d = dragons5S2[i];
	if (d.facingRight) {
		d.animFrame++;
		if (d.animFrame < RD_RIGHT_START || d.animFrame > RD_RIGHT_END) d.animFrame = RD_RIGHT_START;
	}
	else {
		d.animFrame++;
		if (d.animFrame < RD_LEFT_START || d.animFrame > RD_LEFT_END) d.animFrame = RD_LEFT_START;
	}
}

static void setDragonDirection5S2(int i, bool right) {
	PatrolDragon5S2& d = dragons5S2[i];
	if (d.facingRight == right) return;
	d.facingRight = right;
	d.animFrame = right ? RD_RIGHT_START : RD_LEFT_START;
}

static void dragonMouth5S2(int i, float& mx, float& my) {
	PatrolDragon5S2& d = dragons5S2[i];
	mx = d.cx + (d.facingRight ? RD_MOUTH_DX : -RD_MOUTH_DX);
	my = d.y + RD_MOUTH_Y;
}

static void updateDragon5S2() {
	for (int i = 0; i < L5S2_DRAGON_COUNT; i++) {
		PatrolDragon5S2& d = dragons5S2[i];
		if (!d.alive) continue;
		if (d.flash > 0) d.flash--;
		if (d.fireCooldown > 0) d.fireCooldown--;

		// Simple horizontal patrol: each dragon stays within its own lane
		// (d.minX..d.maxX) so the 3 dragons stay in 3 separate places.
		if (d.moveDir < 0) {
			d.cx -= RD_WALK_SPEED;
			if (d.cx <= d.minX) { d.cx = (float)d.minX; d.moveDir = 1; setDragonDirection5S2(i, true); }
		}
		else {
			d.cx += RD_WALK_SPEED;
			if (d.cx >= d.maxX) { d.cx = (float)d.maxX; d.moveDir = -1; setDragonDirection5S2(i, false); }
		}

		if (++d.animTimer >= RD_ANIM_SPEED) {
			d.animTimer = 0;
			stepDragonFrame5S2(i);
			// Keep the original fire-breath timing from the existing dragon sprites.
			if ((d.animFrame == FIREBALL_ANIM_FRAME_A5S2 || d.animFrame == FIREBALL_ANIM_FRAME_B5S2) && d.fireCooldown <= 0) {
				// These 3 patrol dragons are the normal dragons, not the
				// boss - they should throw the regular fireball.png
				// projectile (spawnFireball5S2), not the boss's fire sprite.
				// 20% fewer fireballs overall: 1-in-5 chance to skip the throw.
				if (rand() % 5 != 0) spawnFireball5S2(i);
				d.fireCooldown = 90 + rand() % 80;
			}
		}
		dragonSync5S2(i);
	}
}

static void rewardDragonKill5S2(int i) {
	if (dragons5S2[i].rewarded) return;
	dragons5S2[i].rewarded = true;
	dragonsKilled5S2++;
	if (dragonsKilled5S2 % L5S2_KILLS_PER_BALL != 0) return;
	if (goldBallsSpawned5S2 >= 3) return;
	spawnGoldBall5S2(goldBallsSpawned5S2);
	goldBallsSpawned5S2++;
}

static bool checkPlayerPunchDragonCollision5S2(int attackX, int attackY, int attackW, int attackH) {
	if (bossHitByLaser5S2(attackX, attackY, attackW, attackH)) return true;
	for (int i = 0; i < L5S2_DRAGON_COUNT; i++) {
		PatrolDragon5S2& d = dragons5S2[i];
		if (!d.alive) continue;
		int dx = d.x + 10, dy = d.y + 12, dw = d.width - 20, dh = d.height - 20;
		bool collideX = (attackX + attackW > dx) && (attackX < dx + dw);
		bool collideY = (attackY + attackH > dy) && (attackY < dy + dh);
		if (collideX && collideY) {
			d.punchCount++;
			d.flash = 8;
			if (d.punchCount >= 2) {
				d.punchCount = 0;
				d.health--;
				if (d.health <= 0) {
					d.health = 0;
					d.alive = false;
					rewardDragonKill5S2(i);
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

static void drawDragon5S2() {
	for (int i = 0; i < L5S2_DRAGON_COUNT; i++) {
		const PatrolDragon5S2& d = dragons5S2[i];
		if (!d.alive) continue;
		int tex = dragonTex5S2[d.animFrame];
		if (tex > 0) {
			iShowImage(d.x, d.y, d.width, d.height, tex);
		}
		else {
			iSetColor(180, 80, 100);
			iFilledCircle((int)d.cx, d.y + 30, 25, 30);
		}
		int idx = 6 - d.health;
		if (idx >= 1 && idx <= 5 && dragonHealthBarTextures5S2[idx] > 0)
			iShowImage((int)d.cx - 30, d.y + d.height + 4, 60, 9, dragonHealthBarTextures5S2[idx]);
	}
}

static void checkDragonPlayerCollision5S2() {
	if (hitCooldown5S2 > 0) { hitCooldown5S2--; return; }
	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);
	for (int i = 0; i < L5S2_DRAGON_COUNT; i++) {
		if (!dragons5S2[i].alive) continue;
		int hx = dragons5S2[i].x + 12;
		int hy = dragons5S2[i].y + 15;
		int hw = dragons5S2[i].width - 24;
		int hh = dragons5S2[i].height - 25;
		bool collideX = (px + pw > hx) && (px < hx + hw);
		bool collideY = (py + ph > hy) && (py < hy + hh);
		if (collideX && collideY) {
			dragonHitsToPlayer5S2++;
			if (dragonHitsToPlayer5S2 >= 8) {
				playerLives5S2--;
				dragonHitsToPlayer5S2 = 0;
				if (playerLives5S2 <= 0) { playerLives5S2 = 0; isGameOver5S2 = true; }
			}
			hitCooldown5S2 = 30;
			break;
		}
	}
}

static void drawHUD5S2() {
	drawHUDGeneric(zedsLabelTexture5S2, zedsIconTexture5S2, playerLives5S2, energyFrame5S2, energyTextures5S2);
}

// ---------------------------------------------------------------------------
// Stage / level setup
//
// initLevel5S2Stage() resets the playfield. Zeds, points and energy are
// handled by initLevel5S2() so a stage reset never silently refills them.
// ---------------------------------------------------------------------------
static void initLevel5S2Stage(bool freshStart) {
	for (int i = 0; i < L5S2_PLATFORM_COUNT; i++) {
		level5S2_platforms[i] = level5S2_platformStart[i];
		slabDirection5S2[i] = L5S2_START_DIR[i];
		slabCarry5S2[i] = 0.0f;
		slabDeltaX5S2[i] = 0;
		slabDeltaY5S2[i] = 0;
	}

	// The staircase always restarts at the bottom of its travel.
	resetPath5S2();
	resetLane5S2();

	initPlayer(40, LEVEL5S2_BOTTOM);
	initDragon5S2();
	initGoldAndBlueBalls5S2(freshStart);
	resetBoss5S2();

	level5S2Complete = false;
	hitCooldown5S2 = 0;
	dragonHitsToPlayer5S2 = 0;

	currentPunchCombo5S2 = 0;
	isFighting5S2 = false;
	punchDamageDealt5S2 = false;
	laserResetAll();

	for (int i = 0; i < MAX_FIREBALLS5S2; i++) fireballs5S2[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES5S2; i++) trailParticles5S2[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5S2; i++) explosionParticles5S2[i].active = false;
	fireballHitsToPlayer5S2 = 0;
	fireballHitCooldown5S2 = 0;

	distanceMoved5S2 = 0;
}

static void initLevel5S2(bool freshStart = true) {
	loadSlabTexture5S2();
	loadPauseTextures5S2();
	loadZedsTextures5S2();
	loadEnergyTextures5S2();
	loadFightingTextures5S2();
	loadFireballTexture5S2();
	loadBossTextures5S2();
	loadTransitionTextures5S2();
	loadWinTexture5S2();

	initLevel5S2Stage(freshStart);

	isGameOver5S2 = false;
	isPaused5S2 = false;

	gameWon5S2 = false;

	isLevel5S2Transition = true;
	level5S2TransitionCounter = 0;

	playerLives5S2 = 5;
	// Energy and zeds refill at the start of every level.
	energyFrame5S2 = 1;
}

static void drawLevel5S2Background() {
	iShowBMP(0, 0, IMG_LV5S2_BG);
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
// Which slab (if any) the player is riding this frame, exactly the way a
// gold/blue ball is pinned to its platformIndex. Set in updateMovingSlabs5S2
// and consumed by resolvePlatformCollision5S2.
static int slabRiddenByPlayer5S2 = -1;

static bool playerOverlapsSlab5S2(const Platform& p) {
	return (player.x + player.width > p.x1) && (player.x < p.x2) &&
		(player.y + player.height > p.y1) && (player.y < p.y2);
}

// Vertical: land on tops, bump heads on undersides. Uses where each slab
// WAS before it moved this frame, so a rising slab cannot slip past a
// falling player and a falling slab cannot swallow a rising one.
static void resolvePlatformCollision5S2() {
	int oldY = player.y - player.velocityY;
	int oldTop = oldY + player.height;

	if (player.y <= LEVEL5S2_BOTTOM) {
		player.y = LEVEL5S2_BOTTOM;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	// Same fix as 5S1: if the player is already riding a slab (carried by
	// its exact delta this frame, same as a ball tied to a platformIndex),
	// snap straight to its current top instead of re-deriving "was I above
	// it" from last frame - that derivation only works for a rising slab,
	// so on a DESCENDING stair it silently drops the player out of sync.
	if (slabRiddenByPlayer5S2 != -1 && player.velocityY <= 0) {
		const Platform& rp = level5S2_platforms[slabRiddenByPlayer5S2];
		bool stillWithinX = (player.x + player.width > rp.x1) && (player.x < rp.x2);
		if (stillWithinX) {
			player.y = rp.y2;
			player.velocityY = 0;
			player.onGround = true;
			player.jumping = false;
			return;
		}
	}

	player.onGround = false;

	int bestTop = -100000;
	int bestBottom = 100000;
	for (int i = 1; i < L5S2_PLATFORM_COUNT; i++) {
		Platform p = level5S2_platforms[i];
		bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
		if (!withinX) continue;

		int prevTop = p.y2 - slabDeltaY5S2[i];
		int prevBottom = p.y1 - slabDeltaY5S2[i];

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
static void resolveSideCollision5S2(int oldX) {
	if (player.x == oldX) return;
	bool movingRight = (player.x > oldX);

	for (int i = 1; i < L5S2_PLATFORM_COUNT; i++) {
		Platform p = level5S2_platforms[i];
		if (!playerOverlapsSlab5S2(p)) continue;

		if (movingRight) player.x = p.x1 - player.width;
		else player.x = p.x2;
	}
}

static bool playerStandingOnPlatform5S2(int index) {
	Platform p = level5S2_platforms[index];
	bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
	return withinX && player.onGround && player.velocityY <= 0 && abs(player.y - p.y2) <= 2;
}

// A slab that has moved into the player displaces them by the smallest
// distance along a direction that is actually free (never into the floor,
// the walls or the ceiling), so the player is slid out instead of squeezed.
static void pushPlayerOutOfSlabs5S2() {
	for (int pass = 0; pass < 3; pass++) {
		bool any = false;

		for (int i = 1; i < L5S2_PLATFORM_COUNT; i++) {
			Platform p = level5S2_platforms[i];
			if (!playerOverlapsSlab5S2(p)) continue;
			any = true;

			int up = p.y2 - player.y;
			int down = (player.y + player.height) - p.y1;
			int left = (player.x + player.width) - p.x1;
			int isRight = p.x2 - player.x;

			if (player.y - down < LEVEL5S2_BOTTOM) down = 100000;
			if (player.y + up + player.height > LEVEL5S2_TOP) up = 100000;
			if (player.x - left < LEVEL5S2_LEFT) left = 100000;
			if (player.x + isRight + player.width > LEVEL5S2_RIGHT) isRight = 100000;

			int best = up;
			int dir = 0;
			if (down < best) { best = down; dir = 1; }
			if (left < best) { best = left; dir = 2; }
			if (isRight < best) { best = isRight; dir = 3; }

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
static void applySlabMotion5S2(const bool* playerOnSlab) {
	for (int i = 1; i < L5S2_PLATFORM_COUNT; i++) {
		if (playerOnSlab[i]) {
			player.x += slabDeltaX5S2[i];
			player.y += slabDeltaY5S2[i];
		}
	}

	// ...and slide them clear if a slab has moved into them.
	pushPlayerOutOfSlabs5S2();

	for (int i = 0; i < 3; i++) {
		if (!goldBalls5S2[i].collected) {
			int p = goldBalls5S2[i].platformIndex;
			goldBalls5S2[i].x += slabDeltaX5S2[p];
			goldBalls5S2[i].y += slabDeltaY5S2[p];
		}
	}

	for (int i = 0; i < 9; i++) {
		if (!blueBalls5S2[i].collected) {
			int p = blueBalls5S2[i].platformIndex;
			blueBalls5S2[i].x += slabDeltaX5S2[p];
			blueBalls5S2[i].y += slabDeltaY5S2[p];
		}
	}
}


static void updateMovingSlabs5S2() {
	for (int i = 0; i < L5S2_PLATFORM_COUNT; i++) {
		slabDeltaX5S2[i] = 0;
		slabDeltaY5S2[i] = 0;
	}

	bool playerOnSlab[L5S2_PLATFORM_COUNT] = { false };
	slabRiddenByPlayer5S2 = -1;
	for (int i = 1; i < L5S2_PLATFORM_COUNT; i++) {
		playerOnSlab[i] = playerStandingOnPlatform5S2(i);
		if (playerOnSlab[i]) slabRiddenByPlayer5S2 = i;
	}

	// Diagonal staircase (slabs 1..4)
	for (int k = 0; k < L5S2_PATH_SLABS; k++) {
		int i = k + 1;
		int oldX = level5S2_platforms[i].x1;
		int oldTop = level5S2_platforms[i].y2;

		float s = pathPos5S2[k] + L5S2_PATH_SPEED;
		// Always loop back to the start, carrying whatever (and whoever) is
		// riding it - a conveyor, not a lift that waits at the top.
		if (s >= L5S2_PATH_LEN) {
			s -= L5S2_PATH_LEN;
		}
		pathPos5S2[k] = s;
		placePathSlab5S2(k, s);

		slabDeltaX5S2[i] = level5S2_platforms[i].x1 - oldX;
		slabDeltaY5S2[i] = level5S2_platforms[i].y2 - oldTop;
	}

	// Top lane (slabs 5..7), isRight -> left
	for (int k = 0; k < L5S2_LANE_SLABS; k++) {
		int i = L5S2_LANE_FIRST + k;
		int oldX = level5S2_platforms[i].x1;

		float s = lanePos5S2[k] + L5S2_LANE_SPEED;
		if (s >= L5S2_LANE_LEN) {
			s -= L5S2_LANE_LEN;
		}
		lanePos5S2[k] = s;
		placeLaneSlab5S2(k, s);

		slabDeltaX5S2[i] = level5S2_platforms[i].x1 - oldX;
	}

	applySlabMotion5S2(playerOnSlab);
}

static void drawPlatforms5S2() {
	if (slabTexture5S2 <= 0) return;

	for (int i = 1; i < L5S2_PLATFORM_COUNT; i++) {
		int width = level5S2_platforms[i].x2 - level5S2_platforms[i].x1;
		int height = level5S2_platforms[i].y2 - level5S2_platforms[i].y1;
		iShowImage(level5S2_platforms[i].x1, level5S2_platforms[i].y1, width, height, slabTexture5S2);
	}
}

// ---------------------------------------------------------------------------
// BOSS + WIN CHECK
//
// 3 dragons dead AND 3 golden balls collected does NOT end the level - it
// only brings in the boss. The final win is granted ONLY once the boss is
// dead and its collapse animation has played. gameWon5S2 latches, so this
// fires exactly once.
// ---------------------------------------------------------------------------
static void checkBossAndWin5S2() {
	if (gameWon5S2) return;

	if (!boss5S2.active) {
		if (dragonsKilled5S2 >= L5S2_DRAGON_COUNT && goldBallsCollected5S2 >= 3)
			spawnBoss5S2();
		return;
	}

	if (bossDeathFinished5S2()) {
		gameWon5S2 = true;
		loadWinTexture5S2();
	}
}

// ============================================================================
// FIGHTING / LASER INPUT
// These entry points are required by updateLevel5S2() and iMain.cpp.
// ============================================================================
static void updateFightingAnimation5S2() {
	laserFightStep(isFighting5S2, isLeftMouseDown5S2, fightFrame5S2, fightAnimTimer5S2, punchDamageDealt5S2);
	// Move the laser projectiles and apply their hit test to normal dragons
	// and the red boss through checkPlayerPunchDragonCollision5S2().
	laserUpdateProjectiles(checkPlayerPunchDragonCollision5S2);
}

static void handleMouseClickLevel5S2(int button, int state) {
	if (button != GLUT_LEFT_BUTTON) return;

	if (state == GLUT_DOWN) {
		isLeftMouseDown5S2 = true;

		if (!isFighting5S2 && !isPaused5S2 && !isGameOver5S2 &&
			!isLevel5S2Transition && !gameWon5S2) {
			isFighting5S2 = true;
			fightAnimTimer5S2 = 0;
			punchDamageDealt5S2 = false;
			fightFrame5S2 = 1;
		}
	}
	else if (state == GLUT_UP) {
		isLeftMouseDown5S2 = false;
	}
}

static void updateLevel5S2() {
	// Intro banner
	if (isLevel5S2Transition) {
		level5S2TransitionCounter++;
		if (level5S2TransitionCounter >= 100) {
			isLevel5S2Transition = false;
		}
		return;
	}

	// Once the win screen is up the whole level freezes.
	if (isGameOver5S2 || isPaused5S2 || gameWon5S2) return;

	updateMovingSlabs5S2();

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

	if (player.x < LEVEL5S2_LEFT) player.x = LEVEL5S2_LEFT;

	if (player.x + player.width > LEVEL5S2_RIGHT) {
		// No exit on the isRight edge in the final round.
		player.x = LEVEL5S2_RIGHT - player.width;
	}

	// Slabs are solid boxes - stop the player walking through one.
	resolveSideCollision5S2(oldX);

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved5S2 += moveDist;
		while (distanceMoved5S2 >= 150) {
			distanceMoved5S2 -= 150;
			if (energyFrame5S2 < 145) {
				energyFrame5S2++;
			}
		}
		if (energyFrame5S2 >= 145) {
			isGameOver5S2 = true;
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

	resolvePlatformCollision5S2();

	if (player.y + player.height > LEVEL5S2_TOP) {
		player.y = LEVEL5S2_TOP - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateFightingAnimation5S2();
	updateDragon5S2();
	updateBoss5S2();
	checkDragonPlayerCollision5S2();

	// --- FIREBALL & PARTICLE UPDATES (same order as level 4) ---
	updateFireballs5S2();
	updateTrailParticles5S2();
	updateExplosionParticles5S2();
	checkFireballCollision5S2();

	checkGoldBallCollision5S2();
	checkBlueBallCollision5S2();
	checkBossAndWin5S2();
}

static void drawPauseMenu5S2() {
	drawPauseMenuGeneric(resumeTexture5S2, pauseResumeBtn5S2, restartTexture5S2, pauseRestartBtn5S2, exitTexture5S2, pauseExitBtn5S2);
}

static void drawLevel5S2() {
	drawLevel5S2Background();

	if (isLevel5S2Transition) {
		// Banner text removed - just hold on the background during the
		// intro pause.
		return;
	}

	drawPlatforms5S2();
	drawDragon5S2();
	drawBoss5S2();
	drawGoldBalls5S2();
	drawBlueBalls5S2();
	drawPortal5S2();

	if (!(isFighting5S2 && drawLaserFightSprite(fightingTextures5S2, fightFrame5S2, player.facingRight))) {
		drawGunPlayerBody();
	}
	drawLaserProjectiles();

	drawFireballs5S2(); // trails, explosions and the fireballs themselves
	drawBossFire5S2();
	drawBossBanner5S2();

	drawHUD5S2();
	drawGoldBallHUD5S2();
	drawPointsHUD5S2();

	if (gameWon5S2) {
		// FINAL SCREEN - total points box + win.png (already says
		// "CONGRATULATIONS YOU WON" itself), centered in the middle of the
		// screen, with the exit button beneath it (previously there was no
		// way off this screen at all).
		drawTotalPointsBoxGeneric(totalPointsTexture5S2, playerPoints5S2, 405);
		if (winTexture5S2 > 0) {
			iShowImage(80, 164, 540, 171, winTexture5S2);
		}

		if (exitTexture5S2 > 0) {
			iShowImage(winExitBtn5S2.x1, winExitBtn5S2.y1, 180, 45, exitTexture5S2);
		}
		return;
	}

	if (isPaused5S2) {
		drawPauseMenu5S2();
	}
	else if (isGameOver5S2) {
		drawTotalPointsBoxGeneric(totalPointsTexture5S2, playerPoints5S2, 405);

		if (gameOverTexture5S2 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTexture5S2);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTexture5S2 > 0)
			iShowImage(gameOverRestartBtn5S2.x1, gameOverRestartBtn5S2.y1, 220, 50, gameOverRestartTexture5S2);

		if (gameOverExitTexture5S2 > 0)
			iShowImage(gameOverExitBtn5S2.x1, gameOverExitBtn5S2.y1, 150, 35, gameOverExitTexture5S2);
	}
}

#endif