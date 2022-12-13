#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#include "menu.h"
#include "timer.h"
#include "joystick_input.h"
#include "game.h"
#include "button.h"
#include "game_display.h"
#include "lcd_screen.h"

// '-' is delete
// '\' is enter
static char keyboard[3][10] = {
    {"QWERTYUIOP"},
    {"ASDFGHJKL/"},
    {"ZXCV__BNM-"}
};

static int cursor_row = 0;
static int cursor_col = 0;

static char name[MAX_NAME_SIZE];
static int NameBuff_IDX = 0;
static bool Name_Is_Entered = false;

static bool isMenuRunning = false;
//static bool quitShowRanking  = true;

static pthread_mutex_t cursorPosition_Mutex = PTHREAD_MUTEX_INITIALIZER;

static long long timer_start_time = 0;
static long long timer_stop_time = 0;

// header of linked list
static player_recording* record_list = NULL;
static int record_list_idx = 0;

static bool isQuit = false;

static void move_the_cursor(int next_row, int next_col){

    pthread_mutex_lock(&cursorPosition_Mutex);
    {
        cursor_row = next_row;
        cursor_col = next_col;
    }
    pthread_mutex_unlock(&cursorPosition_Mutex);
}



static void insertPlayer_intoList()
{
    player_recording* currentPlayer = malloc(sizeof(*currentPlayer));

    if(currentPlayer == NULL){
        printf("Run out of memory \n");
        exit(0);
    }

    // initilize the currentPlayer
    for(int i=0; i<NameBuff_IDX + 1; i++){
        if (i == NameBuff_IDX) {
            currentPlayer->name[i] = '\0';
        } else {
            currentPlayer->name[i] = name[i];
        }
    }

    currentPlayer->name_size = NameBuff_IDX;
    currentPlayer->time = timer_stop_time - timer_start_time;
    currentPlayer->next_player = NULL;

    if(record_list == NULL){
        record_list = currentPlayer;
        record_list_idx++;
        return;
    }

    if(record_list_idx == 1){
        if(currentPlayer->time < record_list->time){
            currentPlayer->next_player = record_list;
            record_list->next_player = NULL;
            record_list = currentPlayer;
        }
        else{
            record_list->next_player = currentPlayer;
        }
        record_list_idx ++;
        return;
    }
    
    player_recording* temp_current = record_list;
    player_recording* temp_previous = NULL;

    for(int i=0; i<record_list_idx; i++){

        if(currentPlayer->time < temp_current->time){

            if(i==0){ //first;

                currentPlayer->next_player = record_list;
                record_list = currentPlayer;
                record_list_idx ++;
                return;
            }
            else{
                //player_recording* temp = temp_previous->next_player;
                temp_previous->next_player = currentPlayer;
                currentPlayer->next_player = temp_current;
                record_list_idx ++;
                return;
            }
        }

        temp_previous = temp_current;
        temp_current = temp_current->next_player;

        if(temp_current == NULL){ // the last  

            temp_previous->next_player = currentPlayer;
            record_list_idx ++;
            return;
        }   
    }
}


static void free_list()
{
    if(record_list == NULL){
        return;
    }

    while(record_list->next_player != NULL){

        player_recording* discard = record_list;
        record_list = discard->next_player;
        discard->next_player = NULL;
        free(discard);
    }

    free(record_list);
    record_list_idx = 0;
}

static void clearName()
{
    for (int i = 0; i < MAX_NAME_SIZE; i++) {
        name[i] = '\0';
    }
}

void Menu_NameInput()
{
    JoystickInputInit();
    Game_PauseTheGame();
    isMenuRunning = true;
    cursor_row = 0;
    cursor_col = 0;
    NameBuff_IDX = 0;
    Name_Is_Entered = false;
    
    clearName();
    GameDisplay_displayKeyboard();

    while(!Name_Is_Entered){

        for(int i=0; i<3; i++){
            for(int j=0; j<10; j++){
                if(i == cursor_row && j==cursor_col){
                    printf("1 ");
                }
                else{
                    printf("%c ",keyboard[i][j]);
                }
            }
            printf("\n");
        }
        printf("\n");
        if(NameBuff_IDX == 0){
            printf("empty rn \n");
        }
        else{
            for(int i=0; i<NameBuff_IDX; i++){
                printf("%c", name[i]);
            }
        }
        printf("\n");

        if(isButtonPressed()){
            GameAudio_Keyboard();
            char input = keyboard[cursor_row][cursor_col];

            if(input == '/'){
                if(NameBuff_IDX != 0){
                    name[NameBuff_IDX] = '\0';
                    Name_Is_Entered = true;
                }
            }
            else if(input == '-'){

                if(NameBuff_IDX != 0){
                    NameBuff_IDX --;
                    name[NameBuff_IDX] = '\0';
                }
            }
            else{
                if(NameBuff_IDX < MAX_NAME_SIZE){
                    name[NameBuff_IDX] = input;
                    NameBuff_IDX ++;
                }
            }
        }

        GameDisplay_displayName(name);
        LcdScreen_moveCursor(cursor_row + 1, cursor_col + 5);
        sleepForMs(150);
    }
    isMenuRunning = false;
    JoystickInputClear();
    insertPlayer_intoList();
}



bool Menu_isMenuRunning()
{
    return isMenuRunning;
}



void Menu_MoveCursor(int direction)
{
    switch(direction){

        case JOYSTICK_UP:
        {
            int next_row = cursor_row-1;
            int next_col = cursor_col;
            if(next_row >= 0){
                move_the_cursor(next_row, next_col);
            }
            GameAudio_MoveCharacter();
            break;
        }
        case JOYSTICK_DOWN:
        {
            int next_row = cursor_row + 1;
            int next_col = cursor_col;
            if(next_row < 3){
                move_the_cursor(next_row, next_col);
            }
            GameAudio_MoveCharacter();
            break;
        }
        case JOYSTICK_LEFT:
        {
            int next_row = cursor_row;
            int next_col = cursor_col - 1;
            if(next_col >= 0){
                move_the_cursor(next_row, next_col);
            }
            GameAudio_MoveCharacter();
            break;
        }
        case JOYSTICK_RIGHT:
        {
            int next_row = cursor_row;
            int next_col = cursor_col + 1;
            if(next_col < 10){
                move_the_cursor(next_row, next_col);
            }
            GameAudio_MoveCharacter();
            break;
        }
    }
}



void Menu_TimerStart()
{
    timer_start_time = getTimeInSec();
}



void Menu_TimerStop()
{
    timer_stop_time = getTimeInSec();
}

void Menu_Start()
{
    bool isCursor_onQuit = false;
    bool isPressed = false;
    
    LcdScreen_clear();
    GameDisplay_displayEnd();
    GameDisplay_endCursor(isCursor_onQuit);
    sleepForMs(500);

    while(!isPressed){
        if(Joystick_Get_Direction() == JOYSTICK_LEFT){
            isCursor_onQuit = false;
        }
        else if(Joystick_Get_Direction() == JOYSTICK_RIGHT){
            isCursor_onQuit = true;
        }

        GameDisplay_endCursor(isCursor_onQuit);

        if(isButtonPressed()){
            if(isCursor_onQuit){
                isQuit = true;
                isPressed = true;
            }
            else{
                isPressed = true;
            }
        }
        sleepForMs(150);
    }
}

void Menu_Ranking()
{
    player_recording* currentShowing = record_list;
    // only showing 3
    for (int i=0; i<3; i++){

        if(i == record_list_idx){
            break;
        }

        if(currentShowing == NULL){
            break;
        }

        for(int j=0; j<currentShowing->name_size; j++){
            printf("%c", currentShowing->name[j]);
        }
        printf(" ");
        if(currentShowing->time < 60){
            printf("%lld ", currentShowing->time);
        }
        else{
            int min = (currentShowing->time)/60;
            int sec = (currentShowing->time)%60;
            printf("%d:%d", min, sec);
        }
        printf("\n");
        currentShowing = currentShowing->next_player;
    }

    player_recording* nextPlayer = record_list;
    LcdScreen_hideCursor();
    for (int pages = 0; pages < 3; pages++) {
        sleepForMs(500);
        nextPlayer = GameDisplay_displayRanking(nextPlayer, pages * 3);

        while(!isButtonPressed()) {
            sleepForMs(100);
        }

        if (nextPlayer == NULL) {
            break;
        }
    }
    LcdScreen_showCursor();
}



void Menu_Free()
{
    free_list();
}



bool Menu_isGameQuitted()
{
    return isQuit;
}