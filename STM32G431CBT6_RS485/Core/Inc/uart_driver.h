/*
 * uart_driver.h
 *
 * Created on: 2026. 2. 27.
 * Author: lenovo
 */

#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#include "main.h"
#include "usart.h" // UART_HandleTypeDef를 사용하기 위해 필요합니다.

/* ==========================================================================
 * [통신 모드 설정 스위치]
 * 0 : RS-232 모드 (USART2 사용 - PA2, PA3)
 * 1 : RS-485 모드 (USART3 사용 - PB10, PB11, PB14/DE)
 * ========================================================================== */
#define USE_RS485_MODE    1

#if USE_RS485_MODE
    #define H_UART        huart3      // 활성 UART 핸들 가명
    #define UART_INST     USART3      // 활성 UART 인스턴스 가명
#else
    #define H_UART        huart2
    #define UART_INST     USART2
#endif
/* ========================================================================== */

// 외부(usart.c)에 선언된 핸들을 가져옵니다.
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

// 외부에서 접근 가능한 CLI 관련 변수들
extern char uart_rx_buffer[64];
extern uint8_t char_received_flag;
extern uint8_t command_ready_flag;
extern uint8_t uart_rx_idx;

// 함수 선언
void UART_Driver_Init(void);
void UART_Driver_ResetBuffer(void);

#endif /* __UART_DRIVER_H__ */
