/*
 * uart_driver.h
 *
 *  Created on: 2026. 2. 27.
 *      Author: lenovo
 */

#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#include "main.h"

// 외부에서 접근 가능한 변수들
extern char uart_rx_buffer[64];
extern uint8_t char_received_flag;
extern uint8_t command_ready_flag;
extern uint8_t uart_rx_idx;

// 함수 선언
void UART_Driver_Init(void);
void UART_Driver_ResetBuffer(void);

#endif
