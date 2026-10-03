/*
PROJECT      :         ZENDROID
CONTRIBUTORS :
ID :         00725105101032
00725105101045
00725105101057
*/
#define screenWidth 700
#define screenHeight 500

#include "iGraphics.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include "assets.h"
#include "game_objects.h"
#include "level1.h"
#include "level1_sub1.h"
#include "level2.h"
#include "level2_sub1.h"
#include "level2_sub2.h"
#include "level3.h"
#include "level3_sub1.h"
#include "level3_sub2.h" 
#include "level4.h"
#include "level4_sub1.h"
#include "level4_sub2.h"
#include "level5.h"
#include "level5_sub1.h"
#include "level5_sub2.h"
#include "bg_slide.h"

using namespace std;

int x = 0;
int y = 0;

// ===== Game State =====
enum GameState { INTRO, MAIN_MENU, SETTINGS, SOUND_AND_SCORE, SCORE_BOARD, NAME_ENTRY, CREDITS, LOAD_GAME, PLAYING };
GameState currentState = INTRO;
int currentLevel = 1;

bool soundOn = true;

// ===== Per-Level Background Music =====
// One track (BGM_LEVEL) plays for every level - 1, 2, 3, and so on.
// currentLevelMusic just tracks whether it's already playing (1) or not
// (-1), so we don't reopen/restart it needlessly on sub-level transitions.
int currentLevelMusic = -1; // -1 = none playing, 1 = level track playing

void stopLevelMusic() {
	mciSendString("stop levelbg", NULL, 0, NULL);
	mciSendString("close levelbg", NULL, 0, NULL);
	currentLevelMusic = -1;
}

// Maps an internal level/sub-level id to whether it should have level music.
int musicForLevel(int lv) {
	switch (lv) {
	case 1: case 10:                                        // Level 1 + its sub
	case 2: case 11: case 12:                                // Level 2 + its subs
	case 3: case 4: case 5:
	case 6: case 7: case 8: case 9:
	case 13: case 14: return 1;                              // Levels 3/4/5 + their subs
	}
	return 0;
}

void playLevelMusic(int lv) {
	int track = musicForLevel(lv);
	if (track == currentLevelMusic) return; // already playing, don't restart it

	stopLevelMusic();
	mciSendString("stop bgsong", NULL, 0, NULL); // menu tune pauses while a level plays
	if (!soundOn || track == 0) return;

	char cmd[256];
	sprintf(cmd, "open \"%s\" alias levelbg", BGM_LEVEL);
	mciSendString(cmd, NULL, 0, NULL);
	mciSendString("play levelbg repeat", NULL, 0, NULL);
	currentLevelMusic = track;
}

// Sets currentLevel AND swaps music if the new level needs a different track.
// Used everywhere a bare "currentLevel = X;" used to appear in the level flow.
void switchToLevel(int lv) {
	currentLevel = lv;
	playLevelMusic(lv);
}

// Used everywhere a bare "currentState = MAIN_MENU;" used to appear.
void goToMainMenu() {
	cancelBgSlide();
	stopLevelMusic();
	if (soundOn) mciSendString("play bgsong repeat", NULL, 0, NULL);
	currentState = MAIN_MENU;
}

// Same as goToMainMenu(), but lands on the Load Game screen instead - used by
// the in-game pause/game-over "Exit" buttons so leaving a level takes you
// straight to level selection rather than all the way back to the main menu.
void goToLoadGame() {
	cancelBgSlide();
	stopLevelMusic();
	if (soundOn) mciSendString("play bgsong repeat", NULL, 0, NULL);
	currentState = LOAD_GAME;
}

// ===== Score file handling =====
// Scores are kept one per line as "name,points" in SCORES_FILE, highest
// score first once loaded. loadScores() re-reads the file; saveScore()
// appends a new record then reloads so the in-memory list stays sorted.
const char* SCORES_FILE = "scores.txt";

struct ScoreEntry {
	string name;
	int points;
};

vector<ScoreEntry> scoreList;

void loadScores() {
	scoreList.clear();
	ifstream fin(SCORES_FILE);
	if (!fin.is_open()) return;

	string line;
	while (getline(fin, line)) {
		if (line.empty()) continue;
		size_t comma = line.find_last_of(',');
		if (comma == string::npos) continue;

		ScoreEntry e;
		e.name = line.substr(0, comma);
		e.points = atoi(line.substr(comma + 1).c_str());
		scoreList.push_back(e);
	}
	fin.close();

	// stable_sort keeps the file (= play) order for equal points, so if two
	// players have the same score, the one who played FIRST ranks higher.
	stable_sort(scoreList.begin(), scoreList.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
		return a.points > b.points;
	});
}

void saveScore(const string& name, int points) {
	ofstream fout(SCORES_FILE, ios::app);
	if (fout.is_open()) {
		fout << name << "," << points << "\n";
		fout.close();
	}   
	loadScores(); // Refresh the in-memory list (now sorted, includes the new entry)
}

// Builds a default name that nobody in scores.txt has used yet:
// Player1, Player2, Player3, ...
string makeUniquePlayerName() {
	loadScores();
	for (int n = 1;; n++) {
		string candidate = "Player" + to_string(n);
		bool taken = false;
		for (size_t i = 0; i < scoreList.size(); i++) {
			if (scoreList[i].name == candidate) { taken = true; break; }
		}
		if (!taken) return candidate;
	}
}

// Player name currently being typed on the NAME_ENTRY screen, and the
// final total score that will be attached to it once submitted.
string enteredName = "";
int finalGameScore = 0;
string playerName = "";        // current player's name (asked on first visit to Settings)
int nameEntryPurpose = 0;      // 0 = from Settings (just set name), 1 = game finished (save score)
#define MAX_NAME_LEN 16

// ---- Sound-And-Score Screen Buttons ----
Button soundBtnSAS = { 138, 251, 400, 325 };  // "SOUND" pill
Button scoreBtnSAS = { 138, 151, 400, 225 };   // "SCORE" pill

// ===== Tile Reveal Effect =====
#define GRID_COLS 10
#define GRID_ROWS 6
#define TILE_W (screenWidth / GRID_COLS)
#define TILE_H (screenHeight / GRID_ROWS)

// Modified arrays for the sink effect
float tileScale[GRID_ROWS][GRID_COLS];
bool tileSinking[GRID_ROWS][GRID_COLS];
int revealOrder[GRID_ROWS * GRID_COLS];
int revealIndex = 0;
int frameCounter = 0;
bool introDone = false;

// Controls how fast the wipe plays: revealStepInterval = frames between ticks,
// revealTilesPerTick = tiles revealed per tick. initTiles() resets these to the
// slower intro-screen speed; startTileReveal() speeds them up for level starts.
int revealStepInterval = 2;
int revealTilesPerTick = 1;

// Set true whenever a level (re)starts so the tile-wipe plays over it
bool showLevelIntro = false;

void initTiles(bool randomize = true) {
	for (int r = 0; r < GRID_ROWS; r++) {
		for (int c = 0; c < GRID_COLS; c++) {
			tileScale[r][c] = 1.0f; // Start at full size
			tileSinking[r][c] = false; // Not sinking yet
		}
	}
	int idx = 0;
	for (int r = 0; r < GRID_ROWS; r++) {
		for (int c = 0; c < GRID_COLS; c++) {
			revealOrder[idx++] = r * GRID_COLS + c;
		}
	}
	if (randomize) {
		for (int i = idx - 1; i > 0; i--) {
			int j = rand() % (i + 1);
			int temp = revealOrder[i];
			revealOrder[i] = revealOrder[j];
			revealOrder[j] = temp;
		}
	}
	revealIndex = 0;
	frameCounter = 0;
	introDone = false;
	revealStepInterval = 2;
	revealTilesPerTick = 1;
}

void updateReveal() {
	if (introDone) return;
	frameCounter++;

	// Trigger new tiles to start shrinking based on the interval
	if (frameCounter % revealStepInterval == 0) {
		for (int i = 0; i < revealTilesPerTick && revealIndex < GRID_ROWS * GRID_COLS; i++) {
			int pos = revealOrder[revealIndex];
			int r = pos / GRID_COLS;
			int c = pos % GRID_COLS;
			tileSinking[r][c] = true; // Trigger the sink animation
			revealIndex++;
		}
	}

	// Animate all sinking tiles every frame
	bool allDone = (revealIndex >= GRID_ROWS * GRID_COLS); // Assume done if all are triggered

	for (int r = 0; r < GRID_ROWS; r++) {
		for (int c = 0; c < GRID_COLS; c++) {
			if (tileSinking[r][c] && tileScale[r][c] > 0.0f) {
				tileScale[r][c] -= 0.1f; // Shrink rate (reduce for slower, increase for faster)
				if (tileScale[r][c] <= 0.0f) {
					tileScale[r][c] = 0.0f;
					tileSinking[r][c] = false; // Finished shrinking
				}
			}
			// If any tile is still visible (scale > 0), we aren't done yet
			if (tileScale[r][c] > 0.0f) {
				allDone = false;
			}
		}
	}

	if (allDone) {
		introDone = true;
	}
}

// Call this any time a level starts/restarts to trigger the tile-wipe over it
void startTileReveal() {
	initTiles(false); // ordered: row by row, left to right
	showLevelIntro = true;
}

void drawTiles() {
	for (int r = 0; r < GRID_ROWS; r++) {
		for (int c = 0; c < GRID_COLS; c++) {
			if (tileScale[r][c] > 0.0f) {
				if ((r + c) % 2 == 0)
					iSetColor(52, 30, 20);   // bright ember orange
				else
					iSetColor(26, 14, 9);    // darker fire-red shade

				// Calculate scaled dimensions
				float w = TILE_W * tileScale[r][c];
				float h = TILE_H * tileScale[r][c];

				// Calculate offset to shrink towards the center
				float dx = (TILE_W - w) / 2.0f;
				float dy = (TILE_H - h) / 2.0f;

				// Draw the scaled and centered rectangle
				iFilledRectangle(c * TILE_W + dx, screenHeight - (r + 1) * TILE_H + dy, w, h);
			}
		}
	}
}

// ---- Main Menu Buttons ----
Button newGameBtn = { 306, 332, 484, 370 };
Button loadGameBtn = { 306, 284, 484, 315 };
Button settingBtn = { 306, 234, 484, 260 };
Button creditsBtn = { 306, 183, 484, 218 };
Button exitBtn = { 306, 138, 484, 170 };

// ---- Settings Screen Buttons ----
Button soundOnBtn = { 122, 204, 360, 264 };
Button soundOffBtn = { 122, 130, 360, 185 };

// ---- Load Game Screen Buttons ----
Button level1Btn = { 284, 293, 429, 335 };
Button level2Btn = { 284, 246, 429, 281 };
Button level3Btn = { 284, 199, 429, 234 };
Button level4Btn = { 284, 149, 429, 184 };
Button level5Btn = { 284, 99, 429, 133 };

// ===== Helpers: starting a level / reading the current run's score =====
int pendingLevel = 1; // level to start once the player has typed their name
int runPoints = 0;    // last known points of the current run (used inside bonus stages)

void beginLevel(int lv) {
	cancelBgSlide();
	switchToLevel(lv);
	currentState = PLAYING;
	runPoints = 0;
	switch (lv) {
	case 1: initLevel1(); break;
	case 2: initLevel2(); break;
	case 3: initLevel3(); break;
	case 4: initLevel4(); break;
	case 5: initLevel5(); break;
	}
	startTileReveal();
}

// If the player has no name yet, ask for it first, then start the level.
void requestStart(int lv) {
	if (playerName.empty()) {
		enteredName = "";
		nameEntryPurpose = 2;
		pendingLevel = lv;
		currentState = NAME_ENTRY;
	}
	else {
		beginLevel(lv);
	}
}

int getCurrentPoints() {
	switch (currentLevel) {
	case 1: runPoints = (int)playerPoints; break;
	case 10: runPoints = (int)playerPoints; break; // level 1 sub level shares level 1's points
	case 11: runPoints = (int)l2s1Points; break;   // level 2 sub level 1
	case 12: runPoints = (int)l2s2Points; break;   // level 2 sub level 2
	case 2: runPoints = (int)playerPoints2; break;
	case 3: runPoints = (int)playerPoints3; break;
	case 4: runPoints = (int)playerPoints4; break;
	case 5: runPoints = (int)playerPoints5; break;
	case 6: runPoints = (int)playerPointsSub1; break;
	case 13: runPoints = (int)playerPoints5S1; break; // level 5 sub level 1
	case 14: runPoints = (int)playerPoints5S2; break; // level 5 sub level 2 (final round)
	}
	return runPoints;
}

bool isCurrentGameOver() {
	switch (currentLevel) {
	case 1: return isGameOver;
	case 2: return isGameOver2;
	case 3: return isGameOver3;
	case 4: return isGameOver4;
	case 5: return isGameOver5;
	case 6: return isGameOverSub1;
	case 7: return isGameOverSub2;
	case 8: return isGameOver4S1;
	case 9: return isGameOver4S2;
	case 10: return l1s1IsGameOver;
	case 11: return l2s1IsGameOver;
	case 12: return l2s2IsGameOver;
	case 13: return isGameOver5S1;
	case 14: return isGameOver5S2;
	}
	return false;
}

void saveRunScore() {
	int pts = getCurrentPoints();
	if (pts <= 0) return;
	if (playerName.empty()) playerName = makeUniquePlayerName();
	saveScore(playerName, pts);
}

static void drawExitButton() {
	int btnX = 600;
	int btnY = 440;
	iSetColor(0, 0, 0);
	iText(btnX + 15, btnY + 10, " Exit", GLUT_BITMAP_HELVETICA_10);
}

void iDraw() {
	iClear();

	if (currentState == INTRO) {
		iShowBMP(0, 0, IMG_MAIN_MENU);
		drawTiles();
	}
	else if (currentState == MAIN_MENU) {
		iShowBMP(0, 0, IMG_MAIN_MENU);
	}
	else if (currentState == SETTINGS) {
		iShowBMP(0, 0, IMG_SOUND_OPT);
	}
	else if (currentState == SOUND_AND_SCORE) {
		iShowBMP(0, 0, IMG_SOUND_AND_SCORE);
	}
	else if (currentState == SCORE_BOARD) {
		iShowBMP(0, 0, IMG_SCORE);

		// Row/column geometry measured directly from score.bmp:
		//   rowY[i]         - vertical centre of entry bar i (0 = top row,
		//                     just under the "Name"/"Points" header bar).
		//   NAME_CENTER_X   - horizontal centre of the empty text area inside
		//                     the Name column's bar (the flat stretch between
		//                     the red diamond icon and the tip the bar
		//                     tapers to on the right).
		//   POINTS_CENTER_X - the same, for the Points column's bar.
		const int rowY[5] = { 353, 281, 210, 139, 68 };
		const int NAME_CENTER_X = 220;
		const int POINTS_CENTER_X = 568;

		for (int i = 0; i < 5 && i < (int)scoreList.size(); i++) {
			char rankName[64], pts[32];
			sprintf(rankName, "%d.  %s", i + 1, scoreList[i].name.c_str());
			sprintf(pts, "%d", scoreList[i].points);

			// True pixel width of each string at this font, so it can be
			// centred exactly in its column regardless of name length -
			// instead of always starting at the same fixed x.
			int nameW = 0;
			for (char* c = rankName; *c; c++) nameW += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, (unsigned char)*c);
			int ptsW = 0;
			for (char* c = pts; *c; c++) ptsW += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, (unsigned char)*c);

			int nameX = NAME_CENTER_X - nameW / 2;
			int ptsX = POINTS_CENTER_X - ptsW / 2;
			int textY = rowY[i] - 6;   // baseline sits a touch below the bar's
			// vertical centre so the glyphs (which
			// extend upward from the baseline)
			// land visually centred in the bar

			iSetColor(255, 255, 255);
			iText(nameX, textY, rankName, GLUT_BITMAP_HELVETICA_18);
			iText(nameX + 1, textY, rankName, GLUT_BITMAP_HELVETICA_18);
			iText(ptsX, textY, pts, GLUT_BITMAP_HELVETICA_18);
			iText(ptsX + 1, textY, pts, GLUT_BITMAP_HELVETICA_18);
		}

		if (!playerName.empty()) {
			for (int i = 5; i < (int)scoreList.size(); i++) {
				if (scoreList[i].name == playerName) {
					char mine[96];
					sprintf(mine, "Your best: #%d  %s  -  %d pts", i + 1, playerName.c_str(), scoreList[i].points);
					iSetColor(60, 40, 90);
					iText(200, 22, mine, GLUT_BITMAP_HELVETICA_12);
					iText(201, 22, mine, GLUT_BITMAP_HELVETICA_12);
					break;
				}
			}
		}
	}
	else if (currentState == NAME_ENTRY) {
		// Dark night-sky navy backdrop, same family as mainmenu/sound_options/
		// score/load_game art, instead of the old flat yellow screen.
		iSetColor(10, 9, 26);
		iFilledRectangle(0, 0, screenWidth, screenHeight);

		// Glowing ember-orange border behind the panel (drawn slightly larger
		// than the panel itself so a border ring shows around it), then the
		// dark maroon/black panel on top - the same "dark bar, red-orange
		// glow edge" look used by the bars on score.bmp / sound_options.bmp.
		iSetColor(215, 70, 20);
		iFilledRectangle(56, 86, 588, 324);
		iSetColor(26, 9, 14);
		iFilledRectangle(60, 90, 580, 320);

		iSetColor(255, 130, 40);
		if (nameEntryPurpose == 1) {
			iText(190, 365, "YOU COMPLETED ZENDROID!", GLUT_BITMAP_HELVETICA_18);
			char scoreLine[64];
			sprintf(scoreLine, "Total Points: %d", finalGameScore);
			iText(275, 320, scoreLine, GLUT_BITMAP_HELVETICA_18);
		}
		else {
			iText(250, 350, "WELCOME, PLAYER!", GLUT_BITMAP_HELVETICA_18);
		}

		iSetColor(240, 210, 185);
		iText(200, 260, "Enter your name:", GLUT_BITMAP_HELVETICA_12);

		// Name input box: same dark-panel-with-ember-border treatment, scaled
		// down to button size.
		iSetColor(215, 70, 20);
		iFilledRectangle(197, 202, 306, 42);
		iSetColor(22, 8, 8);
		iFilledRectangle(200, 205, 300, 36);
		iSetColor(255, 190, 120);
		string display = enteredName + "_";
		iText(212, 217, (char*)display.c_str(), GLUT_BITMAP_HELVETICA_18);

		iSetColor(205, 150, 110);
		if (nameEntryPurpose == 1)
			iText(210, 150, "Press ENTER to save your score", GLUT_BITMAP_HELVETICA_12);
		else
			iText(225, 150, nameEntryPurpose == 2 ? "Press ENTER to start" : "Press ENTER to continue", GLUT_BITMAP_HELVETICA_12);
	}
	else if (currentState == CREDITS) {
		iShowBMP(0, 0, IMG_CREDITS);
	}
	else if (currentState == LOAD_GAME) {
		iShowBMP(0, 0, IMG_LOAD_GAME);
	}
	else if (currentState == PLAYING && bgSlideActive) {
		// Entering a sub-level: connected backgrounds slide across (see bg_slide.h)
		drawBgSlide();
	}
	else if (currentState == PLAYING) {
		switch (currentLevel) {
		case 1: drawLevel1(); break;
		case 2: drawLevel2(); break;
		case 3: drawLevel3(); break;
		case 4: drawLevel4(); break;
		case 5: drawLevel5(); break;
		case 6: drawLevel3Sub1(); break;
		case 7: drawLevel3Sub2(); break;
		case 8: drawLevel4Sub1(); break;
		case 9: drawLevel4Sub2(); break;
		case 10: drawLevel1Sub1(); break;
		case 11: drawLevel2Sub1(); break;
		case 12: drawLevel2Sub2(); break;
		case 13: drawLevel5S1(); break;
		case 14: drawLevel5S2(); break;
		}
		if (showLevelIntro) drawTiles();
	}
	if (currentState != INTRO && currentState != MAIN_MENU) {
		drawExitButton();
	}
}

void iMouseMove(int mx, int my) {}
void iPassiveMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) {
	if (currentState == PLAYING) {
		if (showLevelIntro || bgSlideActive) return;

		if (currentLevel == 1) {
			if (isPaused) {
				if (isInside(pauseResumeBtn, mx, my)) { isPaused = false; return; }
				else if (isInside(pauseRestartBtn, mx, my)) { initLevel1(); startTileReveal(); isPaused = false; return; }
				else if (isInside(pauseExitBtn, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver) {
				if (isInside(gameOverRestartBtn, mx, my)) { initLevel1(); startTileReveal(); isGameOver = false; return; }
				else if (isInside(gameOverExitBtn, mx, my)) { goToLoadGame(); return; }
			}
		}
		else if (currentLevel == 10) {
			if (l1s1IsPaused) {
				if (isInside(l1s1PauseResumeBtn, mx, my)) { l1s1IsPaused = false; return; }
				else if (isInside(l1s1PauseRestartBtn, mx, my)) { initLevel1Sub1(); startTileReveal(); l1s1IsPaused = false; return; }
				else if (isInside(l1s1PauseExitBtn, mx, my)) { goToLoadGame(); return; }
			}
			else if (l1s1IsGameOver) {
				if (isInside(l1s1GameOverRestartBtn, mx, my)) { initLevel1Sub1(); startTileReveal(); l1s1IsGameOver = false; return; }
				else if (isInside(l1s1GameOverExitBtn, mx, my)) { goToLoadGame(); return; }
			}
		}
		else if (currentLevel == 11) {
			if (l2s1IsPaused) {
				if (isInside(l2s1PauseResumeBtn, mx, my)) { l2s1IsPaused = false; return; }
				else if (isInside(l2s1PauseRestartBtn, mx, my)) { initLevel2Sub1(); startTileReveal(); l2s1IsPaused = false; return; }
				else if (isInside(l2s1PauseExitBtn, mx, my)) { goToLoadGame(); return; }
			}
			else if (l2s1IsGameOver) {
				if (isInside(l2s1GameOverRestartBtn, mx, my)) { initLevel2Sub1(); startTileReveal(); l2s1IsGameOver = false; return; }
				else if (isInside(l2s1GameOverExitBtn, mx, my)) { goToLoadGame(); return; }
			}
		}
		else if (currentLevel == 12) {
			if (l2s2IsPaused) {
				if (isInside(l2s2PauseResumeBtn, mx, my)) { l2s2IsPaused = false; return; }
				else if (isInside(l2s2PauseRestartBtn, mx, my)) { initLevel2Sub2(); startTileReveal(); l2s2IsPaused = false; return; }
				else if (isInside(l2s2PauseExitBtn, mx, my)) { goToLoadGame(); return; }
			}
			else if (l2s2IsGameOver) {
				if (isInside(l2s2GameOverRestartBtn, mx, my)) { initLevel2Sub2(); startTileReveal(); l2s2IsGameOver = false; return; }
				else if (isInside(l2s2GameOverExitBtn, mx, my)) { goToLoadGame(); return; }
			}
		}
		else if (currentLevel == 2) {
			if (isPaused2) {
				if (isInside(pauseResumeBtn2, mx, my)) { isPaused2 = false; return; }
				else if (isInside(pauseRestartBtn2, mx, my)) { initLevel2(); startTileReveal(); isPaused2 = false; return; }
				else if (isInside(pauseExitBtn2, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver2) {
				if (isInside(gameOverRestartBtn2, mx, my)) { initLevel2(); startTileReveal(); isGameOver2 = false; return; }
				else if (isInside(gameOverExitBtn2, mx, my)) { goToLoadGame(); return; }
			}
		}
		else if (currentLevel == 3) {
			if (isPaused3) {
				if (isInside(pauseResumeBtn3, mx, my)) { isPaused3 = false; return; }
				else if (isInside(pauseRestartBtn3, mx, my)) { initLevel3(); startTileReveal(); isPaused3 = false; return; }
				else if (isInside(pauseExitBtn3, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver3) {
				if (isInside(gameOverRestartBtn3, mx, my)) { initLevel3(); startTileReveal(); isGameOver3 = false; return; }
				else if (isInside(gameOverExitBtn3, mx, my)) { goToLoadGame(); return; }
			}
		}
		else if (currentLevel == 4) {
			if (isPaused4) {
				if (isInside(pauseResumeBtn4, mx, my)) { isPaused4 = false; return; }
				else if (isInside(pauseRestartBtn4, mx, my)) { initLevel4(); startTileReveal(); isPaused4 = false; return; }
				else if (isInside(pauseExitBtn4, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver4) {
				if (isInside(gameOverRestartBtn4, mx, my)) { initLevel4(); startTileReveal(); isGameOver4 = false; return; }
				else if (isInside(gameOverExitBtn4, mx, my)) { goToLoadGame(); return; }
			}
			else {
				handleMouseClickLevel4(button, state);
			}
		}
		else if (currentLevel == 6) {
			if (isPausedSub1) {
				if (isInside(pauseResumeBtnSub1, mx, my)) { isPausedSub1 = false; return; }
				else if (isInside(pauseRestartBtnSub1, mx, my)) { initLevel3Sub1(); startTileReveal(); isPausedSub1 = false; return; }
				else if (isInside(pauseExitBtnSub1, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOverSub1) {
				if (isInside(gameOverRestartBtnSub1, mx, my)) { initLevel3Sub1(); startTileReveal(); isGameOverSub1 = false; return; }
				else if (isInside(gameOverExitBtnSub1, mx, my)) { goToLoadGame(); return; }
			}
		}
		else if (currentLevel == 7) {
			if (isPausedSub2) {
				if (isInside(pauseResumeBtnSub2, mx, my)) { isPausedSub2 = false; return; }
				else if (isInside(pauseRestartBtnSub2, mx, my)) { initLevel3Sub2(); startTileReveal(); isPausedSub2 = false; return; }
				else if (isInside(pauseExitBtnSub2, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOverSub2) {
				if (isInside(gameOverRestartBtnSub2, mx, my)) { initLevel3Sub2(); startTileReveal(); isGameOverSub2 = false; return; }
				else if (isInside(gameOverExitBtnSub2, mx, my)) { goToLoadGame(); return; }
			}
		}
		else if (currentLevel == 8) {
			if (isPaused4S1) {
				if (isInside(pauseResumeBtn4S1, mx, my)) { isPaused4S1 = false; return; }
				else if (isInside(pauseRestartBtn4S1, mx, my)) { initLevel4Sub1(); startTileReveal(); isPaused4S1 = false; return; }
				else if (isInside(pauseExitBtn4S1, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver4S1) {
				if (isInside(gameOverRestartBtn4S1, mx, my)) { initLevel4Sub1(); startTileReveal(); isGameOver4S1 = false; return; }
				else if (isInside(gameOverExitBtn4S1, mx, my)) { goToLoadGame(); return; }
			}
			else {
				handleMouseClickLevel4Sub1(button, state);
			}
		}
		else if (currentLevel == 9) {
			if (isPaused4S2) {
				if (isInside(pauseResumeBtn4S2, mx, my)) { isPaused4S2 = false; return; }
				else if (isInside(pauseRestartBtn4S2, mx, my)) { initLevel4Sub2(); startTileReveal(); isPaused4S2 = false; return; }
				else if (isInside(pauseExitBtn4S2, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver4S2) {
				if (isInside(gameOverRestartBtn4S2, mx, my)) { initLevel4Sub2(); startTileReveal(); isGameOver4S2 = false; return; }
				else if (isInside(gameOverExitBtn4S2, mx, my)) { goToLoadGame(); return; }
			}
			else {
				handleMouseClickLevel4Sub2(button, state);
			}
		}
		else if (currentLevel == 5) {
			if (isPaused5) {
				if (isInside(pauseResumeBtn5, mx, my)) { isPaused5 = false; return; }
				else if (isInside(pauseRestartBtn5, mx, my)) { initLevel5(); startTileReveal(); isPaused5 = false; return; }
				else if (isInside(pauseExitBtn5, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver5) {
				if (isInside(gameOverRestartBtn5, mx, my)) { initLevel5(); startTileReveal(); isGameOver5 = false; return; }
				else if (isInside(gameOverExitBtn5, mx, my)) { goToLoadGame(); return; }
			}
			else {
				handleMouseClickLevel5(button, state);
			}
		}
		else if (currentLevel == 13) {
			if (isPaused5S1) {
				if (isInside(pauseResumeBtn5S1, mx, my)) { isPaused5S1 = false; return; }
				else if (isInside(pauseRestartBtn5S1, mx, my)) { initLevel5S1(); startTileReveal(); isPaused5S1 = false; return; }
				else if (isInside(pauseExitBtn5S1, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver5S1) {
				if (isInside(gameOverRestartBtn5S1, mx, my)) { initLevel5S1(); startTileReveal(); isGameOver5S1 = false; return; }
				else if (isInside(gameOverExitBtn5S1, mx, my)) { goToLoadGame(); return; }
			}
			else {
				handleMouseClickLevel5S1(button, state);
			}
		}
		else if (currentLevel == 14) {
			if (isPaused5S2) {
				if (isInside(pauseResumeBtn5S2, mx, my)) { isPaused5S2 = false; return; }
				else if (isInside(pauseRestartBtn5S2, mx, my)) { initLevel5S2(); startTileReveal(); isPaused5S2 = false; return; }
				else if (isInside(pauseExitBtn5S2, mx, my)) { goToLoadGame(); return; }
			}
			else if (isGameOver5S2) {
				if (isInside(gameOverRestartBtn5S2, mx, my)) { initLevel5S2(); startTileReveal(); isGameOver5S2 = false; return; }
				else if (isInside(gameOverExitBtn5S2, mx, my)) { goToLoadGame(); return; }
			}
			else if (gameWon5S2) {
				if (isInside(winExitBtn5S2, mx, my)) { goToLoadGame(); return; }
			}
			else {
				handleMouseClickLevel5S2(button, state);
			}
		}
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (mx >= 600 && mx <= 660 && my >= 440 && my <= 462) {
			if (currentState == SCORE_BOARD || currentState == SETTINGS) {
				currentState = SOUND_AND_SCORE;
			}
			else {
				goToMainMenu();
			}
		}

		if (currentState == MAIN_MENU) {
			if (isInside(newGameBtn, mx, my)) {
				requestStart(1);
			}
			else if (isInside(loadGameBtn, mx, my)) {
				currentState = LOAD_GAME;
			}
			else if (isInside(settingBtn, mx, my)) {
				if (playerName.empty()) {
					enteredName = "";
					nameEntryPurpose = 0;
					currentState = NAME_ENTRY;
				}
				else {
					currentState = SOUND_AND_SCORE;
				}
			}
			else if (isInside(creditsBtn, mx, my)) {
				currentState = CREDITS;
			}
			else if (isInside(exitBtn, mx, my)) {
				exit(0);
			}
		}
		else if (currentState == SETTINGS) {
			if (isInside(soundOnBtn, mx, my)) {
				soundOn = true;
				mciSendString("play bgsong repeat", NULL, 0, NULL);
				currentState = SOUND_AND_SCORE;
			}
			else if (isInside(soundOffBtn, mx, my)) {
				soundOn = false;
				mciSendString("stop bgsong", NULL, 0, NULL);
				currentState = SOUND_AND_SCORE;
			}
		}
		else if (currentState == SOUND_AND_SCORE) {
			if (isInside(soundBtnSAS, mx, my)) {
				currentState = SETTINGS;
			}
			else if (isInside(scoreBtnSAS, mx, my)) {
				loadScores();
				currentState = SCORE_BOARD;
			}
		}
		else if (currentState == LOAD_GAME) {
			if (isInside(level1Btn, mx, my)) {
				requestStart(1);
			}
			else if (isInside(level2Btn, mx, my)) {
				requestStart(2);
			}
			else if (isInside(level3Btn, mx, my)) {
				requestStart(3);
			}
			else if (isInside(level4Btn, mx, my)) {
				requestStart(4);
			}
			else if (isInside(level5Btn, mx, my)) {
				requestStart(5);
			}
		}
	}
}

void handleNameEntryInput() {
	static bool keyWasDown[256] = { false };

	for (int k = 0; k < 256; k++) {
		bool isDown = (isKeyPressed(k) != 0);
		bool justPressed = isDown && !keyWasDown[k];
		keyWasDown[k] = isDown;
		if (!justPressed) continue;

		if (k == 13) { // ENTER
			string finalName = enteredName.empty() ? makeUniquePlayerName() : enteredName;
			playerName = finalName;
			if (nameEntryPurpose == 1) {
				saveScore(playerName, finalGameScore);
				currentState = SCORE_BOARD;
			}
			else if (nameEntryPurpose == 2) {
				beginLevel(pendingLevel);
			}
			else {
				currentState = SOUND_AND_SCORE;
			}
			enteredName = "";
		}
		else if (k == 8) { // BACKSPACE
			if (!enteredName.empty()) enteredName.erase(enteredName.size() - 1);
		}
		else if ((k >= 'A' && k <= 'Z') || (k >= 'a' && k <= 'z') || (k >= '0' && k <= '9') || k == ' ') {
			if ((int)enteredName.size() < MAX_NAME_LEN) enteredName += (char)k;
		}
	}
}

void fixedUpdate() {
	shotFiredThisFrame = false; // reset once per tick, before any level update runs

	if (currentState == INTRO) {
		updateReveal();
		if (introDone) goToMainMenu();
		return;
	}

	static GameState prevState = INTRO;
	static bool overSaved = false;
	if (currentState == PLAYING) {
		getCurrentPoints();
		bool over = isCurrentGameOver();
		if (over && !overSaved) { saveRunScore(); overSaved = true; }
		else if (!over) overSaved = false;
	}
	if (prevState == PLAYING && currentState != PLAYING && currentState != NAME_ENTRY) {
		if (!overSaved) saveRunScore();
		overSaved = false;
	}
	prevState = currentState;

	static bool spaceWasDown = false;
	bool spaceIsDown = (isKeyPressed(32) != 0);
	bool spaceJustPressed = spaceIsDown && !spaceWasDown;
	spaceWasDown = spaceIsDown;

	if (currentState == PLAYING && !showLevelIntro && !bgSlideActive && spaceJustPressed) {
		if (currentLevel == 1 && !isGameOver) isPaused = !isPaused;
		else if (currentLevel == 2 && !isGameOver2) isPaused2 = !isPaused2;
		else if (currentLevel == 3 && !isGameOver3) isPaused3 = !isPaused3;
		else if (currentLevel == 4 && !isGameOver4) isPaused4 = !isPaused4;
		else if (currentLevel == 5 && !isGameOver5) isPaused5 = !isPaused5;
		else if (currentLevel == 6 && !isGameOverSub1) isPausedSub1 = !isPausedSub1;
		else if (currentLevel == 7 && !isGameOverSub2) isPausedSub2 = !isPausedSub2;
		else if (currentLevel == 8 && !isGameOver4S1) isPaused4S1 = !isPaused4S1;
		else if (currentLevel == 9 && !isGameOver4S2) isPaused4S2 = !isPaused4S2;
		else if (currentLevel == 10 && !l1s1IsGameOver) l1s1IsPaused = !l1s1IsPaused;
		else if (currentLevel == 11 && !l2s1IsGameOver) l2s1IsPaused = !l2s1IsPaused;
		else if (currentLevel == 12 && !l2s2IsGameOver) l2s2IsPaused = !l2s2IsPaused;
		else if (currentLevel == 13 && !isGameOver5S1) isPaused5S1 = !isPaused5S1;
		else if (currentLevel == 14 && !isGameOver5S2) isPaused5S2 = !isPaused5S2;
	}

	if (currentState == CREDITS || currentState == LOAD_GAME || currentState == SETTINGS ||
		currentState == SOUND_AND_SCORE || currentState == SCORE_BOARD || currentState == PLAYING) {
		if (isKeyPressed(27)) {
			goToMainMenu();
		}
	}

	if (currentState == NAME_ENTRY && nameEntryPurpose != 1 && isKeyPressed(27)) {
		goToMainMenu();
	}

	if (currentState == PLAYING && showLevelIntro) {
		updateReveal();
		if (introDone) showLevelIntro = false;
		return;
	}

	// Entering a sub-level: slide the connected backgrounds, then the level starts.
	if (currentState == PLAYING && bgSlideActive) {
		updateBgSlide();
		return;
	}

	if (currentState == PLAYING) {
		switch (currentLevel) {
		case 1:
			if (!isPaused && !isGameOver) {
				updateLevel1();
				if (enterLevel2) {
					enterLevel2 = false;
					switchToLevel(10);
					initLevel1Sub1(false);
					startBgSlide(1, 10);
				}
			}
			break;
		case 10:
			if (!l1s1IsPaused && !l1s1IsGameOver) {
				updateLevel1Sub1();
				if (l1s1EnterLevel2) {
					l1s1EnterLevel2 = false;
					switchToLevel(2);
					initLevel2(false);
					startTileReveal();
				}
			}
			break;
		case 2:
			if (!isPaused2 && !isGameOver2) {
				updateLevel2();
				if (enterLevel3) {
					enterLevel3 = false;
					switchToLevel(11);
					initLevel2Sub1(false);
					startBgSlide(2, 11);
				}
			}
			break;
		case 11:
			if (!l2s1IsPaused && !l2s1IsGameOver) {
				updateLevel2Sub1();
				if (l2s1EnterSub2) {
					l2s1EnterSub2 = false;
					switchToLevel(12);
					initLevel2Sub2(false);
					startBgSlide(11, 12);
				}
			}
			break;
		case 12:
			if (!l2s2IsPaused && !l2s2IsGameOver) {
				updateLevel2Sub2();
				if (l2s2EnterLevel3) {
					l2s2EnterLevel3 = false;
					switchToLevel(3);
					initLevel3(false);
					startTileReveal();
				}
			}
			break;
		case 3:
			if (!isPaused3 && !isGameOver3) {
				updateLevel3();
				if (enterLevel4) {
					switchToLevel(6);
					initLevel3Sub1(false);
					startBgSlide(3, 6);
				}
			}
			break;
		case 6:
			if (!isPausedSub1 && !isGameOverSub1) {
				updateLevel3Sub1();
				if (enterLevel4FromSub1) {
					switchToLevel(7);
					initLevel3Sub2(false);
					startBgSlide(6, 7);
				}
			}
			break;
		case 7:
			if (!isPausedSub2 && !isGameOverSub2) {
				updateLevel3Sub2();
				if (enterLevel4FromSub2) {
					switchToLevel(4);
					initLevel4(false);
					startTileReveal();
				}
			}
			break;
		case 4:
			if (!isPaused4 && !isGameOver4) {
				updateLevel4();
				if (enterLevel5) {
					switchToLevel(8);
					initLevel4Sub1(false);
					startBgSlide(4, 8);
				}
			}
			break;
		case 8:
			if (!isPaused4S1 && !isGameOver4S1) {
				updateLevel4Sub1();
				if (enterLevel4Sub2) {
					switchToLevel(9);
					initLevel4Sub2(false);
					startBgSlide(8, 9);
				}
			}
			break;
		case 9:
			if (!isPaused4S2 && !isGameOver4S2) {
				updateLevel4Sub2();
				if (enterLevel5FromSub2) {
					switchToLevel(5);
					initLevel5(false);
					startTileReveal();
				}
			}
			break;
		case 5:
			if (!isPaused5 && !isGameOver5) {
				updateLevel5();
				if (enterLevel5Sub1) {
					switchToLevel(13);
					initLevel5S1(false);
					startBgSlide(5, 13);
				}
			}
			break;
		case 13:
			if (!isPaused5S1 && !isGameOver5S1) {
				updateLevel5S1();
				if (enterLevel5Sub2) {
					switchToLevel(14);
					initLevel5S2(false);
					startBgSlide(13, 14);
				}
			}
			break;
		case 14:
			if (!isPaused5S2 && !isGameOver5S2) {
				updateLevel5S2();
				// gameWon5S2 latches inside level5_sub2.h itself: it is set ONLY
				// after the 3 dragons + 3 golden balls have brought in the Boss
				// Dragon and the boss has been killed (death animation done).
				// updateLevel5S2() then stops advancing the game and
				// drawLevel5S2() shows win.png every frame from then on -
				// no further state change is needed here.
				if (gameWon5S2) {
					finalGameScore = playerPoints5S2;
				}
			}
			break;
		}
	}

	if (currentState == NAME_ENTRY) {
		handleNameEntryInput();
	}

	resolveJumpSfx(); // by now shotFiredThisFrame reflects whether a shot also happened this tick
}

int main() {
	srand((unsigned int)time(0));
	initTiles();
	loadScores();

	// Play background music if available
	mciSendString("open \"" BGM_MENU "\" alias bgsong", NULL, 0, NULL);
	mciSendString("setaudio bgsong volume to 500", NULL, 0, NULL); // 50% of full (MCI volume range is 0-1000)
	mciSendString("play bgsong repeat", NULL, 0, NULL);

	// Initialize iGraphics window
	iInitialize(screenWidth, screenHeight, "Zendroid");
	iStart();

	// Explicit main loop call in case iStart() does not block execution
	glutMainLoop();

	return 0;
}