#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "mya2d.h"
#include "joystick.h"

#define CMD_BUFF 100

#define JOYSTICK_PUSHED_THRESHOLD 0.5

double Joystick_Get_XValue()
{   
    char A2D_Value_File[CMD_BUFF];

    snprintf(A2D_Value_File, CMD_BUFF, "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw", JOYSTICK_X_A2D);

    double X_value = (double)A2D_getVoltage0Reading(A2D_Value_File);

    double A2D_Scale_Stanrd = (double)A2D_MAX_READING/2;

    X_value = (A2D_Scale_Stanrd - X_value)/A2D_Scale_Stanrd;

    return X_value;

}


double Joystick_Get_YValue()
{
    char A2D_Value_File[CMD_BUFF];

    snprintf(A2D_Value_File, CMD_BUFF, "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw", JOYSTICK_Y_A2D);

    double Y_value = (double)A2D_getVoltage0Reading(A2D_Value_File);

    double A2D_Scale_Stanrd = (double)A2D_MAX_READING/2;

    Y_value = (A2D_Scale_Stanrd - Y_value)/A2D_Scale_Stanrd;

    return Y_value;

}



int Joystick_Get_Direction()
{
    double X_value = Joystick_Get_XValue();
    double Y_value = Joystick_Get_YValue();
    
    double absX = fabs(X_value);
    double absY = fabs(Y_value);
    
    if(absX<JOYSTICK_PUSHED_THRESHOLD && absY<JOYSTICK_PUSHED_THRESHOLD){
        return JOYSTICK_CENTER;
    }
    
    if(absX < JOYSTICK_PUSHED_THRESHOLD || absY < JOYSTICK_PUSHED_THRESHOLD)
    {
        // Logics for moving right or left
        if(absX < JOYSTICK_PUSHED_THRESHOLD){
            if( Y_value > 0){
                return JOYSTICK_RIGHT;
            }
            else{
                return JOYSTICK_LEFT;
            }
        }

    // Logics for moving up or down
        if(absY < JOYSTICK_PUSHED_THRESHOLD){
            if( X_value > 0){
                return JOYSTICK_UP;
            }
            else{
                return JOYSTICK_DOWN;
            }
        }
    }

    //logics for moving other directions 
    if((Y_value * X_value) > 0){
        if(X_value > 0){
            return JOYSTICK_RIGHT_UP;
        }
        else{
            return JOYSTICK_LEFT_DOWN;
        }
    }

    if(X_value < 0){
        return JOYSTICK_RIGHT_DOWN;
    }
    else{
        return JOYSTICK_LEFT_UP;
    }
}