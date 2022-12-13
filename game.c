#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "game_audio.h"
#include "game.h"
#include "gpio.h"

#define HOW_MANY_MAPS 4

static int currentMapIDX = 0;
static pthread_mutex_t currentMapIDX_Mutex = PTHREAD_MUTEX_INITIALIZER;

static character player;
static pthread_mutex_t characterPosition_Mutex = PTHREAD_MUTEX_INITIALIZER;

static bool isUnlocked = false;
static bool isThisRoomDone = false;
static bool isOperation_Allowed = false;
static pthread_mutex_t generalFlag_Mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t id_Game_Thread;

static bool isPause = true;
static pthread_mutex_t isPause_Mutex = PTHREAD_MUTEX_INITIALIZER;

static bool isDone = false;

static int MAP[HOW_MANY_MAPS][MAP_ROW][MAP_COL];

static int KEY_ROW_BUFF[HOW_MANY_MAPS];
static int KEY_COL_BUFF[HOW_MANY_MAPS];

static int EXIT_ROW_BUFF[HOW_MANY_MAPS];
static int EXIT_COL_BUFF[HOW_MANY_MAPS];

static int LOCK_ROW_BUFF[HOW_MANY_MAPS];
static int LOCK_COL_BUFF[HOW_MANY_MAPS];

// Assumes maps are formatted correctly
void readMap(char* filePath, int map_idx)
{
    FILE *pFile = fopen(filePath, "r");
    if (pFile == NULL) {
        printf("ERROR OPENING %s.", filePath);
        exit(1);
    }

    char buff = 0;
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL + 1; j++) {
            buff = getc(pFile);

            // Remove \r and \n chars at end or rows
            if (j >= MAP_COL) {
                continue;
            }

            MAP[map_idx][i][j] = buff - '0';
        }
    }
    fclose(pFile);
}

static void Generate_Map(){

    readMap(TUTORIAL_MAP_PATH, 0);
    readMap(MAP1_PATH, 1);
    readMap(MAP2_PATH, 2);
    readMap(MAP3_PATH, 3);


    for(int i=0; i<HOW_MANY_MAPS; i++){

        for(int r=0; r<MAP_ROW; r++){

            for(int c=0; c<MAP_COL; c++){

                int currentElement = MAP[i][r][c];

                if(currentElement == KEY){
                    KEY_ROW_BUFF[i] = r;
                    KEY_COL_BUFF[i] = c;
                }

                if(currentElement == LOCK){
                    LOCK_ROW_BUFF[i] = r;
                    LOCK_COL_BUFF[i] = c;
                }

                if(currentElement == EXIT){
                    EXIT_ROW_BUFF[i] = r;
                    EXIT_COL_BUFF[i] = c;
                }
            }           
        }
    }
}


static void Character_Born()
{
    pthread_mutex_lock(&characterPosition_Mutex);
    {
        player.isKeyFound = false;
        player.col = 0;
        player.row = 0;
    }
    pthread_mutex_unlock(&characterPosition_Mutex);

}


static void Set_Character_Position(unsigned int new_row, unsigned int new_col){

    pthread_mutex_lock(&characterPosition_Mutex);
    {
        player.col = new_col;
        player.row = new_row;
    }
    pthread_mutex_unlock(&characterPosition_Mutex);
}


static void Character_Pick_Key(){
    pthread_mutex_lock(&characterPosition_Mutex);
    {
        player.isKeyFound = true;
    }
    pthread_mutex_unlock(&characterPosition_Mutex);
}


static void move_the_character(int next_row, int next_col)
{
    int nextElement = MAP[currentMapIDX][next_row][next_col];

    if(nextElement == BLOCK){
        //adding a voice sound here
        GameAudio_HittingBlock();
        printf("hit the block \n");
        return;
    }

    if(nextElement == KEY){
        Character_Pick_Key();
        MAP[currentMapIDX][next_row][next_col] = EMPTY;
        Set_Character_Position(next_row,next_col); 
        GameAudio_PickUpKey();
        printf("Key is Picked \n");   
        return;
    }


    if(nextElement == LOCK){

        if(player.isKeyFound && !isUnlocked){
            // play a sound maybe
            GameAudio_Unlock();
            pthread_mutex_lock(&generalFlag_Mutex);
            {
                isUnlocked = true;
                player.isKeyFound = false;
            }
            pthread_mutex_unlock(&generalFlag_Mutex);
            printf("unlock the LOCK \n");
        }
        else {
            //play a sound 
            GameAudio_HittingBlock();
            printf("hit the LOCK \n");
        }
        return;
    }
    

    if(nextElement == EXIT){

        if(isUnlocked == true){
            pthread_mutex_lock(&generalFlag_Mutex);
            {
                isThisRoomDone = true;
            }
            pthread_mutex_unlock(&generalFlag_Mutex);
        }
        else{
            //play a sound 
            GameAudio_HittingBlock();
            printf("hit the EXIT \n");
        }
        return;
    }
    
    if(nextElement == EMPTY){
        GameAudio_MoveCharacter();
        Set_Character_Position(next_row,next_col);
    }
}

static void* Game_Thread(void* _)
{
    //AudioMixer_init();
    Generate_Map();
    Character_Born();
    JoystickInputInit();
    //GameAudio_GenerateSounds();
    isPause = false;
    currentMapIDX = 0;
    isOperation_Allowed = true;
    isDone = false;

    while(!isDone){

        if(UserIsPressed()) {
            isThisRoomDone = true;
            currentMapIDX = HOW_MANY_MAPS;
            printf("Force Quit Game\n");
        }
        
        if(isPause){
            isOperation_Allowed = false;
            continue;
        }
        else{
            isOperation_Allowed = true;
        }

        if(isThisRoomDone == true){

            isOperation_Allowed = false;

            if(currentMapIDX < HOW_MANY_MAPS){
                pthread_mutex_lock(&currentMapIDX_Mutex);
                {   
                    currentMapIDX++;
                }
                pthread_mutex_unlock(&currentMapIDX_Mutex);
            }
            
            if(currentMapIDX == HOW_MANY_MAPS){
                
                printf("all maps are finished. \n");
                //currentMapIDX = 0;
                //isOperation_Allowed = false;
                isDone = true;
                continue;
            }

            pthread_mutex_lock(&generalFlag_Mutex);
            {
                isUnlocked = false;
                isThisRoomDone = false;
            }
            pthread_mutex_unlock(&generalFlag_Mutex);
            Character_Born();

            isOperation_Allowed = true;
        }

        sleepForMs(50);
    }

    //printf("Game Thread Done \n");
    JoystickInputClear();
    //AudioMixer_cleanup();
    //GameAduio_ClearSounds();
    return NULL;
}


void Game_init()
{
    isDone = false;
    currentMapIDX = 0;
    isUnlocked = false;
    isThisRoomDone = false;
    pthread_create(&id_Game_Thread, NULL, &Game_Thread, NULL);
}


void Game_clear()
{
    //printf("stop \n");
    isDone = true;
    pthread_join(id_Game_Thread, NULL);
    //printf("stop Done\n");

}


void Game_GetRoom(room* newRoom)
{
    int idx;
    pthread_mutex_lock(&currentMapIDX_Mutex);
    {
        if(currentMapIDX < HOW_MANY_MAPS){
            idx = currentMapIDX;
        }
        else{
            idx = HOW_MANY_MAPS - 1;
        }
    }
    pthread_mutex_unlock(&currentMapIDX_Mutex);

    newRoom->row1 = malloc(MAP_COL * sizeof(*(newRoom->row1)));
    newRoom->row2 = malloc(MAP_COL * sizeof(*(newRoom->row2)));
    newRoom->row3 = malloc(MAP_COL * sizeof(*(newRoom->row3)));
    newRoom->row4 = malloc(MAP_COL * sizeof(*(newRoom->row4)));

    //pthread_mutex_lock(&currentMapIDX_Mutex);
    //{
        for(int i=0; i<MAP_COL; i++){
            
            newRoom->row1[i] = MAP[idx][0][i];
            newRoom->row2[i] = MAP[idx][1][i];
            newRoom->row3[i] = MAP[idx][2][i];
            newRoom->row4[i] = MAP[idx][3][i];
        }
    //}
    //pthread_mutex_unlock(&currentMapIDX_Mutex);

    //pthread_mutex_lock(&currentMapIDX_Mutex);
    //{
        newRoom->lock_row = LOCK_ROW_BUFF[idx];
        newRoom->lock_col = LOCK_COL_BUFF[idx];

        newRoom->key_row = KEY_ROW_BUFF[idx];
        newRoom->key_col = KEY_COL_BUFF[idx];

        newRoom->exit_row = EXIT_ROW_BUFF[idx];
        newRoom->exit_col = EXIT_COL_BUFF[idx];
    //}
    //pthread_mutex_unlock(&currentMapIDX_Mutex);
}


void Game_FreeRoom(room* currentRoom)
{
    free(currentRoom->row1);
    free(currentRoom->row2);
    free(currentRoom->row3);
    free(currentRoom->row4);

    currentRoom->row1 = NULL;
    currentRoom->row1 = NULL;
    currentRoom->row1 = NULL;
    currentRoom->row1 = NULL;

    currentRoom->lock_row = 0;
    currentRoom->lock_col = 0;

    currentRoom->key_col = 0;
    currentRoom->key_row = 0;

    currentRoom->exit_col = 0;
    currentRoom->exit_row = 0;
}


void Game_MoveCharacter(int direction)
{
    switch(direction)
    {
        case CHARACTER_UP:
        {
            if(player.row == 0){
                //adding a voice sound here
                GameAudio_HittingBlock();
                printf("Hit the border \n");
                return;
            }
            int next_row = player.row - 1;
            int next_col = player.col;
            move_the_character(next_row,next_col);
            break;
        }

        case CHARACTER_DOWN:
        {
            if(player.row == MAP_ROW-1){
                GameAudio_HittingBlock();
                printf("Hit the border \n");
                return;
            }
            int next_row = player.row + 1;
            int next_col = player.col;
            move_the_character(next_row,next_col);
            break;
        }

        case CHARACTER_LEFT:
        {
            if(player.col == 0){
                printf("Hit the border \n");
                GameAudio_HittingBlock();
                return;
            }
            int next_row = player.row;
            int next_col = player.col - 1;
            move_the_character(next_row,next_col);
            break;
        }

        case CHARACTER_RIGHT:
        {
            if(player.col == MAP_COL-1){
                printf("Hit the border \n");
                GameAudio_HittingBlock();
                return;
            }
            int next_row = player.row;
            int next_col = player.col + 1;
            move_the_character(next_row,next_col);
            break;
        }

        case CHARACTER_LEFT_UP:
        {
            if(player.row == 0 || player.col == 0){
                printf("Hit the border \n");
                GameAudio_HittingBlock();
                return;
            }
            int next_row = player.row - 1;
            int next_col = player.col - 1;
            move_the_character(next_row,next_col);
            break;
        }

        case CHARACTER_RIGHT_UP:
        {
            if(player.row == 0 || player.col == MAP_COL-1){
                printf("Hit the border \n");
                GameAudio_HittingBlock();
                return;
            }
            int next_row = player.row - 1;
            int next_col = player.col + 1;
            move_the_character(next_row,next_col);
            break;
        }

        case CHARACTER_LEFT_DOWN:
        {
            if(player.row == MAP_ROW-1 || player.col == 0){
                printf("Hit the border \n");
                GameAudio_HittingBlock();
                return;
            }
            int next_row = player.row + 1;
            int next_col = player.col - 1;
            move_the_character(next_row,next_col);
            break;
        }

        case CHARACTER_RIGHT_DOWN:
        {
            if(player.row == MAP_ROW-1 || player.col == MAP_COL-1){
                printf("Hit the border \n");
                GameAudio_HittingBlock();
                return;
            }
            int next_row = player.row + 1;
            int next_col = player.col + 1;
            move_the_character(next_row,next_col);
            break;
        }
    }
}


bool Game_isKeyPicked()
{
    return player.isKeyFound;
}


bool Game_isUnlocked()
{
    return isUnlocked;
}


bool Game_isNextMap()
{
    return isThisRoomDone;
}


bool Game_Allows_Operation()
{
    return isOperation_Allowed;
}


int Game_Character_Row()
{
    return player.row;
}


int Game_Character_Col()
{
    return player.col;
}


bool Game_Is_Done()
{
    return isDone;
}


void Game_PauseTheGame()
{
    pthread_mutex_lock(&isPause_Mutex);
    {
        isPause = true;
    }
    pthread_mutex_unlock(&isPause_Mutex);
}


bool Game_IsAllMapDone()
{
    return (currentMapIDX == HOW_MANY_MAPS);
}


void Game_ResumeTheGame()
{
    pthread_mutex_lock(&isPause_Mutex);
    {
        isPause = false;;
    }
    pthread_mutex_unlock(&isPause_Mutex);
}