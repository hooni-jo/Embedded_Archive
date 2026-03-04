/*
 * led.c
 *
 *  Created on: 2026. 3. 3.
 *      Author: lenovo
 */

#include "led.h"

void LED_Control(uint8_t num, uint8_t state) {
    GPIO_PinState pinState = (state == LED_ON) ? GPIO_PIN_RESET : GPIO_PIN_SET;

    switch (num) {
        case 1:
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, pinState);
            break;
        case 2:
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, pinState);
            break;
        default:
            // 알 수 없는 번호는 무시
            break;
    }
}

void LED_Toggle(uint8_t num) {
    switch (num) {
        case 1:
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
            break;
        case 2:
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
            break;
    }
}

void LED_All_Off(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
}
