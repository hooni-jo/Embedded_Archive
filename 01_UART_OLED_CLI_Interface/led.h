/*
 * led.h
 *
 *  Created on: 2026. 3. 3.
 *      Author: lenovo
 */

#ifndef __LED_H__
#define __LED_H__

#include "main.h" // HAL 드라이버 및 핀 정의 포함

// LED 상태 정의 (가독성을 위해)
#define LED_OFF  0
#define LED_ON   1

/**
 * @brief LED 개별 제어 함수
 * @param num   LED 번호 (1, 2, 3...)
 * @param state LED 상태 (0: OFF, 1: ON)
 */
void LED_Control(uint8_t num, uint8_t state);

/**
 * @brief LED 상태 반전 함수 (테스트용)
 * @param num LED 번호
 */
void LED_Toggle(uint8_t num);

/**
 * @brief 보드의 모든 LED를 끔
 */
void LED_All_Off(void);

#endif /* __LED_H__ */
