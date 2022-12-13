#ifndef GAME_DISPLAY_H
#define GAME_DISPLAY_H

#include "menu.h"

// #define CHAR_PLAYER_BAG 1
// #define CHAR_DOOR_PLAYER 3
enum Game_mapIds {
	CHAR_BLANK, CHAR_WALL, CHAR_KEY, CHAR_DOOR, CHAR_LOCK, CHAR_PLAYER
};

#define NUM_ELEMENTS 6

void GameDisplay_setup(void);
void GameDisplay_init(void);
void GameDisplay_cleanUp(void);

void GameDisplay_displayKeyboard();
void GameDisplay_displayName(char* name);
player_recording* GameDisplay_displayRanking(player_recording* rankings, int placement);
void GameDisplay_displayEnd();
void GameDisplay_endCursor(bool isQuit);

#endif
