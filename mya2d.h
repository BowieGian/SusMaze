// This module is for BeagleBone Green A2D
#ifndef MYADC
#define MYADC

// A2D max value in terms of voltage
#define A2D_VOLTAGE_REF_V 1.8

// A2D max value
#define A2D_MAX_READING 4095

// Reading A2D register value 
// A2DFile is under /sys/bus/iio/devices/iio:device0/
int A2D_getVoltage0Reading(char* A2DFile);

#endif