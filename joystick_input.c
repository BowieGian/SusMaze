#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#include "timer.h"
#include "joystick_input.h"
#include "menu.h"

static int Direction = JOYSTICK_CENTER;
static pthread_t idJoystick;
static bool isDone = false;

static void* GetDirection(void* _)
{
    isDone = false;
    while(!isDone){

        Direction = Joystick_Get_Direction();

        if(Menu_isMenuRunning()){

            Menu_MoveCursor(Direction);
        }
        else if(Game_Allows_Operation()){

            Game_MoveCharacter(Direction);
            //printf("%d \n", Direction);
        }
        sleepForMs(200);
    }
    return NULL;
}


void JoystickInputInit()
{
    pthread_create(&idJoystick, NULL, &GetDirection, NULL);
}


int JoystickInputGetDirection()
{
    return Direction;
}


void JoystickInputClear()
{
    isDone = true;
    pthread_join(idJoystick, NULL);
}