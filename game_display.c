#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "game_display.h"
#include "game.h"
#include "lcd_screen.h"
#include "menu.h"

#define NUM_CSTM_CHARS 9
static customChar_t customChar[NUM_CSTM_CHARS] = {
	{{0b00001, 0b00001, 0b00101, 0b01001, 0b11111, 0b01000, 0b00100, 0b00000}}, // Enter
	{{0b00010, 0b00110, 0b01110, 0b11110, 0b01110, 0b00110, 0b00010, 0b00000}}, // Backspace
	{{0b01110, 0b10001, 0b10001, 0b01110, 0b00100, 0b01100, 0b00100, 0b01100}}, // Key
	{{0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001}}, // Door
	{{0b01110, 0b10001, 0b10001, 0b10001, 0b11111, 0b11111, 0b11011, 0b11111}}, // Lock
	{{0b00000, 0b00000, 0b01110, 0b01000, 0b01110, 0b01110, 0b01010, 0b00000}}, // Player
	{{0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111}}, // Hidden Door
	{{0b01110, 0b10001, 0b00001, 0b00001, 0b11111, 0b11111, 0b11011, 0b11111}}, // Unlock
	{{0b00000, 0b00000, 0b01110, 0b11000, 0b11110, 0b11110, 0b01010, 0b00000}}  // Player Bag
};

static int MAP_ELEMENT[NUM_ELEMENTS] = {' ', 0xFF, 2, 3, 4, 5};

#define NAME_OFFSET 5

char keyboard[LCD_ROW][LCD_COL] = {
	{"Name:               "},
	{"     QWERTYUIOP     "},
	{' ',' ',' ',' ',' ','A','S','D','F','G','H','J','K','L',0,' ',' ',' ',' ',' '},
	{' ',' ',' ',' ',' ','Z','X','C','V','_','_','B','N','M',1,' ',' ',' ',' ',' '}
};

// Game Display Threading
static void* GameDisplay_thread(void* _);
static pthread_t GameDisplay_threadId;
static bool isDoneRunning = false;

void GameDisplay_loadCstmChars()
{
	for (int i = 0; i < 6; i++) {
		LcdScreen_loadCstmChar(i, customChar[i]);
	}
}

void GameDisplay_setup(void)
{
	LcdScreen_setup(false);
	GameDisplay_loadCstmChars();
}

void GameDisplay_init(void)
{
	isDoneRunning = false;
	pthread_create(&GameDisplay_threadId, NULL, &GameDisplay_thread, NULL);
}

void GameDisplay_cleanUp(void)
{
	isDoneRunning = true;
	pthread_join(GameDisplay_threadId, NULL);

	LcdScreen_clear();
}

static void GameDisplay_displayArray(int rows, int cols, int map[rows][cols])
{
	char buff[LCD_COL] = {0};
	for (int i = 0; i < LCD_ROW; i++) {
		for (int j = 0; j < LCD_COL; j++) {
			buff[j] = MAP_ELEMENT[map[i][j]];
		}
		LcdScreen_moveCursor(i, 0);
		LcdScreen_writeString(buff);
	}
}

static void GameDisplay_displayMap()
{
	room newRoom;
	Game_GetRoom(&newRoom);

	int currentRoom[MAP_ROW][MAP_COL];
	for(int i=0; i<MAP_COL; i++){
		currentRoom[0][i] = newRoom.row1[i];
		currentRoom[1][i] = newRoom.row2[i];
		currentRoom[2][i] = newRoom.row3[i];
		currentRoom[3][i] = newRoom.row4[i];
	}

	currentRoom[Game_Character_Row()][Game_Character_Col()] = CHAR_PLAYER;
	if (Game_isKeyPicked()) {
		LcdScreen_loadCstmChar(CHAR_PLAYER, customChar[8]);
	} else {
		LcdScreen_loadCstmChar(CHAR_PLAYER, customChar[5]);
	}

	if (Game_isUnlocked()) {
		LcdScreen_loadCstmChar(CHAR_LOCK, customChar[7]);
		LcdScreen_loadCstmChar(CHAR_DOOR, customChar[3]);
	} else {
		LcdScreen_loadCstmChar(CHAR_LOCK, customChar[4]);
		LcdScreen_loadCstmChar(CHAR_DOOR, customChar[6]);
	}

	GameDisplay_displayArray(MAP_ROW, MAP_COL, currentRoom);
	Game_FreeRoom(&newRoom);
}

void GameDisplay_displayKeyboard()
{
	for (int i = 0; i < LCD_ROW; i++) {
		LcdScreen_moveCursor(i, 0);
		LcdScreen_writeString(keyboard[i]);
	}
}

void GameDisplay_displayName(char* name)
{
	int length = strlen(name);

	for (int i = 0; i < (LCD_COL - NAME_OFFSET); i++) {
		if (i < length) {
			LcdScreen_placeChar(0, i + NAME_OFFSET, name[i]);
		} else {
			LcdScreen_placeChar(0, i + NAME_OFFSET, ' ');
		}
	}
}

player_recording* GameDisplay_displayRanking(player_recording* playerInfo, int placement)
{
	LcdScreen_clear();
	LcdScreen_moveCursor(0, 0);
	LcdScreen_writeString("----Leaderboard-----");

	for (int i = 0; i < 3; i++) {
		if (playerInfo == NULL) {
			return NULL;
		}

		placement++;
		char message[LCD_COL + 1] = {0};
		int min = (playerInfo->time)/60;
		int sec = (playerInfo->time)%60;
		snprintf(message, LCD_COL + 1, "%2d. %-10s %02d:%02d", placement, playerInfo->name, min, sec);

		LcdScreen_moveCursor(1 + i, 0);
		LcdScreen_writeString(message);
		playerInfo = playerInfo->next_player;
	}

	return playerInfo;
}

void GameDisplay_displayEnd(void)
{
	LcdScreen_moveCursor(0, 0);
	LcdScreen_writeString("------SUS MAZE------");
	LcdScreen_moveCursor(2, 0);
	LcdScreen_writeString("    Play    Quit    ");
}

void GameDisplay_endCursor(bool isQuit)
{
	if (!isQuit) {
		LcdScreen_moveCursor(2, 3);
	} else {
		LcdScreen_moveCursor(2, 11);
	}
}

static void* GameDisplay_thread(void* _)
{
	while(!isDoneRunning) {
		GameDisplay_displayMap();
		sleepForMs(1);
	}
	return NULL;
}
