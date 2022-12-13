// This module is for BeagleBone Green carrying Mini 2-Axis Analog Thumbstick with Analog Mini Thumbstick Breakout Board
#ifndef JOYSTICK_H
#define JOYSTICK_H

// Which A2D that the Analog Mini Thumbstick Breakout Board connects to 
#define JOYSTICK_X_A2D 2
#define JOYSTICK_Y_A2D 3

enum JoystickDirections
{
    JOYSTICK_UP = 0,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,
    JOYSTICK_RIGHT_UP,
    JOYSTICK_RIGHT_DOWN,
    JOYSTICK_LEFT_UP,
    JOYSTICK_LEFT_DOWN,
    JOYSTICK_CENTER,
    JOYSTICK_MAX_NUMBER_DIRECTIONS      // Get the number of directions via the enum
};

// getting the Joystick x/y direction value from [-1, 1]
// returning 0 means joystick is unpushed at x/y direction
// returning 1 means joystick is fully pushed that the x/y direction opposites to the label Analog Mini Thumbstick Breakout Board
// returing -1 meeans joystick is fully pushed in the x/y direction of the label Analog Mini Thumbstick Breakout Board
double Joystick_Get_XValue();

double Joystick_Get_YValue();

// returning the direction of the joystick in terms of the
// elements in the above declared enum
// has very stric critical conditions on judging JOYSTICK_RIGHT_UP, JOYSTICK_RIGHT_DOWN, JOYSTICK_LEFT_UP, and JOYSTICK__LEFT_DOWNS
int Joystick_Get_Direction();

#endif