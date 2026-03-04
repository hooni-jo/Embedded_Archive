/*
 * buzzer.c
 *
 *  Created on: 2026. 3. 3.
 *      Author: lenovo
 */

#include "buzzer.h"
#include "tim.h" // MX에서 생성된 htim1 변수를 사용하기 위함
#include "notes.h"

void Buzzer_SetFrequency(uint32_t freq) {
    if (freq == 0) {
        Buzzer_Off();
        return;
    }

    // 1MHz(1,000,000Hz) / 목표 주파수 = (ARR + 1)
    uint32_t arr = (1000000 / freq) - 1;
    uint32_t pulse = (arr + 1) / 2; // 50% Duty Cycle

    // 타이머 레지스터 직접 변경
    __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);

    // PWM 다시 시작 (설정 적용을 위함)
    Buzzer_On();
}

void Buzzer_PlayScale(void) {
    const uint32_t scale[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};

    for (int i = 0; i < 8; i++) {
        Buzzer_SetFrequency(scale[i]);
        HAL_Delay(300);
        Buzzer_Off();
        HAL_Delay(50);
    }
}

void Buzzer_Control(uint8_t state) {
    if (state) {
        Buzzer_On();
    } else {
        Buzzer_Off();
    }
}

void Buzzer_On(void) {
    // TIM1은 Advanced 타이머이므로 CH1N(PC13) 제어를 위해 Ex와 PWMN이 붙은 함수를 사용합니다.
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}

void Buzzer_Off(void) {
    // PWM 출력을 중단하여 소리를 끕니다.
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
}
