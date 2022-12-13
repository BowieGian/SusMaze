#include "button.h"

void ButtonInit()
{   
    GPIO_9_Init(GAME_BUTTON_GPIO_PIN);
}

bool isButtonPressed()
{
    return(ReadGPIOValue(GAME_BUTTON_GPIO_NO));
}
