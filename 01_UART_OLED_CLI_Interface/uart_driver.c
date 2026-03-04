/*
 * uart_driver.c
 *
 *  Created on: 2026. 2. 27.
 *      Author: lenovo
 */

#include "uart_driver.h"
#include "usart.h"
#include "cli.h"
#include <string.h>

uint8_t rxData;
char uart_rx_buffer[64];
uint8_t uart_rx_idx = 0;
uint8_t char_received_flag = 0;
uint8_t command_ready_flag = 0;

void UART_Driver_Init(void) {
    HAL_UART_Receive_IT(&huart2, &rxData, 1);
}

void UART_Driver_ResetBuffer(void) {
    memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
    uart_rx_idx = 0;
    command_ready_flag = 0;
}

static uint8_t esc_state = 0; // ESC 시퀀스 상태 변수

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART2) {
        // 1. ESC 시퀀스 처리 (상태 머신)
        if (rxData == 0x1B) { // ESC 시작
            esc_state = 1;
        }
        else if (esc_state == 1) {
            if (rxData == 0x5B) esc_state = 2; // '[' 감지
            else esc_state = 0; // 잘못된 시퀀스면 초기화
        }
        else if (esc_state == 2) {
            if (rxData == 0x41) { // 'A' (위쪽 화살표)
                CLI_HistoryNavigate(1);
            } else if (rxData == 0x42) { // 'B' (아래쪽 화살표)
                CLI_HistoryNavigate(-1);
            }
            esc_state = 0; // 시퀀스 종료
        }
        // 2. 일반 문자 및 기타 키 처리 (ESC 시퀀스 중이 아닐 때만 실행)
        else {
            if(rxData == '\r' || rxData == '\n') {
                uart_rx_buffer[uart_rx_idx] = '\0';
                command_ready_flag = 1;
                HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 10);
            }
            else if(rxData == 0x09) { // Tab
                CLI_TabAutocomplete();
            }
            else if(rxData == 0x08 || rxData == 0x7F) { // Backspace
                if(uart_rx_idx > 0) {
                    uart_rx_idx--;
                    uart_rx_buffer[uart_rx_idx] = '\0';
                    HAL_UART_Transmit(&huart2, (uint8_t*)"\b \b", 3, 10);
                    char_received_flag = 1;
                }
            }
            else { // 일반 문자
                if(uart_rx_idx < 63) {
                    uart_rx_buffer[uart_rx_idx++] = rxData;
                    HAL_UART_Transmit(&huart2, &rxData, 1, 10);
                    char_received_flag = 1;
                }
            }
        }

        // 다음 바이트 수신 대기
        HAL_UART_Receive_IT(&huart2, &rxData, 1);
    }
}
