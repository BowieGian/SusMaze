//  created by Chenting Mao
//  header file for functions related to timer
//	last edited: 2022/9/30	by:	Chenting Mao

#ifndef TIMER_H
#define TIMER_H
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

//  Function given in notes
//  Get the current time of timer in ms
long long getTimeInMs(void);

long long getTimeInSec(void);

// Get the current time in ns
long long getTimeInNs(void);

//  Functiojn given in notes
//  Let the processor to wait for "delayInMs" ms
void sleepForMs(long long delayInMs);


#endif