#ifndef MENU_H
#define MENU_H

#define PLAYER_RANKING_NUM 10
#define MAX_NAME_SIZE 10

typedef struct player_recording {

    char name[10];
    int name_size;
    long long time;
    struct player_recording* next_player;

}player_recording;

void Menu_NameInput();

bool Menu_isMenuRunning();
void Menu_MoveCursor(int direction);

void Menu_TimerStart();
void Menu_TimerStop();

void Menu_Start();
void Menu_Ranking();

void Menu_Free();
bool Menu_isGameQuitted();

#endif