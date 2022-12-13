#include <stdio.h>
#include <ctype.h>

#include "game.h"
#include "timer.h"
#include "gpio.h"
#include "menu.h"
#include "game_display.h"
#include "lcd_screen.h"
#include "button.h"

int main()
{
    printf("Game Start\n"); 
    ButtonInit();
    UserInit();

    GameDisplay_setup();
    AudioMixer_init();
    GameAudio_GenerateSounds();
    Menu_Start();

    while(!Menu_isGameQuitted()){
        //Menu_NameInput();
        Game_init();
        GameDisplay_init();
        Menu_TimerStart();
        //after passing all maps
        sleepForMs(1000);
        while(!Game_Is_Done()){
            sleepForMs(100);
        } //something)
    
        Menu_TimerStop();
        GameDisplay_cleanUp();
        Game_clear();

        Menu_NameInput();
        Menu_Ranking(); // rank the player time
        Menu_Start();
    }

    LcdScreen_clear();
    AudioMixer_cleanup();
    GameAduio_ClearSounds();

    


    Menu_Free();



    return 0;
}