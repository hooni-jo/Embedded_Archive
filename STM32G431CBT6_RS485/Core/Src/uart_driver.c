/*
 * uart_driver.c
 *
 * Created on: 2026. 2. 27.
 * Author: lenovo
 */

#include "uart_driver.h"
#include "usart.h"
#include "cli.h"
#include <string.h>

// 수신 데이터 및 버퍼 관련 변수
uint8_t rxData;
char uart_rx_buffer[64];
uint8_t uart_rx_idx = 0;
uint8_t char_received_flag = 0;
uint8_t command_ready_flag = 0;

static uint8_t esc_state = 0; // ESC 시퀀스 상태 변수

/**
 * @brief 드라이버 초기화 및 수신 인터럽트 시작
 */
void UART_Driver_Init(void) {
    // 가명(H_UART)을 사용하여 어떤 포트든 동일한 함수로 수신 시작
    HAL_UART_Receive_IT(&H_UART, &rxData, 1);
}

/**
 * @brief 수신 버퍼 및 플래그 초기화
 */
void UART_Driver_ResetBuffer(void) {
    memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
    uart_rx_idx = 0;
    command_ready_flag = 0;
}

/**
 * @brief UART 수신 완료 콜백 함수
 * 하드웨어 흐름 제어(RS-485)가 설정되어 있어 PB14(DE) 제어 코드가 필요 없음
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // 현재 인터럽트가 발생한 포트가 우리가 설정한 인스턴스(UART_INST)인지 확인
    if(huart->Instance == UART_INST) {

        // 1. ESC 시퀀스 처리 (상태 머신)
        if (rxData == 0x1B) { // ESC 시작
            esc_state = 1;
        }
        else if (esc_state == 1) {
            if (rxData == 0x5B) esc_state = 2; // '[' 감지
            else esc_state = 0;
        }
        else if (esc_state == 2) {
            if (rxData == 0x41) { // 'A' (위쪽 화살표)
                CLI_HistoryNavigate(1);
            } else if (rxData == 0x42) { // 'B' (아래쪽 화살표)
                CLI_HistoryNavigate(-1);
            }
            esc_state = 0;
        }
        // 2. 일반 문자 및 기타 키 처리 (ESC 시퀀스 중이 아닐 때만 실행)
        else {
            if(rxData == '\r' || rxData == '\n') {
                uart_rx_buffer[uart_rx_idx] = '\0';
                command_ready_flag = 1;
                // 에코 전송: 하드웨어가 자동으로 DE 핀 제어
                HAL_UART_Transmit(&H_UART, (uint8_t*)"\r\n", 2, 10);
            }
            else if(rxData == 0x09) { // Tab
                CLI_TabAutocomplete();
            }
            else if(rxData == 0x08 || rxData == 0x7F) { // Backspace
                if(uart_rx_idx > 0) {
                    uart_rx_idx--;
                    uart_rx_buffer[uart_rx_idx] = '\0';
                    HAL_UART_Transmit(&H_UART, (uint8_t*)"\b \b", 3, 10);
                    char_received_flag = 1;
                }
            }
            else { // 일반 문자
                if(uart_rx_idx < 63) {
                    uart_rx_buffer[uart_rx_idx++] = rxData;
                    // 에코 전송: 하드웨어가 자동으로 DE 핀 제어
                    HAL_UART_Transmit(&H_UART, &rxData, 1, 10);
                    char_received_flag = 1;
                }
            }
        }

        // 다시 1바이트 수신 대기 상태로 전환
        HAL_UART_Receive_IT(&H_UART, &rxData, 1);
    }
}
