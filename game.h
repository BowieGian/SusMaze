#ifndef GAME_H
#define GAME_H

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#include "timer.h"
#include "joystick_input.h"
#include "audioMixer.h"
#include "game_audio.h"

#define LCD_4X20

#ifdef LCD_4X20
#define MAP_ROW 4
#define MAP_COL 20
#endif

#define TUTORIAL_MAP_PATH "./source-file/mapfile/tutorial.txt"
#define MAP1_PATH "./source-file/mapfile/map1.txt"
#define MAP2_PATH "./source-file/mapfile/map2.txt"
#define MAP3_PATH "./source-file/mapfile/map3.txt"

typedef struct {
    int row;
    int col;
    bool isKeyFound;
} character;

#ifdef LCD_4X20
typedef struct {
    int* row1;
    int* row2;
    int* row3;
    int* row4;
    int key_row;
    int key_col;
    int exit_row;
    int exit_col;
    int lock_row;
    int lock_col;
} room;
#endif

enum MapElement
{
    EMPTY = 0,
    BLOCK,
    KEY,
    EXIT,
    LOCK,
    NUM_Elements, 
};

enum CharacterMove
{
    CHARACTER_UP = 0,
    CHARACTER_DOWN,
    CHARACTER_LEFT,
    CHARACTER_RIGHT,
    CHARACTER_RIGHT_UP,
    CHARACTER_RIGHT_DOWN,
    CHARACTER_LEFT_UP,
    CHARACTER_LEFT_DOWN,
    CHARACTER_CENTER,
    CHARACTER_NUMBER_DIRECTIONS      // Get the number of directions via the enum
};

void Game_init();
void Game_clear();

void Game_MoveCharacter(int direction);

int Game_Character_Row();
int Game_Character_Col();

bool Game_isKeyPicked();
bool Game_isUnlocked();
bool Game_isNextMap();

void Game_GetRoom(room* newMap);
void Game_FreeRoom(room* currentRoom);

bool Game_Allows_Operation();
bool Game_Is_Done();

void Game_PauseTheGame();
void Game_ResumeTheGame();
bool Game_IsAllMapDone();

#endif