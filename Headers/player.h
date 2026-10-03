#ifndef PLAYER_H
#define PLAYER_H


#include "iGraphics.h"
#include "assets.h"
#include <cstdio>

enum PlayerAnimState { ANIM_IDLE, ANIM_WALK_LEFT, ANIM_WALK_RIGHT, ANIM_JUMP };

struct Player {
	int x, y;
	int width, height;
	int velocityY;
	bool onGround;
	bool jumping;
	bool facingRight;
	bool isMoving;

	PlayerAnimState animState;
	int animFrame;
	int animTimer;
	bool jumpToggle;

	int energy; // Player energy/health percentage (0 to 100)
};

#define PLAYER_WIDTH   50
#define PLAYER_HEIGHT  65
#define GRAVITY        1
#define JUMP_FORCE     18
#define MOVE_SPEED     4
#define MAX_FALL_SPEED 15

#define IDLE_START  1
#define IDLE_END    16

#define LEFT_START  17
#define LEFT_END    45

#define RIGHT_START 46
#define RIGHT_END   75

#define ANIM_SPEED  4

// The new player sprite sheet (Images/player/<n>.png) ships as 142x134 PNGs
// with real alpha transparency - sharper than the old 60x75 colour-keyed
// BMPs, but with a bit of extra transparent padding around the character
// (so the antenna/raised arm never get clipped). Rendering at the old BMP
// HEIGHT keeps the character's on-screen footprint, ground alignment and
// jump arc identical to before; the width is scaled by the same factor so
// the art isn't stretched/squashed.
#define PLAYER_SPRITE_SRC_W     142
#define PLAYER_SPRITE_SRC_H     134
#define PLAYER_SPRITE_DRAW_H    75
#define PLAYER_SPRITE_DRAW_W    ((int)(PLAYER_SPRITE_DRAW_H * (PLAYER_SPRITE_SRC_W / (float)PLAYER_SPRITE_SRC_H)))

static Player player;

// ---------------------------------------------------------------------------
// Jump / shoot sound coordination
// ---------------------------------------------------------------------------
// Jumping only REQUESTS the jump sound here - it isn't played immediately.
// In the fighting levels, shooting is checked later in the same game tick
// (see laser_fight.h's laserSpawn()), and if a shot fires this tick, only
// the shoot sound should be heard. resolveJumpSfx() is called once per tick
// from fixedUpdate() in iMain.cpp, AFTER every level's update logic has run,
// so by then we know for certain whether a shot also happened this tick.
static bool jumpSfxPending = false;
static bool shotFiredThisFrame = false; // set by laser_fight.h's laserSpawn()

static void requestJumpSfx() {
	jumpSfxPending = true;
}

// Small rotating pool of MCI aliases (like the shoot SFX) so repeated jumps
// don't cut each other's sound off.
#define JUMP_SFX_SLOTS 3
static void resolveJumpSfx() {
	if (jumpSfxPending && !shotFiredThisFrame && soundOn) {
		static bool slotOpened[JUMP_SFX_SLOTS] = { false };
		static int slot = 0;

		char alias[16];
		sprintf(alias, "jumpsfx%d", slot);

		char cmd[256];
		if (slotOpened[slot]) {
			sprintf(cmd, "close %s", alias);
			mciSendString(cmd, NULL, 0, NULL);
		}
		sprintf(cmd, "open \"%s\" alias %s", SFX_JUMP, alias);
		mciSendString(cmd, NULL, 0, NULL);
		sprintf(cmd, "play %s", alias);
		mciSendString(cmd, NULL, 0, NULL);
		slotOpened[slot] = true;

		slot = (slot + 1) % JUMP_SFX_SLOTS;
	}
	jumpSfxPending = false;
}

// animFrame runs from IDLE_START(1) to RIGHT_END(75); index 0 is unused.
static unsigned int playerFrameTex[RIGHT_END + 1] = { 0 };
static unsigned int playerJumpTex1 = 0;
static unsigned int playerJumpTex2 = 0;
static bool playerTexturesLoaded = false;

static void loadPlayerTextures() {
	if (playerTexturesLoaded) return;

	char path[64];
	for (int i = IDLE_START; i <= RIGHT_END; i++) {
		sprintf(path, "Images/player/%d.png", i);
		playerFrameTex[i] = iLoadImage(path);
	}

	char jump1[64], jump2[64];
	sprintf(jump1, SPRITE_JUMP1);
	sprintf(jump2, SPRITE_JUMP2);
	playerJumpTex1 = iLoadImage(jump1);
	playerJumpTex2 = iLoadImage(jump2);

	playerTexturesLoaded = true;
}

static void initPlayer(int startX, int startY) {
	loadPlayerTextures();

	player.x = startX;
	player.y = startY;
	player.width = PLAYER_WIDTH;
	player.height = PLAYER_HEIGHT;
	player.velocityY = 0;
	player.onGround = false;
	player.jumping = false;
	player.facingRight = true;
	player.isMoving = false;

	player.animState = ANIM_IDLE;
	player.animFrame = IDLE_START;
	player.animTimer = 0;
	player.jumpToggle = false;

	player.energy = 100;
}

static void updatePlayerAnimation() {
	PlayerAnimState newState;

	if (!player.onGround)
		newState = ANIM_JUMP;
	else if (player.isMoving && !player.facingRight)
		newState = ANIM_WALK_LEFT;
	else if (player.isMoving && player.facingRight)
		newState = ANIM_WALK_RIGHT;
	else
		newState = ANIM_IDLE;

	if (newState != player.animState) {
		player.animState = newState;
		player.animTimer = 0;

		switch (newState) {
		case ANIM_IDLE:       player.animFrame = IDLE_START;  break;
		case ANIM_WALK_LEFT:  player.animFrame = LEFT_START;  break;
		case ANIM_WALK_RIGHT: player.animFrame = RIGHT_START; break;
		case ANIM_JUMP:       player.jumpToggle = false;      break;
		}
	}

	player.animTimer++;
	if (player.animTimer < ANIM_SPEED)
		return;
	player.animTimer = 0;

	switch (player.animState) {
	case ANIM_IDLE:
		player.animFrame++;
		if (player.animFrame > IDLE_END) player.animFrame = IDLE_START;
		break;

	case ANIM_WALK_LEFT:
		player.animFrame++;
		if (player.animFrame > LEFT_END) player.animFrame = LEFT_START;
		break;

	case ANIM_WALK_RIGHT:
		player.animFrame++;
		if (player.animFrame > RIGHT_END) player.animFrame = RIGHT_START;
		break;

	case ANIM_JUMP:
		player.jumpToggle = !player.jumpToggle;
		break;
	}
}

// ---------------------------------------------------------------------------
// Collision hitbox tuning
// ---------------------------------------------------------------------------
// player.width/height (50x65) describe the full sprite CANVAS, not the
// visible character silhouette - every animation frame (idle/walk/jump) has
// some transparent padding baked into that box, most noticeably during the
// jump pose (extra headroom for the raised arm). Each level's dragon hurtbox
// is already trimmed down from its raw sprite box with per-direction offset
// constants, but it was being compared against the PLAYER's full, untrimmed
// box - so the invisible padding around the player could overlap the
// dragon's hitbox before the visible sprites actually touched. That's what
// caused "hits" with no visible contact.
//
// getPlayerHitbox() shrinks the raw box down to the part that's actually
// solid. Tune these percentages if it still feels off (increase to be more
// forgiving / shrink the hitbox further, decrease to make it stricter).
#define PLAYER_HITBOX_INSET_X_PCT      0.20f  // trimmed off each side (left+right)
#define PLAYER_HITBOX_INSET_TOP_PCT    0.15f  // trimmed off the top (jump-pose headroom)
#define PLAYER_HITBOX_INSET_BOTTOM_PCT 0.05f  // trimmed off the bottom (feet)

static void getPlayerHitbox(int& hx, int& hy, int& hw, int& hh) {
	int insetX = (int)(player.width * PLAYER_HITBOX_INSET_X_PCT);
	int insetTop = (int)(player.height * PLAYER_HITBOX_INSET_TOP_PCT);
	int insetBottom = (int)(player.height * PLAYER_HITBOX_INSET_BOTTOM_PCT);

	hx = player.x + insetX;
	hw = player.width - 2 * insetX;
	hy = player.y + insetBottom;
	hh = player.height - insetTop - insetBottom;
}

static void drawPlayer() {
	loadPlayerTextures(); // no-op after the first call; safety net if initPlayer() wasn't hit first

	unsigned int tex;

	if (player.animState == ANIM_JUMP) {
		tex = player.jumpToggle ? playerJumpTex1 : playerJumpTex2;
	}
	else {
		tex = playerFrameTex[player.animFrame];
	}

	if (tex) {
		iShowImage(player.x, player.y, PLAYER_SPRITE_DRAW_W, PLAYER_SPRITE_DRAW_H, tex);
	}
}

#endif