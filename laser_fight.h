#ifndef LASER_FIGHT_H
#define LASER_FIGHT_H

// ============================================================================
// laser_fight.h
//
// Shared "laser gun" player + attack used by every fighting level (level4/5
// and their sub-levels).
//
//   PLAYER (gun, always carried) --fires--> LASER PROJECTILE
//   travels --hits--> DRAGON
//
// The player now visibly carries the gun at all times in Level 4/5 (and
// their sub-levels), not just while attacking:
//
//   * Standing / not moving   -> 4-frame idle-with-gun animation
//   * Moving left/right       -> 8-frame walk-with-gun animation
//   * Left mouse held/clicked -> 8-frame shoot animation (aim/recoil), and
//     a separate LaserShot projectile is spawned at the gun's muzzle on the
//     firing frame. The projectile is NOT a beam glued to the player - it is
//     its own object that keeps travelling after the shoot pose ends.
//   * On a hit, a 6-frame impact burst plays where the projectile lands.
//
// All of this is shared/global (declared once here) because only one level
// is ever active at a time and the player is the same character everywhere,
// exactly like the existing shared laserShots/laserImpacts arrays below.
// Each level file keeps its own isFightingN / fightFrameN / etc. state (so
// per-level pause/mouse handling is untouched) and just calls into the
// shared step/draw functions here.
//
// Images/PlayerLaser/idle/        1-4 facing right, 5-8 mirrored (left)
// Images/PlayerLaser/walk/        1-8 facing right, 9-16 mirrored (left)
// Images/PlayerLaser/shoot/       1-8 facing right, 9-16 mirrored (left)
// Images/PlayerLaser/projectile/  1-9 facing right, 10-18 mirrored (left)
// Images/PlayerLaser/impact/      1-6 facing right, 7-12 mirrored (left)
//
// The game ticks every 16 ms, so "ticks" below are 16 ms each.
// ============================================================================

#include "iGraphics.h"
#include "player.h"
#include <cstdio>

#define LASER_FRAMES      8    // right-facing shoot frames 1..8
#define LASER_FRAMES_ALL  16   // + mirrored left-facing frames 9..16

#define LASER_HIT_FRAME   4    // the frame on which the projectile leaves the gun
#define LASER_LOOP_LAST   6    // last "active" frame (recoil)
#define LASER_LOOP_FIRST  2    // held mouse: jump back here instead of the guard pose

// Duration of each frame in ticks (index = frame number, [0] unused).
// Whole attack = 27 ticks (~0.43 s); held-mouse loop (frames 2-6) = 17 ticks.
static const int LASER_FRAME_TICKS[LASER_FRAMES + 1] = { 0, 3, 3, 3, 5, 4, 3, 3, 3 };

// ---------------------------------------------------------------------------
// Shared draw geometry (game units).
// The idle/walk/shoot sprites were all normalized onto canvases where the
// character's feet-centre sits on the canvas' horizontal centre and its
// feet sit GUN_BASELINE_OFFSET (source px, already folded into the numbers
// below) above the canvas bottom - so every state draws from the same
// feet-centre anchor and there's no visual "pop" switching between them.
// ---------------------------------------------------------------------------
#define GUN_DRAW_H          75    // shared draw height for idle/walk/shoot - matches the
// normal (unarmed) player sprite's on-screen size (60x75)
#define GUN_IDLE_DRAW_W     61
#define GUN_WALK_DRAW_W     61
#define GUN_SHOOT_DRAW_W    80
#define GUN_BASELINE_OFFSET 2     // draw Y = player.y - this, so feet line up with player.y

#define GUN_IDLE_FRAMES        4
#define GUN_WALK_FRAMES        8
#define GUN_IDLE_FRAME_TICKS   10
#define GUN_WALK_FRAME_TICKS   6

// ---------------------------------------------------------------------------
// Projectile settings
// ---------------------------------------------------------------------------
#define LASER_SPEED        12    // units the laser moves per tick (16 ms)  <- tune here
#define MAX_LASERS         6     // shots that can be in flight at once
#define LASER_LEN          42    // projectile draw length (units)
#define LASER_H            21    // projectile draw height (units)
#define LASER_HIT_PAD      5     // extra hit-box height above/below the sprite (forgiving)

// Muzzle position, measured from the player's feet-centre (player.x + PLAYER_WIDTH/2).
// Tuned by comparing the new sprite's gun-tip position against the same
// measurement the old fighting sprite used, so the point the shot leaves
// from lines up with where the gun is actually drawn.
#define GUN_MUZZLE_DX      30    // muzzle (gun tip) from feet-centre when facing right
#define GUN_MUZZLE_DY      41    // muzzle height above player.y (feet)
// (Both rescaled to match the gun sprite's new, player-sized draw dimensions
// above - keep this in the same ratio to GUN_DRAW_H/GUN_SHOOT_DRAW_W if those
// are ever changed again, so the laser keeps leaving from the gun's tip.)

#define LASER_SCREEN_W       700  // laser is removed once it leaves the play area
#define PROJECTILE_FRAMES    9
#define PROJECTILE_FRAME_TICKS 3  // ticks each projectile animation frame is shown

#define IMPACT_FRAMES         6
#define LASER_IMPACT_TICKS   18   // total impact lifetime (3 ticks/frame * 6 frames)
#define LASER_IMPACT_W        34
#define LASER_IMPACT_H        34

static int laserSpeed = LASER_SPEED;   // change at runtime if you want power-ups etc.

struct LaserShot {
	float x;        // left edge of the sprite
	int y;          // bottom edge of the sprite
	int dir;        // +1 = right, -1 = left
	bool active;
	int animFrame;  // 1..PROJECTILE_FRAMES
	int animTimer;
};

struct LaserImpact {
	int x, y, dir, timer;   // timer > 0 while the flash is showing
};

static LaserShot laserShots[MAX_LASERS];
static LaserImpact laserImpacts[MAX_LASERS];

// Set by the level's hit-check function (checkPlayerPunchDragonCollision*/
// bossHitByLaser*) right before it returns true, to the centre of whatever
// was actually hit (dragon or boss) - so the impact flash can be drawn in
// the middle of the target instead of wherever the laser bolt happened to
// be travelling through when the hit was registered.
static int laserHitCenterX = 0;
static int laserHitCenterY = 0;
static bool laserHitCenterSet = false;

// index 1..PROJECTILE_FRAMES = right facing, PROJECTILE_FRAMES+1..2*PROJECTILE_FRAMES = left
static int laserProjectileTex[2 * PROJECTILE_FRAMES + 1] = { 0 };
// index 1..IMPACT_FRAMES = right facing, IMPACT_FRAMES+1..2*IMPACT_FRAMES = left
static int laserImpactTex[2 * IMPACT_FRAMES + 1] = { 0 };
static bool laserTexLoaded = false;
static bool laserPrevMouseDown = false;
static bool laserClickQueued = false;

// ---------------------------------------------------------------------------
// Gun-player idle/walk (shared across every level - only one level plays at
// a time and it's always the same character).
// ---------------------------------------------------------------------------
static int gunIdleTex[GUN_IDLE_FRAMES * 2 + 1] = { 0 };
static int gunWalkTex[GUN_WALK_FRAMES * 2 + 1] = { 0 };
static bool gunBodyTexLoaded = false;

static int gunBodyFrame = 1;
static int gunBodyTimer = 0;
static bool gunBodyWasMoving = false;

static void gunPlayerLoadTextures() {
	if (gunBodyTexLoaded) return;
	char path[128];
	for (int i = 1; i <= GUN_IDLE_FRAMES * 2; i++) {
		sprintf(path, "Images/PlayerLaser/idle/%d.png", i);
		gunIdleTex[i] = iLoadImage(path);
		if (gunIdleTex[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/idle/%d.png", i);
			gunIdleTex[i] = iLoadImage(path);
		}
	}
	for (int i = 1; i <= GUN_WALK_FRAMES * 2; i++) {
		sprintf(path, "Images/PlayerLaser/walk/%d.png", i);
		gunWalkTex[i] = iLoadImage(path);
		if (gunWalkTex[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/walk/%d.png", i);
			gunWalkTex[i] = iLoadImage(path);
		}
	}
	gunBodyTexLoaded = true;
}

static void gunPlayerResetAnim() {
	gunBodyFrame = 1;
	gunBodyTimer = 0;
	gunBodyWasMoving = false;
}

// Steps the idle/walk animation. Only called while the player is NOT in the
// middle of a shoot animation (see laserFightStep below).
static void gunPlayerStepAnim() {
	if (player.isMoving != gunBodyWasMoving) {
		gunBodyWasMoving = player.isMoving;
		gunBodyFrame = 1;
		gunBodyTimer = 0;
	}

	int frameTicks = player.isMoving ? GUN_WALK_FRAME_TICKS : GUN_IDLE_FRAME_TICKS;
	int maxFrame = player.isMoving ? GUN_WALK_FRAMES : GUN_IDLE_FRAMES;

	gunBodyTimer++;
	if (gunBodyTimer < frameTicks) return;
	gunBodyTimer = 0;

	gunBodyFrame++;
	if (gunBodyFrame > maxFrame) gunBodyFrame = 1;
}

// Draws the player holding the gun (idle or walk pose). Falls back to the
// original drawPlayer() sprite if the new art failed to load, so the game
// never draws nothing for the player.
static void drawGunPlayerBody() {
	gunPlayerLoadTextures();

	bool moving = player.isMoving;
	int maxFrame = moving ? GUN_WALK_FRAMES : GUN_IDLE_FRAMES;
	int frame = gunBodyFrame;
	if (frame < 1 || frame > maxFrame) frame = 1;

	int* texArr = moving ? gunWalkTex : gunIdleTex;
	int idx = player.facingRight ? frame : frame + maxFrame;
	int tex = texArr[idx];

	if (tex <= 0) {
		drawPlayer();   // fallback: old bare-handed sprite
		return;
	}

	int drawW = moving ? GUN_WALK_DRAW_W : GUN_IDLE_DRAW_W;
	int drawX = player.x + PLAYER_WIDTH / 2 - drawW / 2;
	int drawY = player.y - GUN_BASELINE_OFFSET;
	iShowImage(drawX, drawY, drawW, GUN_DRAW_H, tex);
}

static void laserLoadTextures() {
	if (laserTexLoaded) return;
	char path[128];
	for (int i = 1; i <= PROJECTILE_FRAMES * 2; i++) {
		sprintf(path, "Images/PlayerLaser/projectile/%d.png", i);
		laserProjectileTex[i] = iLoadImage(path);
		if (laserProjectileTex[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/projectile/%d.png", i);
			laserProjectileTex[i] = iLoadImage(path);
		}
	}
	for (int i = 1; i <= IMPACT_FRAMES * 2; i++) {
		sprintf(path, "Images/PlayerLaser/impact/%d.png", i);
		laserImpactTex[i] = iLoadImage(path);
		if (laserImpactTex[i] <= 0) {
			sprintf(path, "../Images/PlayerLaser/impact/%d.png", i);
			laserImpactTex[i] = iLoadImage(path);
		}
	}
	laserTexLoaded = true;
}

static void laserResetAll() {
	for (int i = 0; i < MAX_LASERS; i++) {
		laserShots[i].active = false;
		laserImpacts[i].timer = 0;
	}
	laserClickQueued = false;
	laserPrevMouseDown = false;
	gunPlayerResetAnim();
}

static int laserTextureIndex(int frame, bool facingRight) {
	return facingRight ? frame : frame + LASER_FRAMES;
}

// Plays the gunshot sound effect. Uses a small rotating pool of MCI aliases
// (instead of one fixed alias) so that firing rapidly - e.g. holding the
// mouse down - lets overlapping shots each play their own sound instead of
// cutting each other off.
#define SHOOT_SFX_SLOTS 4
static void playShootSfx() {
	if (!soundOn) return;

	static bool slotOpened[SHOOT_SFX_SLOTS] = { false };
	static int slot = 0;

	char alias[16];
	sprintf(alias, "shootsfx%d", slot);

	char cmd[256];
	if (slotOpened[slot]) {
		sprintf(cmd, "close %s", alias);
		mciSendString(cmd, NULL, 0, NULL);
	}
	sprintf(cmd, "open \"%s\" alias %s", SFX_SHOOT, alias);
	mciSendString(cmd, NULL, 0, NULL);
	sprintf(cmd, "play %s", alias);
	mciSendString(cmd, NULL, 0, NULL);
	slotOpened[slot] = true;

	slot = (slot + 1) % SHOOT_SFX_SLOTS;
}

// Plays the dragon/boss hit sound. Own rotating pool of MCI aliases (kept
// separate from the shoot-sfx pool above) so a hit sound never has to wait
// on - or get cut off by - a gunshot sound, and rapid-fire hits each get
// their own channel instead of stepping on one another. "play" is issued
// without the "wait" flag, so this call returns immediately (no stalling
// the game loop / no added latency) and the sound plays on its own thread.
#define HIT_SFX_SLOTS 4
static void playHitSfx() {
	if (!soundOn) return;

	static bool slotOpened[HIT_SFX_SLOTS] = { false };
	static int slot = 0;

	char alias[16];
	sprintf(alias, "hitsfx%d", slot);

	char cmd[256];
	if (slotOpened[slot]) {
		sprintf(cmd, "close %s", alias);
		mciSendString(cmd, NULL, 0, NULL);
	}
	sprintf(cmd, "open \"%s\" alias %s", SFX_HIT, alias);
	mciSendString(cmd, NULL, 0, NULL);
	sprintf(cmd, "play %s", alias);
	mciSendString(cmd, NULL, 0, NULL);
	slotOpened[slot] = true;

	slot = (slot + 1) % HIT_SFX_SLOTS;
}

// Creates a new projectile at the gun muzzle, travelling the way the player faces.
static void laserSpawn() {
	laserLoadTextures();
	int slot = -1;
	for (int i = 0; i < MAX_LASERS; i++) if (!laserShots[i].active) { slot = i; break; }
	if (slot < 0) return;

	shotFiredThisFrame = true; // this tick is a shooting tick - suppress any pending jump sound
	playShootSfx();   // fire sound plays at the exact moment the shot leaves the gun

	LaserShot& s = laserShots[slot];
	s.dir = player.facingRight ? 1 : -1;

	// Muzzle position relative to the player's feet-centre - NOT the player's
	// hand, chest, or a fixed screen coordinate - so it tracks the player and
	// mirrors correctly depending on which way they're facing.
	int centerX = player.x + PLAYER_WIDTH / 2;
	int muzzleX = centerX + (s.dir > 0 ? GUN_MUZZLE_DX : -GUN_MUZZLE_DX);

	s.x = (float)(s.dir > 0 ? muzzleX : muzzleX - LASER_LEN);
	s.y = player.y + GUN_MUZZLE_DY - LASER_H / 2;
	s.active = true;
	s.animFrame = 1;
	s.animTimer = 0;
}

// Advances one tick of the gun animation and fires the projectile on the
// firing frame. (The attack/hold/queue behaviour is unchanged from before.)
static void laserFightStep(bool& fighting, bool mouseDown, int& frame, int& timer, bool& damageDealt) {
	// Remember a fresh click that lands while an attack is still playing.
	bool freshClick = mouseDown && !laserPrevMouseDown;
	laserPrevMouseDown = mouseDown;
	if (fighting && freshClick && !(frame == 1 && timer == 0))
		laserClickQueued = true;

	if (!fighting) {
		if (mouseDown || laserClickQueued) {
			fighting = true;
			frame = 1;
			timer = 0;
			damageDealt = false;
		}
		laserClickQueued = false;
		if (!fighting) {
			gunPlayerStepAnim();   // not shooting: keep the idle/walk cycle going
			return;
		}
	}

	if (frame == LASER_HIT_FRAME && !damageDealt) {
		laserSpawn();            // the shot leaves the gun here
		damageDealt = true;
	}

	timer++;
	if (timer < LASER_FRAME_TICKS[frame]) return;

	timer = 0;
	frame++;
	damageDealt = false;

	// Held mouse / queued click: keep firing from the wind-up, skipping the recovery.
	if (frame > LASER_LOOP_LAST && frame <= LASER_FRAMES && (mouseDown || laserClickQueued)) {
		frame = LASER_LOOP_FIRST;
		laserClickQueued = false;
		return;
	}

	if (frame > LASER_FRAMES) {
		if (mouseDown || laserClickQueued) {
			frame = 1;                 // fire again
			laserClickQueued = false;
		}
		else {
			fighting = false;          // back to idle/walk gun pose
			frame = 1;
		}
	}
}

// Moves every projectile one tick and tests it against the dragons.
// hitFn(x, y, w, h) is the level's dragon-hit check; it applies the damage
// and returns true if a dragon was hit.
static void laserUpdateProjectiles(bool(*hitFn)(int, int, int, int)) {
	for (int i = 0; i < MAX_LASERS; i++)
	if (laserImpacts[i].timer > 0) laserImpacts[i].timer--;

	for (int i = 0; i < MAX_LASERS; i++) {
		LaserShot& s = laserShots[i];
		if (!s.active) continue;

		// step the travelling-projectile animation
		s.animTimer++;
		if (s.animTimer >= PROJECTILE_FRAME_TICKS) {
			s.animTimer = 0;
			s.animFrame++;
			if (s.animFrame > PROJECTILE_FRAMES) s.animFrame = 1;
		}

		float oldX = s.x;
		s.x += s.dir * laserSpeed;

		// swept box (old position -> new position) so a fast laser can't skip over a dragon
		float minX = (oldX < s.x) ? oldX : s.x;
		float maxX = ((oldX > s.x) ? oldX : s.x) + LASER_LEN;
		laserHitCenterSet = false;   // the hit-check fills this in only if it hits something
		if (hitFn((int)minX, s.y - LASER_HIT_PAD, (int)(maxX - minX), LASER_H + 2 * LASER_HIT_PAD)) {
			playHitSfx();   // fire the instant the hit is registered - same tick, no delay
			s.active = false;
			for (int k = 0; k < MAX_LASERS; k++) {
				if (laserImpacts[k].timer <= 0) {
					// Impact flash shows at the middle of the dragon/boss that was hit,
					// falling back to the laser's own position if the hit-check didn't
					// report a centre for some reason.
					if (laserHitCenterSet) {
						laserImpacts[k].x = laserHitCenterX;
						laserImpacts[k].y = laserHitCenterY;
					}
					else {
						laserImpacts[k].x = (int)(s.dir > 0 ? s.x + LASER_LEN : s.x);
						laserImpacts[k].y = s.y + LASER_H / 2;
					}
					laserImpacts[k].dir = s.dir;
					laserImpacts[k].timer = LASER_IMPACT_TICKS;
					break;
				}
			}
			continue;
		}

		if (s.x > LASER_SCREEN_W || s.x + LASER_LEN < 0) s.active = false;
	}
}

// Draws the projectiles (and their impact flashes).
static void drawLaserProjectiles() {
	laserLoadTextures();
	for (int i = 0; i < MAX_LASERS; i++) {
		if (!laserShots[i].active) continue;
		LaserShot& s = laserShots[i];
		int frame = (s.animFrame < 1 || s.animFrame > PROJECTILE_FRAMES) ? 1 : s.animFrame;
		int idx = s.dir > 0 ? frame : frame + PROJECTILE_FRAMES;
		int tex = laserProjectileTex[idx];
		if (tex > 0)
			iShowImage((int)s.x, s.y, LASER_LEN, LASER_H, tex);
		else {   // fallback if the image is missing
			iSetColor(120, 200, 255);
			iFilledRectangle((int)s.x, s.y + 3, LASER_LEN, LASER_H - 6);
		}
	}
	for (int i = 0; i < MAX_LASERS; i++) {
		LaserImpact& im = laserImpacts[i];
		if (im.timer <= 0) continue;
		// timer counts DOWN from LASER_IMPACT_TICKS to 0; map that to frames
		// 1 (small burst) -> IMPACT_FRAMES (dissipating) as time passes.
		int elapsed = LASER_IMPACT_TICKS - im.timer;
		int ticksPerFrame = LASER_IMPACT_TICKS / IMPACT_FRAMES;
		if (ticksPerFrame < 1) ticksPerFrame = 1;
		int frame = 1 + elapsed / ticksPerFrame;
		if (frame > IMPACT_FRAMES) frame = IMPACT_FRAMES;
		int idx = im.dir > 0 ? frame : frame + IMPACT_FRAMES;
		int tex = laserImpactTex[idx];
		if (tex <= 0) continue;
		iShowImage(im.x - LASER_IMPACT_W / 2, im.y - LASER_IMPACT_H / 2, LASER_IMPACT_W, LASER_IMPACT_H, tex);
	}
}

// Draws the current gun-animation (shooting) frame at the player's position.
// Returns false if the texture is missing so the caller can fall back to the
// idle/walk gun pose (drawGunPlayerBody).
static bool drawLaserFightSprite(int* textures, int frame, bool facingRight) {
	if (frame < 1 || frame > LASER_FRAMES) return false;
	int tex = textures[laserTextureIndex(frame, facingRight)];
	if (tex <= 0) return false;

	int drawX = player.x + PLAYER_WIDTH / 2 - GUN_SHOOT_DRAW_W / 2;
	int drawY = player.y - GUN_BASELINE_OFFSET;
	iShowImage(drawX, drawY, GUN_SHOOT_DRAW_W, GUN_DRAW_H, tex);
	return true;
}

#endif