#ifndef LEVEL5S1_H
#define LEVEL5S1_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "iGraphics.h"
#include "level_common.h"
#include "game_objects.h"
#include "player.h"

// ============================================================================
// level5S1_sub1.h - LEVEL 5, SUB LEVEL 1
//
// Architecture, naming and mechanics are taken straight from level4.h. The
// whole level still exposes exactly the same entry points level 4 does, so
// iMain.cpp needs no structural change:
//     initLevel5S1(bool freshStart), updateLevel5S1(), drawLevel5S1(),
//     handleMouseClickLevel5S1(), isPaused5S1, isGameOver5S1, the pause buttons
//     and the game-over buttons.
//
// Level 5 is played as three stages, one file each:
//     level5S1.h -> level5S1_sub1.h  (this file)  -> level5S1_sub2.h
//
// Rules are identical to level 5 / level 4: 3 dragons, killing 1 dragon
// drops 1 golden ball at a random reachable spot, and collecting all 3
// opens the portal on the right edge. Walking into it sets enterLevel5S1Sub2
// so iMain.cpp can hand over to the final round with points, energy and
// Zeds intact.
//
// The staircase is a DIFFERENT layout from level5S1.h: it descends left to
// right, so the climb runs right-to-left, and the three extra slabs sit in
// their own free bands. No slab shares a position with any other stage.
//
// There is NO boss here - only the normal Images/dragon sprites are used.
// Nothing from Images/boss_sprites or Images/dragon_boss is loaded.
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

#define IMG_LV5S1_BG      "Images/lv5_bg.bmp"
#define IMG_LV5S1_SLAB    "Images/slabsfinal.png"

// ---------------------------------------------------------------------------
// PLATFORMS
//
// 5 climbing slabs + 2 extra slabs + the fixed ground.
//
// This stage is the timing climb: the five slabs do NOT move as one block.
// Each one bobs up and down inside its own vertical band, and the bands
// step upwards left to right, so the player has to hop from slab to slab
// and read the movement rather than just walking up.
//
// The bands are set so the climb works at EVERY phase: from any slab's
// lowest top to the next slab's highest top is only 95 px, well inside
// the player's jump, so you can never get stranded waiting.
//
// Slabs are SOLID on every side now, so you cannot slide through one -
// you land on it or you are stopped by it.
//
// Slabs are 102 x 21. Five in a row span 510 px, leaving a clear strip of
// ground (x 20..140) with nothing overhead - that is where the player
// spawns. Every slab also sits at least 80 px up, above the 75 px a
// standing player occupies, so the ground is ALWAYS a free corridor and
// the player can never be walled into a pocket with no way back out.
// ---------------------------------------------------------------------------
#define L5S1_PLATFORM_COUNT 8

#define L5S1_GROUND  0
#define L5S1_STAIR1  1
#define L5S1_STAIR2  2
#define L5S1_STAIR3  3
#define L5S1_STAIR4  4
#define L5S1_STAIR5  5
#define L5S1_EXTRA_A 6
#define L5S1_EXTRA_B 7

#define L5S1_SLAB_W 102
#define L5S1_SLAB_H 21

// A lip this low is climbed automatically instead of stopping the player.
#define L5S1_STEP_UP 22

// Axis each platform travels along.
#define L5S1_AXIS_NONE 0
#define L5S1_AXIS_X    1
#define L5S1_AXIS_Y    2

static Platform level5S1_platforms[L5S1_PLATFORM_COUNT] = {
	{ 20, 20, 680, 20 },      // 0 ground (fixed, never drawn)
	{ 30, 110, 132, 131 },    // 1..4 conveyor stairs, 120 px apart (18 px gap
	{ 150, 152, 252, 173 },   //      between neighbours); positions are set by
	{ 270, 194, 372, 215 },   //      resetPath5S1 / updateMovingSlabs5S1
	{ 390, 236, 492, 257 },
	{ 616, 323, 680, 344 },   // 5 FIXED ledge, reached from the last stair
	{ 500, 379, 602, 400 },   // 6 FIXED summit ledge
	{ 398, 379, 498, 400 }    // 7 FIXED summit ledge (walk on from ledge 6)
};

static const Platform level5S1_platformStart[L5S1_PLATFORM_COUNT] = {
	{ 20, 20, 680, 70 },
	{ 30, 110, 132, 131 },
	{ 150, 152, 252, 173 },
	{ 270, 194, 372, 215 },
	{ 390, 236, 492, 257 },
	{ 616, 323, 680, 344 },
	{ 500, 379, 602, 400 },
	{ 398, 379, 498, 400 }
};

static const int L5S1_START_DIR[L5S1_PLATFORM_COUNT] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// ---------------------------------------------------------------------------
// THE LOOPING STAIRCASE
//
// Four slabs, 120 px apart with a clear gap between them, ride one diagonal line from the bottom-left up to the top-right
// (5 px sideways for every 2 px up). They are evenly spaced, so it always
// reads as a staircase climbing towards the summit ledges. When a slab
// reaches the top-right end it goes back to the bottom-left start and the
// loop repeats. A slab someone is standing on simply waits at the end until
// they step off, so the player is never dropped or teleported.
// ---------------------------------------------------------------------------
#define L5S1_PATH_X0    30      // left edge of a slab at the start of the path
#define L5S1_PATH_Y0    131     // top surface of a slab at the start of the path
#define L5S1_PATH_LEN   480     // sideways length of the path
#define L5S1_PATH_SPEED 2.0f    // px per frame along the path (x); y is 0.35 of it
#define L5S1_PATH_SLABS 4

static float pathPos5S1[L5S1_PATH_SLABS] = { 0, 120, 240, 360 };

static void placePathSlab5S1(int slab, float s) {
	int i = slab + 1;
	int x1 = L5S1_PATH_X0 + (int)s;
	int top = L5S1_PATH_Y0 + (int)(s * 0.35f);
	level5S1_platforms[i].x1 = x1;
	level5S1_platforms[i].x2 = x1 + L5S1_SLAB_W;
	level5S1_platforms[i].y2 = top;
	level5S1_platforms[i].y1 = top - L5S1_SLAB_H;
}

static void resetPath5S1() {
	for (int k = 0; k < L5S1_PATH_SLABS; k++) {
		pathPos5S1[k] = k * (L5S1_PATH_LEN / (float)L5S1_PATH_SLABS);
		placePathSlab5S1(k, pathPos5S1[k]);
	}
}

// ---------------------------------------------------------------------------
// Slab speed.
// Level 4 runs at 2 * 1.2 = 2.4 px/frame. Level 5 is a touch quicker at
// 2 * 1.5 = 3.0 px/frame. The float speed + per-slab fractional carry from
// level4.h is kept so the motion stays smooth and the average speed is
// exact rather than being rounded away each frame (see slabStep5S1()).
// ---------------------------------------------------------------------------
#define SLAB_SPEED_SCALE5S1 1.5f
static float slabSpeed5S1[L5S1_PLATFORM_COUNT] = {
	0.0f,
	2 * SLAB_SPEED_SCALE5S1, 2 * SLAB_SPEED_SCALE5S1, 2 * SLAB_SPEED_SCALE5S1,
	2 * SLAB_SPEED_SCALE5S1, 2 * SLAB_SPEED_SCALE5S1, 2 * SLAB_SPEED_SCALE5S1,
	2 * SLAB_SPEED_SCALE5S1
};
static float slabCarry5S1[L5S1_PLATFORM_COUNT] = { 0.0f };
static int slabDirection5S1[L5S1_PLATFORM_COUNT] = { 0 };
static int slabDeltaX5S1[L5S1_PLATFORM_COUNT] = { 0 };
static int slabDeltaY5S1[L5S1_PLATFORM_COUNT] = { 0 };

// Which slab (if any) the player is riding this frame, exactly the way a
// gold/blue ball is pinned to its platformIndex. Set in updateMovingSlabs5S1
// and consumed by resolvePlatformCollision5S1 so a rider never falls out of
// sync with the stair the way a fixed-tolerance top/bottom scan can.
static int slabRiddenByPlayer5S1 = -1;

static int slabTexture5S1 = 0;

#define LEVEL5S1_LEFT   20
#define LEVEL5S1_RIGHT  680
#define LEVEL5S1_BOTTOM 25
#define LEVEL5S1_TOP    480

// ===== Dragons =====
// Exactly level 4's reward rule: 3 dragons, every kill drops 1 golden ball.
#define L5S1_DRAGON_COUNT   3
#define L5S1_KILLS_PER_BALL 1

// ===== Dragon Frame Boundaries =====
#define D5S1_LEFT_START   1
#define D5S1_LEFT_END     17
#define D5S1_RIGHT_START  31
#define D5S1_RIGHT_END    63
#define D5S1_TOTAL_FRAMES 65

#define D5S1_SPEED        2
#define D5S1_ANIM_SPEED   3

// ===== Dragon Health Bar Sprites =====
static int dragonHealthBarTextures5S1[6] = { 0 }; // 1.png (5 lives) .. 5.png (1 life)

// ===== Fireball Breath Attack (identical to level 4) =====
// Dragons throw a fireball whenever their flight animation reaches frame 16
// (last frame of the left-facing cycle) or frame 54 (mid right-facing
// cycle) - the "mouth open / breathing fire" poses.
#define FIREBALL_ANIM_FRAME_A5S1 16
#define FIREBALL_ANIM_FRAME_B5S1 54

#define MAX_FIREBALLS5S1 10
#define MAX_TRAIL_PARTICLES5S1 60
#define MAX_EXPLOSION_PARTICLES5S1 40

struct Fireball5S1 {
	int x, y;
	int width, height;
	float vx, vy;
	float rotation;
	bool active;
};

struct TrailParticle5S1 {
	float x, y;
	float radius;
	int alpha;
	bool active;
};

struct ExplosionParticle5S1 {
	float x, y;
	float vx, vy;
	float radius;
	int life;
	bool active;
};

static Fireball5S1 fireballs5S1[MAX_FIREBALLS5S1];
static TrailParticle5S1 trailParticles5S1[MAX_TRAIL_PARTICLES5S1];
static ExplosionParticle5S1 explosionParticles5S1[MAX_EXPLOSION_PARTICLES5S1];
static int fireballTexture5S1 = 0;

// Fireball hits needed to drop 1 zed, kept separate from the melee-touch
// counter so the two damage sources never interfere with each other.
static int fireballHitsToPlayer5S1 = 0;
static int fireballHitCooldown5S1 = 0;

// ===== Fighting & Combo Variables =====
#define F5S1_FIGHT_FRAMES     LASER_FRAMES
#define F5S1_FIGHT_FRAMES_ALL LASER_FRAMES_ALL
#define F5S1_FIGHT_ANIM_SPEED 7

static int fightingTextures5S1[F5S1_FIGHT_FRAMES_ALL + 1] = { 0 };
static bool isFighting5S1 = false;
static bool isLeftMouseDown5S1 = false;
static int currentPunchCombo5S1 = 0;
static int fightFrame5S1 = 1;
static int fightAnimTimer5S1 = 0;
static bool punchDamageDealt5S1 = false;

// ===== Game State Variables =====
static bool level5S1Complete = false;   // current sub-level's 3 balls collected
static bool isGameOver5S1 = false;
static bool isPaused5S1 = false;
static int hitCooldown5S1 = 0;
static int dragonHitsToPlayer5S1 = 0;

// ----- Exit transition (Level 5 cleared -> Level 5 Sub 2 starts).
//       Counter driven, no iSetTimer, so the handover can never double-fire
//       or leave a paused timer id behind. -----
static bool enterLevel5Sub2 = false;
static bool showLevel5S1ExitTransition = false;
static int level5S1ExitCounter = 0;

// ----- Intro transition (level4 completed -> level5S1 starts) -----
static bool isLevel5S1Transition = true;
static int level5S1TransitionCounter = 0;
static int level4CompletedTexture5S1 = 0;
static int level5S1StartsTexture5 = 0;

// ===== Player Lives, Energy & Points Variables =====
static int playerLives5S1 = 5;
static int& energyFrame5S1 = gEnergyFrame;
static int distanceMoved5S1 = 0;
static int energyTextures5S1[150] = { 0 };
static int& playerPoints5S1 = gPlayerPoints;

// ===== Zeds (Life) Textures =====
static int zedsLabelTexture5S1 = 0;
static int zedsIconTexture5S1 = 0;

// ===== Menu Textures & Bounding Boxes =====
static int resumeTexture5S1 = 0;
static int restartTexture5S1 = 0;
static int exitTexture5S1 = 0;

static Button pauseResumeBtn5S1 = { 260, 280, 440, 325 };
static Button pauseRestartBtn5S1 = { 260, 220, 440, 265 };
static Button pauseExitBtn5S1 = { 260, 160, 440, 205 };

// ===== Game Over Menu Textures & Bounding Boxes =====
static int totalPointsTexture5S1 = 0;
static int gameOverTexture5S1 = 0;
static int gameOverRestartTexture5S1 = 0;
static int gameOverExitTexture5S1 = 0;

static Button gameOverRestartBtn5S1 = { 240, 115, 460, 165 };
static Button gameOverExitBtn5S1 = { 275, 70, 425, 105 };

// ===== Gold Ball Collectibles, Portal & HUD =====
struct GoldBall5S1 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static GoldBall5S1 goldBalls5S1[3];
static int goldBallTexture5S1 = 0;
static int goldBallIconTextures5S1[5] = { 0 };
static int goldBallsCollected5S1 = 0;
static int dispearTexture5S1 = 0;

// Kill / reward bookkeeping. goldBallsSpawned5S1 is the single source of
// truth for "how many balls have been handed out this sub-level", so a
// dragon can never pay out twice and a 4th ball can never appear.
static int dragonsKilled5S1 = 0;
static int goldBallsSpawned5S1 = 0;

// ===== Blue Ball Collectibles & Points HUD =====
struct BlueBall5S1 {
	int x, y;
	int width, height;
	bool collected;
	int platformIndex;
};

static BlueBall5S1 blueBalls5S1[9];
static int blueBallTexture5S1 = 0;
static int pointsTexture5S1 = 0;

struct PatrolDragon5S1 {
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

static PatrolDragon5S1 dragons5S1[L5S1_DRAGON_COUNT];

static int dragonTextures5S1[D5S1_TOTAL_FRAMES];
static bool texturesLoaded5S1 = false;

// ---------------------------------------------------------------------------
// Texture loading
// ---------------------------------------------------------------------------
static void loadDragonHealthBarTextures5S1() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 5; i++) {
		sprintf(path, "Images/Dragonhealthbar/%d.png", i);
		dragonHealthBarTextures5S1[i] = iLoadImage(path);

		if (dragonHealthBarTextures5S1[i] <= 0) {
			sprintf(path, "../Images/Dragonhealthbar/%d.png", i);
			dragonHealthBarTextures5S1[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

static void loadDragonTextures5S1() {
	if (texturesLoaded5S1) return;

	char path[128];
	for (int i = 1; i <= D5S1_RIGHT_END; i++) {
		if (i >= 18 && i <= 30) {
			dragonTextures5S1[i] = 0;
			continue;
		}

		sprintf(path, "Images/dragon/%d.png", i);
		dragonTextures5S1[i] = iLoadImage(path);

		if (dragonTextures5S1[i] <= 0) {
			sprintf(path, "../Images/dragon/%d.png", i);
			dragonTextures5S1[i] = iLoadImage(path);
		}
	}

	texturesLoaded5S1 = true;
}

static void loadFireballTexture5S1() {
	static bool loaded = false;
	if (loaded) return;
	fireballTexture5S1 = loadTex("Images/fireball.png", "../Images/fireball.png");
	loaded = true;
}

static void loadFightingTextures5S1() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= F5S1_FIGHT_FRAMES_ALL; i++) {
		sprintf(path, "Images/PlayerLaser/shoot/%d.png", i);
		fightingTextures5S1[i] = iLoadImage(path);

		if (fightingTextures5S1[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/shoot/%d.png", i);
			fightingTextures5S1[i] = iLoadImage(path);
		}
	}
	loaded = true;
}

// Laser attack frames: 1-10 face right, 11-20 are their mirrors (facing left).
static int getFightTextureIndex5S1(int frame, bool facingRight) {
	return laserTextureIndex(frame, facingRight);
}

static void loadTransitionTextures5S1() {
	static bool loaded = false;
	if (loaded) return;

	level4CompletedTexture5S1 = loadTex("Images/level4completed.png", "../Images/level4completed.png");
	level5S1StartsTexture5 = loadTex("Images/level5starts.png", "../Images/level5starts.png");

	loaded = true;
}

static void loadZedsTextures5S1() {
	static bool loaded = false;
	if (loaded) return;

	zedsLabelTexture5S1 = loadTex("Images/zeds.png", "../Images/zeds.png");

	zedsIconTexture5S1 = loadTex("Images/zedslives.png", "../Images/zedslives.png");
	if (zedsIconTexture5S1 <= 0) zedsIconTexture5S1 = iLoadImage("Images/zedsicon.png");
	if (zedsIconTexture5S1 <= 0) zedsIconTexture5S1 = iLoadImage("../Images/zedsicon.png");

	loaded = true;
}

static void loadEnergyTextures5S1() {
	static bool loaded = false;
	if (loaded) return;

	char path[128];
	for (int i = 1; i <= 145; i++) {
		sprintf(path, "Images/energyicon/%d.png", i);
		energyTextures5S1[i] = iLoadImage(path);

		if (energyTextures5S1[i] <= 0) {
			sprintf(path, "../Images/energyicon/%d.png", i);
			energyTextures5S1[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadSlabTexture5S1() {
	static bool loaded = false;
	if (loaded) return;

	slabTexture5S1 = loadTex(IMG_LV5S1_SLAB, "../Images/slabsfinal.png");
	if (slabTexture5S1 <= 0) slabTexture5S1 = loadTex("Images/slabs.png", "../Images/slabs.png");
	loaded = true;
}

static void loadPauseTextures5S1() {
	static bool loaded = false;
	if (loaded) return;

	resumeTexture5S1 = loadTex("Images/resume.png", "../Images/resume.png");
	restartTexture5S1 = loadTex("Images/restart.png", "../Images/restart.png");
	exitTexture5S1 = loadTex("Images/exit.png", "../Images/exit.png");
	totalPointsTexture5S1 = loadTex("Images/totalpoints.png", "../Images/totalpoints.png");
	gameOverTexture5S1 = loadTex("Images/gameover.png", "../Images/gameover.png");
	gameOverRestartTexture5S1 = loadTex("Images/gameoverRestart.png", "../Images/gameoverRestart.png");
	gameOverExitTexture5S1 = loadTex("Images/gameoverExit.png", "../Images/gameoverExit.png");

	loaded = true;
}

static void loadGoldBallTextures5S1() {
	static bool loaded = false;
	if (loaded) return;

	goldBallTexture5S1 = loadTex("Images/goldball.png", "../Images/goldball.png");
	dispearTexture5S1 = loadTex("Images/dispear4.png", "../Images/dispear4.png");

	char path[128];
	for (int i = 1; i <= 4; i++) {
		sprintf(path, "Images/goldenballicon/%d.png", i);
		goldBallIconTextures5S1[i] = iLoadImage(path);
		if (goldBallIconTextures5S1[i] <= 0) {
			sprintf(path, "../Images/goldenballicon/%d.png", i);
			goldBallIconTextures5S1[i] = iLoadImage(path);
		}
	}

	loaded = true;
}

static void loadBlueBallAndPointsTextures5S1() {
	static bool loaded = false;
	if (loaded) return;

	blueBallTexture5S1 = loadTex("Images/blueball.png", "../Images/blueball.png");
	pointsTexture5S1 = loadTex("Images/points.png", "../Images/points.png");

	loaded = true;
}

// ===========================================================================
// FIREBALL BREATH ATTACK - identical to level4.h, only the suffix differs
// ===========================================================================
static void spawnExplosion5S1(float x, float y) {
	for (int k = 0; k < 8; k++) {
		for (int j = 0; j < MAX_EXPLOSION_PARTICLES5S1; j++) {
			if (!explosionParticles5S1[j].active) {
				explosionParticles5S1[j].x = x;
				explosionParticles5S1[j].y = y;
				float angle = (rand() % 360) * 3.14159f / 180.0f;
				float speed = 2.0f + (rand() % 3);
				explosionParticles5S1[j].vx = cos(angle) * speed;
				explosionParticles5S1[j].vy = sin(angle) * speed;
				explosionParticles5S1[j].radius = 3.0f + (rand() % 3);
				explosionParticles5S1[j].life = 20 + (rand() % 10);
				explosionParticles5S1[j].active = true;
				break;
			}
		}
	}
}

static void spawnTrailParticle5S1(float x, float y) {
	for (int i = 0; i < MAX_TRAIL_PARTICLES5S1; i++) {
		if (!trailParticles5S1[i].active) {
			trailParticles5S1[i].x = x + (rand() % 6 - 3);
			trailParticles5S1[i].y = y + (rand() % 6 - 3);
			trailParticles5S1[i].radius = 4.0f + (rand() % 4);
			trailParticles5S1[i].alpha = 255;
			trailParticles5S1[i].active = true;
			break;
		}
	}
}

static void spawnFireball5S1(int dragonIndex) {
	for (int j = 0; j < MAX_FIREBALLS5S1; j++) {
		if (!fireballs5S1[j].active) {
			fireballs5S1[j].x = dragons5S1[dragonIndex].x + dragons5S1[dragonIndex].width / 2 - 10;
			fireballs5S1[j].y = dragons5S1[dragonIndex].y + dragons5S1[dragonIndex].height / 2 - 10;
			fireballs5S1[j].width = 20; fireballs5S1[j].height = 20;

			float dx = (player.x + player.width / 2.0f) - (fireballs5S1[j].x + 10);
			float dy = (player.y + player.height / 2.0f) - (fireballs5S1[j].y + 10);
			float dist = sqrt(dx * dx + dy * dy);
			if (dist == 0) dist = 1;

			float speed = 7.0f;
			fireballs5S1[j].vx = (dx / dist) * speed;
			// Arc boost for a parabola instead of a flat, unavoidable line shot
			fireballs5S1[j].vy = (dy / dist) * speed + 4.5f;
			fireballs5S1[j].rotation = 0.0f;
			fireballs5S1[j].active = true;
			break;
		}
	}
}

static void updateFireballs5S1() {
	for (int i = 0; i < MAX_FIREBALLS5S1; i++) {
		if (!fireballs5S1[i].active) continue;

		int oldY = fireballs5S1[i].y;

		// Gravity
		fireballs5S1[i].vy -= 0.2f;

		fireballs5S1[i].x += fireballs5S1[i].vx;
		fireballs5S1[i].y += fireballs5S1[i].vy;
		fireballs5S1[i].rotation += fireballs5S1[i].vx * 6.0f;

		// Bounce off the ground
		if (fireballs5S1[i].y <= LEVEL5S1_BOTTOM) {
			fireballs5S1[i].y = LEVEL5S1_BOTTOM;
			fireballs5S1[i].vy = -fireballs5S1[i].vy * 0.5f;
			fireballs5S1[i].vx *= 0.8f;
			spawnExplosion5S1(fireballs5S1[i].x + 10, fireballs5S1[i].y + 10);
			if (fabs(fireballs5S1[i].vy) < 1.5f) {
				fireballs5S1[i].active = false;
			}
		}

		// Explode against the floating slabs / ground platform
		if (fireballs5S1[i].active && fireballs5S1[i].vy < 0) {
			for (int p = 0; p < L5S1_PLATFORM_COUNT; p++) {
				Platform plat = level5S1_platforms[p];

				bool withinX = (fireballs5S1[i].x + fireballs5S1[i].width > plat.x1) && (fireballs5S1[i].x < plat.x2);
				if (!withinX) continue;

				if (oldY >= plat.y2 && fireballs5S1[i].y <= plat.y2) {
					fireballs5S1[i].y = plat.y2;
					spawnExplosion5S1(fireballs5S1[i].x + 10, fireballs5S1[i].y + 10);
					fireballs5S1[i].active = false;
					break;
				}
			}
		}

		if (fireballs5S1[i].x < -50 || fireballs5S1[i].x > 750) {
			fireballs5S1[i].active = false;
		}

		if (fireballs5S1[i].active)
			spawnTrailParticle5S1(fireballs5S1[i].x + 10, fireballs5S1[i].y + 10);
	}
}

static void updateTrailParticles5S1() {
	for (int i = 0; i < MAX_TRAIL_PARTICLES5S1; i++) {
		if (!trailParticles5S1[i].active) continue;
		trailParticles5S1[i].alpha -= 15;
		trailParticles5S1[i].radius -= 0.2f;
		if (trailParticles5S1[i].alpha <= 0 || trailParticles5S1[i].radius <= 0) {
			trailParticles5S1[i].active = false;
		}
	}
}

static void updateExplosionParticles5S1() {
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5S1; i++) {
		if (!explosionParticles5S1[i].active) continue;
		explosionParticles5S1[i].x += explosionParticles5S1[i].vx;
		explosionParticles5S1[i].y += explosionParticles5S1[i].vy;
		explosionParticles5S1[i].vy -= 0.1f;
		explosionParticles5S1[i].life--;
		explosionParticles5S1[i].radius -= 0.15f;
		if (explosionParticles5S1[i].life <= 0 || explosionParticles5S1[i].radius <= 0) {
			explosionParticles5S1[i].active = false;
		}
	}
}

// 8 fireball hits to lose 1 zed, independent of the melee-touch counter.
static void checkFireballCollision5S1() {
	if (fireballHitCooldown5S1 > 0) {
		fireballHitCooldown5S1--;
		return;
	}

	int px, py, pw, ph;
	getPlayerHitbox(px, py, pw, ph);

	for (int i = 0; i < MAX_FIREBALLS5S1; i++) {
		if (!fireballs5S1[i].active) continue;

		bool collideX = (px + pw > fireballs5S1[i].x) && (px < fireballs5S1[i].x + fireballs5S1[i].width);
		bool collideY = (py + ph > fireballs5S1[i].y) && (py < fireballs5S1[i].y + fireballs5S1[i].height);

		if (collideX && collideY) {
			fireballHitsToPlayer5S1++;

			if (fireballHitsToPlayer5S1 >= 8) { // 8 fireball hits to lose 1 zed
				playerLives5S1--;
				fireballHitsToPlayer5S1 = 0;

				if (playerLives5S1 <= 0) {
					playerLives5S1 = 0;
					isGameOver5S1 = true;
				}
			}

			fireballHitCooldown5S1 = 30;
			spawnExplosion5S1(fireballs5S1[i].x + 10, fireballs5S1[i].y + 10);
			fireballs5S1[i].active = false;
			break;
		}
	}
}

static void drawFireballs5S1() {
	// 1. Trail
	for (int i = 0; i < MAX_TRAIL_PARTICLES5S1; i++) {
		if (!trailParticles5S1[i].active) continue;
		int alpha = trailParticles5S1[i].alpha;
		if (alpha > 150) iSetColor(255, 150, 0);
		else if (alpha > 50) iSetColor(200, 50, 0);
		else iSetColor(100, 20, 0);
		iFilledCircle(trailParticles5S1[i].x, trailParticles5S1[i].y, trailParticles5S1[i].radius, 20);
	}

	// 2. Explosions
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5S1; i++) {
		if (!explosionParticles5S1[i].active) continue;
		if (explosionParticles5S1[i].life > 15) iSetColor(255, 255, 100);
		else if (explosionParticles5S1[i].life > 5) iSetColor(255, 100, 0);
		else iSetColor(150, 0, 0);
		iFilledCircle(explosionParticles5S1[i].x, explosionParticles5S1[i].y, explosionParticles5S1[i].radius, 20);
	}

	// 3. Main fireballs
	for (int i = 0; i < MAX_FIREBALLS5S1; i++) {
		if (!fireballs5S1[i].active) continue;
		float cx = fireballs5S1[i].x + 10;
		float cy = fireballs5S1[i].y + 10;

		if (fireballTexture5S1 > 0) {
			iRotate(cx, cy, fireballs5S1[i].rotation);
			iShowImage(fireballs5S1[i].x, fireballs5S1[i].y, fireballs5S1[i].width, fireballs5S1[i].height, fireballTexture5S1);
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
// dragons pay out. spawnGoldBall5S1() always anchors the ball to a real
// platform (a slab top, or the ground) and tags it with that platform's
// index, so it rides the moving slab and can never end up floating in a
// spot the player cannot jump to. The platform and the x offset on it are
// random, so no two runs look the same.
// ---------------------------------------------------------------------------
static void spawnGoldBall5S1(int ballIndex) {
	if (ballIndex < 0 || ballIndex > 2) return;

	// 1 in 4 balls drops on the ground, the rest on a random slab.
	bool onGround = ((rand() % 4) == 0);

	if (onGround) {
		goldBalls5S1[ballIndex].x = 60 + (rand() % 540);
		goldBalls5S1[ballIndex].y = LEVEL5S1_BOTTOM + 5;
		goldBalls5S1[ballIndex].platformIndex = L5S1_GROUND;
		return;
	}

	int p = L5S1_STAIR1 + (rand() % (L5S1_PLATFORM_COUNT - 1));
	Platform pl = level5S1_platforms[p];

	int minX = pl.x1 + 15;
	int maxX = pl.x2 - 40;
	if (maxX <= minX) maxX = minX + 1;

	goldBalls5S1[ballIndex].x = minX + (rand() % (maxX - minX));
	goldBalls5S1[ballIndex].y = pl.y2 + 5;
	goldBalls5S1[ballIndex].platformIndex = p;
}

static void initGoldAndBlueBalls5S1(bool freshStart) {
	loadGoldBallTextures5S1();
	loadBlueBallAndPointsTextures5S1();

	goldBallsCollected5S1 = 0;
	goldBallsSpawned5S1 = 0;
	dragonsKilled5S1 = 0;
	if (freshStart) playerPoints5S1 = 0;

	// 1. GOLD BALLS - unchanged. Parked off-screen until a dragon pays out.
	for (int i = 0; i < 3; i++) {
		goldBalls5S1[i].width = 25;
		goldBalls5S1[i].height = 25;
		goldBalls5S1[i].x = -1000;
		goldBalls5S1[i].y = -1000;
		goldBalls5S1[i].collected = false;
		goldBalls5S1[i].platformIndex = L5S1_GROUND;
	}

	// 2. BLUE BALLS ON THE SLABS - placed deliberately now, not scattered
	//    at random. The old random spread kept dropping them inside slabs
	//    or in pockets the player had no route to. One ball now sits in
	//    the middle of every slab, so simply making the climb pays out,
	//    and each ball rides its slab because platformIndex tags it.
	for (int i = L5S1_STAIR1; i < L5S1_PLATFORM_COUNT; i++) {
		int b = i - 1;
		blueBalls5S1[b].width = 18;
		blueBalls5S1[b].height = 18;
		blueBalls5S1[b].x = level5S1_platforms[i].x1 + (L5S1_SLAB_W / 2) - 9;
		blueBalls5S1[b].y = level5S1_platforms[i].y2 + 5;
		blueBalls5S1[b].collected = false;
		blueBalls5S1[b].platformIndex = i;
	}

	// 3. THE LAST TWO float over the clear strip of ground beside the
	//    staircase, low enough that one standing jump collects them.
	int airX[2] = { 300, 520 };
	int airY[2] = { 100, 100 };
	for (int i = 0; i < 2; i++) {
		int b = (L5S1_PLATFORM_COUNT - 1) + i;
		blueBalls5S1[b].width = 18;
		blueBalls5S1[b].height = 18;
		blueBalls5S1[b].x = airX[i];
		blueBalls5S1[b].y = airY[i];
		blueBalls5S1[b].collected = false;
		blueBalls5S1[b].platformIndex = L5S1_GROUND;
	}
}

static void checkGoldBallCollision5S1() {
	checkGoldBallCollisionGeneric(goldBalls5S1, &goldBallsCollected5S1, &level5S1Complete);
}

static void checkBlueBallCollision5S1() {
	checkBlueBallCollisionGeneric(blueBalls5S1, &playerPoints5S1);
}

static void drawGoldBalls5S1() {
	drawGoldBallsGeneric(goldBalls5S1, goldBallTexture5S1);
}

static void drawBlueBalls5S1() {
	drawBlueBallsGeneric(blueBalls5S1, blueBallTexture5S1);
}

static void drawGoldBallHUD5S1() {
	drawGoldBallHUDGeneric(goldBallsCollected5S1, goldBallIconTextures5S1);
}

static void drawPointsHUD5S1() {
	drawPointsHUDGeneric(pointsTexture5S1, playerPoints5S1);
}

// ---------------------------------------------------------------------------
// Portal - opens once all 3 golden balls are collected.
// ---------------------------------------------------------------------------
static void drawPortal5S1() {
	if (level5S1Complete) {
		if (dispearTexture5S1 > 0) {
			iShowImage(680, 20, 35, 100, dispearTexture5S1);
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
static void stepNextFrame5S1(int i) {
	int safetyCounter = 0;
	do {
		dragons5S1[i].animFrame++;

		if (dragons5S1[i].movingRight) {
			if (dragons5S1[i].animFrame > D5S1_RIGHT_END || dragons5S1[i].animFrame < D5S1_RIGHT_START)
				dragons5S1[i].animFrame = D5S1_RIGHT_START;
		}
		else {
			if (dragons5S1[i].animFrame > D5S1_LEFT_END || dragons5S1[i].animFrame < D5S1_LEFT_START)
				dragons5S1[i].animFrame = D5S1_LEFT_START;
		}

		safetyCounter++;
		if (safetyCounter > D5S1_TOTAL_FRAMES) break;
	} while (dragonTextures5S1[dragons5S1[i].animFrame] <= 0);
}

static void setupDragon5S1(int i, int x, int y, int minX, int maxX) {
	dragons5S1[i].width = 70;
	dragons5S1[i].height = 70;
	dragons5S1[i].x = x;
	dragons5S1[i].y = y;
	dragons5S1[i].minX = minX;
	dragons5S1[i].maxX = maxX;
	dragons5S1[i].movingRight = true;
	dragons5S1[i].animFrame = D5S1_RIGHT_START;
	dragons5S1[i].animTimer = 0;
	dragons5S1[i].health = 5;
	dragons5S1[i].punchCount = 0;
	dragons5S1[i].alive = true;
	dragons5S1[i].rewarded = false;
	dragons5S1[i].platformIndex = L5S1_GROUND;
	if (dragonTextures5S1[dragons5S1[i].animFrame] <= 0) stepNextFrame5S1(i);
}

// Dragon patrol lanes are chosen so a dragon never flies straight through
// the band a staircase slab occupies.
static void initDragon5S1() {
	loadDragonTextures5S1();
	loadDragonHealthBarTextures5S1();

	// One dragon per zone of the climb. Every lane sits in air the staircase
	// never sweeps through, so no dragon is ever buried inside a slab, and
	// each one can be reached with a punch from a real standing spot.
	// Every lane sits in air the staircase never sweeps through, so no
	// dragon is ever buried inside a slab, and each one can be reached
	// with a punch from a real standing spot.
	// Every lane sits in air no slab ever sweeps through, so no dragon is
	// ever buried inside one, and each can be punched from a real spot.
	setupDragon5S1(0, 60, 20, 40, 660);  // ground corridor patrol
	setupDragon5S1(1, 60, 260, 40, 300);  // left side, above the first stairs
	setupDragon5S1(2, 300, 400, 260, 640);  // summit guard above the top ledges
}

static void updateDragon5S1() {
	for (int i = 0; i < L5S1_DRAGON_COUNT; i++) {
		if (!dragons5S1[i].alive) continue;

		int minX = dragons5S1[i].minX;
		int maxX = dragons5S1[i].maxX - dragons5S1[i].width;

		if (dragons5S1[i].movingRight) {
			dragons5S1[i].x += D5S1_SPEED;
			if (dragons5S1[i].x >= maxX) {
				dragons5S1[i].x = maxX;
				dragons5S1[i].movingRight = false;
				dragons5S1[i].animFrame = D5S1_LEFT_START;
				if (dragonTextures5S1[dragons5S1[i].animFrame] <= 0) stepNextFrame5S1(i);
			}
		}
		else {
			dragons5S1[i].x -= D5S1_SPEED;
			if (dragons5S1[i].x <= minX) {
				dragons5S1[i].x = minX;
				dragons5S1[i].movingRight = true;
				dragons5S1[i].animFrame = D5S1_RIGHT_START;
				if (dragonTextures5S1[dragons5S1[i].animFrame] <= 0) stepNextFrame5S1(i);
			}
		}

		dragons5S1[i].animTimer++;
		if (dragons5S1[i].animTimer >= D5S1_ANIM_SPEED) {
			dragons5S1[i].animTimer = 0;
			stepNextFrame5S1(i);

			// Breath-attack poses: the instant the animation reaches frame
			// 16 or 54, throw one fireball at the player.
			// 20% fewer fireballs overall: 1-in-5 chance to skip the throw.
			if (dragons5S1[i].animFrame == FIREBALL_ANIM_FRAME_A5S1 || dragons5S1[i].animFrame == FIREBALL_ANIM_FRAME_B5S1) {
				if (rand() % 5 != 0) spawnFireball5S1(i);
			}
		}
	}
}

// Called once, and only once, per dragon death: every kill pays out one
// golden ball, and the >= 3 guard means a 4th can never appear.
static void rewardDragonKill5S1(int i) {
	if (dragons5S1[i].rewarded) return;
	dragons5S1[i].rewarded = true;

	dragonsKilled5S1++;

	if (dragonsKilled5S1 % L5S1_KILLS_PER_BALL != 0) return;
	if (goldBallsSpawned5S1 >= 3) return; // never hand out a 4th ball

	spawnGoldBall5S1(goldBallsSpawned5S1);
	goldBallsSpawned5S1++;
}

// Called by laserUpdateProjectiles() with the laser projectile's rectangle.
// 5 laser hits = 1 dragon life. Returns true if a dragon was hit.
static bool checkPlayerPunchDragonCollision5S1(int attackX, int attackY, int attackW, int attackH) {
	for (int i = 0; i < L5S1_DRAGON_COUNT; i++) {
		if (!dragons5S1[i].alive) continue;

		int dx = dragons5S1[i].x;
		int dy = dragons5S1[i].y;
		int dw = dragons5S1[i].width;
		int dh = dragons5S1[i].height;

		bool collideX = (attackX + attackW > dx) && (attackX < dx + dw);
		bool collideY = (attackY + attackH > dy) && (attackY < dy + dh);

		if (collideX && collideY) {
			dragons5S1[i].punchCount++;

			if (dragons5S1[i].punchCount >= 5) {
				dragons5S1[i].punchCount = 0;
				dragons5S1[i].health--;

				if (dragons5S1[i].health <= 0) {
					dragons5S1[i].health = 0;
					dragons5S1[i].alive = false;
					rewardDragonKill5S1(i);
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

static void updateFightingAnimation5S1() {
	laserFightStep(isFighting5S1, isLeftMouseDown5S1, fightFrame5S1, fightAnimTimer5S1, punchDamageDealt5S1);
	laserUpdateProjectiles(checkPlayerPunchDragonCollision5S1);   // moves the lasers + hit test
}

static void handleMouseClickLevel5S1(int button, int state) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			isLeftMouseDown5S1 = true;
			if (!isFighting5S1 && !isPaused5S1 && !isGameOver5S1 && !showLevel5S1ExitTransition && !isLevel5S1Transition) {
				isFighting5S1 = true;
				fightAnimTimer5S1 = 0;
				punchDamageDealt5S1 = false;
				fightFrame5S1 = 1;
			}
		}
		else if (state == GLUT_UP) {
			isLeftMouseDown5S1 = false;
		}
	}
}

static void drawDragon5S1() {
	for (int i = 0; i < L5S1_DRAGON_COUNT; i++) {
		if (!dragons5S1[i].alive) continue;

		int textureId = dragonTextures5S1[dragons5S1[i].animFrame];

		if (textureId > 0) {
			iShowImage(dragons5S1[i].x, dragons5S1[i].y, dragons5S1[i].width, dragons5S1[i].height, textureId);
		}
		else {
			iSetColor(255, 0, 0);
			iFilledRectangle(dragons5S1[i].x, dragons5S1[i].y, dragons5S1[i].width, dragons5S1[i].height);
		}

		// health 5 -> 1.png, health 4 -> 2.png, ...
		int spriteIdx = 6 - dragons5S1[i].health;

		if (spriteIdx >= 1 && spriteIdx <= 5 && dragonHealthBarTextures5S1[spriteIdx] > 0) {
			iShowImage(dragons5S1[i].x + 10, dragons5S1[i].y + dragons5S1[i].height + 5, 50, 8, dragonHealthBarTextures5S1[spriteIdx]);
		}
	}
}

static void checkDragonPlayerCollision5S1() {
	if (hitCooldown5S1 > 0) {
		hitCooldown5S1--;
		return;
	}

	for (int i = 0; i < L5S1_DRAGON_COUNT; i++) {
		if (!dragons5S1[i].alive) continue;

		int hx = dragons5S1[i].x;
		int hy = dragons5S1[i].y;
		int hw = dragons5S1[i].width;
		int hh = dragons5S1[i].height;

		if (dragons5S1[i].animFrame >= 1 && dragons5S1[i].animFrame <= 17) {
			hx = dragons5S1[i].x + (int)(dragons5S1[i].width * 0.556f);
			hy = dragons5S1[i].y + (int)(dragons5S1[i].height * 0.312f);
			hw = (int)(dragons5S1[i].width * 0.363f);
			hh = (int)(dragons5S1[i].height * 0.347f);
		}
		else if (dragons5S1[i].animFrame >= D5S1_RIGHT_START && dragons5S1[i].animFrame <= D5S1_RIGHT_END) {
			hx = dragons5S1[i].x + (int)(dragons5S1[i].width * 0.081f);
			hy = dragons5S1[i].y + (int)(dragons5S1[i].height * 0.312f);
			hw = (int)(dragons5S1[i].width * 0.363f);
			hh = (int)(dragons5S1[i].height * 0.347f);
		}

		bool collideX = (player.x + player.width > hx) && (player.x < hx + hw);
		bool collideY = (player.y + player.height > hy) && (player.y < hy + hh);

		if (collideX && collideY) {
			dragonHitsToPlayer5S1++;

			// 8 touches to lose 1 zed (life)
			if (dragonHitsToPlayer5S1 >= 8) {
				playerLives5S1--;
				dragonHitsToPlayer5S1 = 0;

				if (playerLives5S1 <= 0) {
					playerLives5S1 = 0;
					isGameOver5S1 = true;
				}
			}

			hitCooldown5S1 = 30;
			break;
		}
	}
}

static void drawHUD5S1() {
	drawHUDGeneric(zedsLabelTexture5S1, zedsIconTexture5S1, playerLives5S1, energyFrame5S1, energyTextures5S1);
}

// ---------------------------------------------------------------------------
// Stage / level setup
//
// initLevel5S1Stage() resets the playfield. Zeds, points and energy are
// handled by initLevel5S1() so a stage reset never silently refills them.
// ---------------------------------------------------------------------------
static void initLevel5S1Stage(bool freshStart) {
	for (int i = 0; i < L5S1_PLATFORM_COUNT; i++) {
		level5S1_platforms[i] = level5S1_platformStart[i];
		slabDirection5S1[i] = L5S1_START_DIR[i];
		slabCarry5S1[i] = 0.0f;
		slabDeltaX5S1[i] = 0;
		slabDeltaY5S1[i] = 0;
	}


	resetPath5S1();

	initPlayer(40, LEVEL5S1_BOTTOM);
	initDragon5S1();
	initGoldAndBlueBalls5S1(freshStart);

	level5S1Complete = false;
	hitCooldown5S1 = 0;
	dragonHitsToPlayer5S1 = 0;

	currentPunchCombo5S1 = 0;
	isFighting5S1 = false;
	punchDamageDealt5S1 = false;
	laserResetAll();

	for (int i = 0; i < MAX_FIREBALLS5S1; i++) fireballs5S1[i].active = false;
	for (int i = 0; i < MAX_TRAIL_PARTICLES5S1; i++) trailParticles5S1[i].active = false;
	for (int i = 0; i < MAX_EXPLOSION_PARTICLES5S1; i++) explosionParticles5S1[i].active = false;
	fireballHitsToPlayer5S1 = 0;
	fireballHitCooldown5S1 = 0;

	distanceMoved5S1 = 0;
}

static void initLevel5S1(bool freshStart = true) {
	loadSlabTexture5S1();
	loadPauseTextures5S1();
	loadZedsTextures5S1();
	loadEnergyTextures5S1();
	loadFightingTextures5S1();
	loadFireballTexture5S1();
	loadTransitionTextures5S1();

	initLevel5S1Stage(freshStart);

	isGameOver5S1 = false;
	isPaused5S1 = false;

	enterLevel5Sub2 = false;
	showLevel5S1ExitTransition = false;
	level5S1ExitCounter = 0;

	isLevel5S1Transition = true;
	level5S1TransitionCounter = 0;

	playerLives5S1 = 5;
	// Energy and zeds refill at the start of every level.
	energyFrame5S1 = 1;
}

static void drawLevel5S1Background() {
	iShowBMP(0, 0, IMG_LV5S1_BG);
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
static bool playerOverlapsSlab5S1(const Platform& p) {
	return (player.x + player.width > p.x1) && (player.x < p.x2) &&
		(player.y + player.height > p.y1) && (player.y < p.y2);
}

// Vertical: land on tops, bump heads on undersides. Uses where each slab
// WAS before it moved this frame, so a rising slab cannot slip past a
// falling player and a falling slab cannot swallow a rising one.
static void resolvePlatformCollision5S1() {
	int oldY = player.y - player.velocityY;
	int oldTop = oldY + player.height;

	if (player.y <= LEVEL5S1_BOTTOM) {
		player.y = LEVEL5S1_BOTTOM;
		player.velocityY = 0;
		player.onGround = true;
		player.jumping = false;
		return;
	}

	// The player is already glued to this slab (applySlabMotion5S1 carried
	// them by its exact delta this frame), same as a ball tied to a
	// platformIndex. Snap straight to its current top instead of re-deriving
	// "was I above it" from last frame's position: that derivation assumes
	// the slab only ever rises, so on a DESCENDING stair it fails and the
	// player quietly falls out of sync with the slab underneath them. Skip
	// this only while actually jumping (velocityY > 0) or once the player
	// has walked off the slab's X range.
	if (slabRiddenByPlayer5S1 != -1 && player.velocityY <= 0) {
		const Platform& rp = level5S1_platforms[slabRiddenByPlayer5S1];
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
	for (int i = 1; i < L5S1_PLATFORM_COUNT; i++) {
		Platform p = level5S1_platforms[i];
		bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
		if (!withinX) continue;

		int prevTop = p.y2 - slabDeltaY5S1[i];
		int prevBottom = p.y1 - slabDeltaY5S1[i];

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
static void resolveSideCollision5S1(int oldX) {
	if (player.x == oldX) return;
	bool movingRight = (player.x > oldX);

	for (int i = 1; i < L5S1_PLATFORM_COUNT; i++) {
		Platform p = level5S1_platforms[i];
		if (!playerOverlapsSlab5S1(p)) continue;

		if (movingRight) player.x = p.x1 - player.width;
		else player.x = p.x2;
	}
}

static bool playerStandingOnPlatform5S1(int index) {
	Platform p = level5S1_platforms[index];
	bool withinX = (player.x + player.width > p.x1) && (player.x < p.x2);
	return withinX && player.onGround && player.velocityY <= 0 && abs(player.y - p.y2) <= 2;
}

// A slab that has moved into the player displaces them by the smallest
// distance along a direction that is actually free (never into the floor,
// the walls or the ceiling), so the player is slid out instead of squeezed.
static void pushPlayerOutOfSlabs5S1() {
	for (int pass = 0; pass < 3; pass++) {
		bool any = false;

		for (int i = 1; i < L5S1_PLATFORM_COUNT; i++) {
			Platform p = level5S1_platforms[i];
			if (!playerOverlapsSlab5S1(p)) continue;
			any = true;

			int up = p.y2 - player.y;
			int down = (player.y + player.height) - p.y1;
			int left = (player.x + player.width) - p.x1;
			int right = p.x2 - player.x;

			if (player.y - down < LEVEL5S1_BOTTOM) down = 100000;
			if (player.y + up + player.height > LEVEL5S1_TOP) up = 100000;
			if (player.x - left < LEVEL5S1_LEFT) left = 100000;
			if (player.x + right + player.width > LEVEL5S1_RIGHT) right = 100000;

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
static void applySlabMotion5S1(const bool* playerOnSlab) {
	for (int i = 1; i < L5S1_PLATFORM_COUNT; i++) {
		if (playerOnSlab[i]) {
			player.x += slabDeltaX5S1[i];
			player.y += slabDeltaY5S1[i];
		}
	}

	// ...and slide them clear if a slab has moved into them.
	pushPlayerOutOfSlabs5S1();

	for (int i = 0; i < 3; i++) {
		if (!goldBalls5S1[i].collected) {
			int p = goldBalls5S1[i].platformIndex;
			goldBalls5S1[i].x += slabDeltaX5S1[p];
			goldBalls5S1[i].y += slabDeltaY5S1[p];
		}
	}

	for (int i = 0; i < 9; i++) {
		if (!blueBalls5S1[i].collected) {
			int p = blueBalls5S1[i].platformIndex;
			blueBalls5S1[i].x += slabDeltaX5S1[p];
			blueBalls5S1[i].y += slabDeltaY5S1[p];
		}
	}
}


static void updateMovingSlabs5S1() {
	for (int i = 0; i < L5S1_PLATFORM_COUNT; i++) {
		slabDeltaX5S1[i] = 0;
		slabDeltaY5S1[i] = 0;
	}

	bool playerOnSlab[L5S1_PLATFORM_COUNT] = { false };
	slabRiddenByPlayer5S1 = -1;
	for (int i = 1; i < L5S1_PLATFORM_COUNT; i++) {
		playerOnSlab[i] = playerStandingOnPlatform5S1(i);
		if (playerOnSlab[i]) slabRiddenByPlayer5S1 = i;
	}

	for (int k = 0; k < L5S1_PATH_SLABS; k++) {
		int i = k + 1;
		int oldX = level5S1_platforms[i].x1;
		int oldTop = level5S1_platforms[i].y2;

		float s = pathPos5S1[k] + L5S1_PATH_SPEED;
		// Always loop back to the bottom-left, whether or not the player is
		// riding it - it's a conveyor, not a lift that waits at the top.
		// Whatever is on the slab (player included) is carried along by the
		// same delta as the slab itself, so it reappears at the bottom too.
		if (s >= L5S1_PATH_LEN) {
			s -= L5S1_PATH_LEN;
		}
		pathPos5S1[k] = s;
		placePathSlab5S1(k, s);

		slabDeltaX5S1[i] = level5S1_platforms[i].x1 - oldX;
		slabDeltaY5S1[i] = level5S1_platforms[i].y2 - oldTop;
	}

	applySlabMotion5S1(playerOnSlab);
}

static void drawPlatforms5S1() {
	if (slabTexture5S1 <= 0) return;

	for (int i = 1; i < L5S1_PLATFORM_COUNT; i++) {
		int width = level5S1_platforms[i].x2 - level5S1_platforms[i].x1;
		int height = level5S1_platforms[i].y2 - level5S1_platforms[i].y1;
		iShowImage(level5S1_platforms[i].x1, level5S1_platforms[i].y1, width, height, slabTexture5S1);
	}
}

// ---------------------------------------------------------------------------
// Level 5 -> Level 5 Sub 1 handover
// ---------------------------------------------------------------------------
static void checkPortalCollision5S1() {
	if (!level5S1Complete || showLevel5S1ExitTransition || enterLevel5Sub2) return;

	bool collideX = (player.x + player.width >= 700);
	bool collideY = (player.y <= 120 && player.y + player.height >= 20);

	if (collideX && collideY) {
		showLevel5S1ExitTransition = true;
		level5S1ExitCounter = 0;
	}
}

static void updateLevel5S1() {
	// Intro banner
	if (isLevel5S1Transition) {
		level5S1TransitionCounter++;
		if (level5S1TransitionCounter >= 100) {
			isLevel5S1Transition = false;
		}
		return;
	}

	// Exit banner, then hand over to Sub 1. enterLevel5Sub2 latches, so the
	// handover is raised exactly once.
	if (showLevel5S1ExitTransition) {
		level5S1ExitCounter++;
		if (level5S1ExitCounter >= 90) {
			showLevel5S1ExitTransition = false;
			level5S1ExitCounter = 0;
			enterLevel5Sub2 = true;
		}
		return;
	}

	if (isGameOver5S1 || isPaused5S1 || enterLevel5Sub2) return;

	updateMovingSlabs5S1();

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

	if (player.x < LEVEL5S1_LEFT) player.x = LEVEL5S1_LEFT;

	if (player.x + player.width > LEVEL5S1_RIGHT) {
		bool atPortalCoordinates = (player.y <= 120 && player.y + player.height >= 20);
		if (!(level5S1Complete && atPortalCoordinates)) {
			player.x = LEVEL5S1_RIGHT - player.width;
		}
	}

	// Slabs are solid boxes - stop the player walking through one.
	resolveSideCollision5S1(oldX);

	int moveDist = abs(player.x - oldX);
	if (moveDist > 0) {
		distanceMoved5S1 += moveDist;
		while (distanceMoved5S1 >= 150) {
			distanceMoved5S1 -= 150;
			if (energyFrame5S1 < 145) {
				energyFrame5S1++;
			}
		}
		if (energyFrame5S1 >= 145) {
			isGameOver5S1 = true;
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

	resolvePlatformCollision5S1();

	if (player.y + player.height > LEVEL5S1_TOP) {
		player.y = LEVEL5S1_TOP - player.height;
		player.velocityY = 0;
	}

	updatePlayerAnimation();
	updateFightingAnimation5S1();
	updateDragon5S1();
	checkDragonPlayerCollision5S1();

	// --- FIREBALL & PARTICLE UPDATES (same order as level 4) ---
	updateFireballs5S1();
	updateTrailParticles5S1();
	updateExplosionParticles5S1();
	checkFireballCollision5S1();

	checkGoldBallCollision5S1();
	checkBlueBallCollision5S1();
	checkPortalCollision5S1();
}

static void drawPauseMenu5S1() {
	drawPauseMenuGeneric(resumeTexture5S1, pauseResumeBtn5S1, restartTexture5S1, pauseRestartBtn5S1, exitTexture5S1, pauseExitBtn5S1);
}

static void drawLevel5S1() {
	drawLevel5S1Background();

	if (isLevel5S1Transition) {
		// Banner text removed - just hold on the background during the
		// intro pause.
		return;
	}

	drawPlatforms5S1();
	drawDragon5S1();
	drawGoldBalls5S1();
	drawBlueBalls5S1();
	drawPortal5S1();

	if (!(isFighting5S1 && drawLaserFightSprite(fightingTextures5S1, fightFrame5S1, player.facingRight))) {
		drawGunPlayerBody();
	}
	drawLaserProjectiles();

	drawFireballs5S1(); // trails, explosions and the fireballs themselves

	drawHUD5S1();
	drawGoldBallHUD5S1();
	drawPointsHUD5S1();

	if (isPaused5S1) {
		drawPauseMenu5S1();
	}
	else if (isGameOver5S1) {
		drawTotalPointsBoxGeneric(totalPointsTexture5S1, playerPoints5S1, 405);

		if (gameOverTexture5S1 > 0) {
			iShowImage(175, 175, 350, 220, gameOverTexture5S1);
		}
		else {
			iSetColor(255, 0, 0);
			iText(280, 280, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
		}

		if (gameOverRestartTexture5S1 > 0)
			iShowImage(gameOverRestartBtn5S1.x1, gameOverRestartBtn5S1.y1, 220, 50, gameOverRestartTexture5S1);

		if (gameOverExitTexture5S1 > 0)
			iShowImage(gameOverExitBtn5S1.x1, gameOverExitBtn5S1.y1, 150, 35, gameOverExitTexture5S1);
	}
}

#endif