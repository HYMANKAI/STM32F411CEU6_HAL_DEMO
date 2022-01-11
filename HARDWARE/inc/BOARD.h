#ifndef _BOARD_H
#define _BOARD_H

#include "main.h"
    
#define LED_ON()   HAL_GPIO_WritePin(LED_PORT,LED_PIN,GPIO_PIN_RESET)
#define LED_OFF()  HAL_GPIO_WritePin(LED_PORT,LED_PIN,GPIO_PIN_SET)
#define LED_RVS()  HAL_GPIO_TogglePin(LED_PORT,LED_PIN)

void MX_GPIO_Init(void);
unsigned char KEY_READ(void);

#endif
