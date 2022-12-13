//  created by Chenting Mao
//  header file for functions related to GPIOs on beaglebone
//	last edited: 2022/10/2	by:	Chenting Mao

#ifndef GPIO_H
#define GPIO_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define USER_VALUE "/sys/class/gpio/gpio72/value"

//  Config USER(GPIO72) as gpio
//  return FALSE and print ERROR message if config fails
bool UserInit(void);

//  Read the the value of USER(GPIO72)
//  return 1/0 if USER is unpressed/pressed
//  return an ERROR# and print ERROR message if error occurs
//  function ReadUserValueErrorCheck() detect ERROR#
int ReadUserValue(void);


//  Detect ERROR# from the return value of the function ReadUserValue()
//  return TRUE if "ReadUserValue" == ERROR#
//  parameter should be the return value of ReadUserValue()
//  bool ReadUserValueErrorCheck(void);


//  Return TRUE/FALSE if user buttom is pressed/unpressed
bool UserIsPressed(void);

void GPIO_8_Init(int gpioNum);
void GPIO_9_Init(int gpioNum);

int ReadGPIOValue(int gpioNum);



#endif

