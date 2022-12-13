#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "mya2d.h"

int A2D_getVoltage0Reading(char* A2DFile)
{
    // Open file
    FILE *f = fopen(A2DFile, "r");
    if (!f) {
        printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
        printf(" Check /boot/uEnv.txt for correct options.\n");
        exit(-1);
    }
    // Get reading
    int a2dReading = 0;
    int itemsRead = fscanf(f, "%d", &a2dReading);
    if (itemsRead <= 0) {
        printf("ERROR: Unable to read values from voltage input file.\n");
        exit(-1);
    }
    // Close file
    fclose(f);
    return a2dReading;
}
