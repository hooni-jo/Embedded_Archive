/*
 * cli.c
 *
 *  Created on: 2026. 3. 3.
 *      Author: lenovo
 */

#include "cli.h"
#include "uart_driver.h"
#include "led.h"
#include "buzzer.h"
#include "usart.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// --- History 설정 ---
#define MAX_HISTORY 10
static char cmd_history[MAX_HISTORY][64];
static int history_count = 0;

// 명령어 함수 원형 선언
static void cli_help(int argc, char **argv);
static void cli_led(int argc, char **argv);
static void cli_buzzer(int argc, char **argv);
static void cli_cls(int argc, char **argv);
static void cli_history(int argc, char **argv);
static void cli_play(int argc, char **argv);

// 명령어 구조체 정의
typedef struct {
    char *name;           // 명령어 이름
    char *help;           // 도움말 메시지
    void (*func)(int, char**); // 실행할 함수 포인터
} CLI_Command_t;

// [명령어 테이블]
static const CLI_Command_t cmd_table[] = {
    {"help",    "Show all available commands", cli_help},
    {"led",     "Control LED (ex: led 1 on)",  cli_led},
    {"buzzer",  "Control Buzzer (ex: buzzer on)", cli_buzzer},
    {"cls",     "Clear Terminal Screen",       cli_cls},
    {"history", "Show command history",        cli_history},
    {"play",    "Play a scale melody",         cli_play} // 이 줄을 추가!
};

static const int cmd_count = sizeof(cmd_table) / sizeof(CLI_Command_t);

static void add_to_history(char *cmd) {
    if (strlen(cmd) == 0) return;

    // 중복 저장 방지 (방금 친 명령어와 같으면 저장 안 함)
    if (history_count > 0 && strcmp(cmd_history[(history_count - 1) % MAX_HISTORY], cmd) == 0) return;

    strncpy(cmd_history[history_count % MAX_HISTORY], cmd, 63);
    history_count++;
}

static void cli_play(int argc, char **argv) {
    // 하드웨어 제어 로직은 이제 buzzer.c 안에 캡슐화되어 있음
    Buzzer_PlayScale();

    HAL_UART_Transmit(&huart2, (uint8_t*)"Melody finished.\r\n", 18, 100);
}

void CLI_Init(void) {
    HAL_UART_Transmit(&huart2, (uint8_t*)"\033[2J\033[H", 7, 100); // 화면 지우고 시작
    HAL_UART_Transmit(&huart2, (uint8_t*)"STM32 CLI System Ready.\r\nSTM32> ", 31, 100);
}

static int history_view_idx = -1; // 현재 보고 있는 히스토리 위치 (-1은 입력 중)

// [History 탐색 함수]
void CLI_HistoryNavigate(int direction) {
    if (history_count == 0) return;

    // 인덱스 계산
    if (direction == 1) { // 위쪽 화살표
        if (history_view_idx == -1) history_view_idx = history_count - 1;
        else if (history_view_idx > 0 && history_view_idx > (history_count - MAX_HISTORY)) history_view_idx--;
    } else { // 아래쪽 화살표
        if (history_view_idx != -1 && history_view_idx < history_count - 1) history_view_idx++;
        else { // 마지막이면 빈 칸으로
            history_view_idx = -1;
            UART_Driver_ResetBuffer();
        }
    }

    // 터미널 화면의 현재 입력 줄 지우기
    // \r: 커서를 맨 앞으로, \033[K: 커서 이후 라인 끝까지 삭제
    HAL_UART_Transmit(&huart2, (uint8_t*)"\rSTM32> \033[K", 12, 10);

    if (history_view_idx != -1) {
        // 히스토리에서 버퍼로 복사
        strcpy(uart_rx_buffer, cmd_history[history_view_idx % MAX_HISTORY]);
        uart_rx_idx = strlen(uart_rx_buffer);
        // 화면에 출력
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_rx_buffer, uart_rx_idx, 10);
    }
}

void CLI_Process(void) {
    if (!command_ready_flag) return;

    // 1. 히스토리에 원본 명령어 저장
    add_to_history(uart_rx_buffer);
    history_view_idx = -1; // 명령 실행 후 뷰 인덱스 초기화

    // 2. 문자열 토큰화
    char *argv[10];
    int argc = 0;
    char *ptr = strtok(uart_rx_buffer, " ");

    while (ptr != NULL && argc < 10) {
        argv[argc++] = ptr;
        ptr = strtok(NULL, " ");
    }

    if (argc > 0) {
        int found = 0;
        for (int i = 0; i < cmd_count; i++) {
            if (strcmp(argv[0], cmd_table[i].name) == 0) {
                cmd_table[i].func(argc, argv);
                found = 1;
                break;
            }
        }
        if (!found) {
            HAL_UART_Transmit(&huart2, (uint8_t*)"Unknown command. Type 'help'.\r\n", 31, 100);
        }
    }

    UART_Driver_ResetBuffer();
    command_ready_flag = 0;
    HAL_UART_Transmit(&huart2, (uint8_t*)"STM32> ", 7, 100);
}

// [Tab 자동완성] uart_driver.c에서 호출됨
void CLI_TabAutocomplete(void) {
    if (uart_rx_idx == 0) return;

    int matches = 0;
    int last_match_idx = -1;

    for (int i = 0; i < cmd_count; i++) {
        if (strncmp(uart_rx_buffer, cmd_table[i].name, uart_rx_idx) == 0) {
            matches++;
            last_match_idx = i;
        }
    }

    if (matches == 1) {
        // 하나만 일치하면 나머지 글자 자동 완성
        char *remaining = cmd_table[last_match_idx].name + uart_rx_idx;
        int len = strlen(remaining);

        // 하드웨어 버퍼 업데이트 (중요: uart_rx_idx를 늘려줘야 함)
        strcat(uart_rx_buffer, remaining);
        uart_rx_idx += len;

        // 터미널에 완성된 글자 전송
        HAL_UART_Transmit(&huart2, (uint8_t*)remaining, len, 10);
    }
    else if (matches > 1) {
        // 여러 개 일치하면 목록 출력 (리눅스 방식)
        HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 10);
        for (int i = 0; i < cmd_count; i++) {
            if (strncmp(uart_rx_buffer, cmd_table[i].name, uart_rx_idx) == 0) {
                HAL_UART_Transmit(&huart2, (uint8_t*)cmd_table[i].name, strlen(cmd_table[i].name), 10);
                HAL_UART_Transmit(&huart2, (uint8_t*)"  ", 2, 10);
            }
        }
        // 다시 프롬프트와 입력 중이던 글자 복구
        HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nSTM32> ", 9, 10);
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_rx_buffer, uart_rx_idx, 10);
    }
}

// --- 명령어 구현부 ---

static void cli_help(int argc, char **argv) {
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nAvailable Commands:\r\n", 23, 100);
    for (int i = 0; i < cmd_count; i++) {
        char buf[128];
        int len = sprintf(buf, "  %s\t: %s\r\n", cmd_table[i].name, cmd_table[i].help);
        HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 100);
    }
}

static void cli_history(int argc, char **argv) {
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n--- Command History ---\r\n", 27, 100);
    int start = (history_count > MAX_HISTORY) ? (history_count - MAX_HISTORY) : 0;
    for (int i = start; i < history_count; i++) {
        char buf[80];
        int len = sprintf(buf, "%4d  %s\r\n", i + 1, cmd_history[i % MAX_HISTORY]);
        HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 100);
    }
}

// led, buzzer, cls는 기존 코드 유지 (생략 가능)
static void cli_led(int argc, char **argv) {
    if (argc < 3) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Usage: led <num> <on/off>\r\n", 27, 100);
        return;
    }
    int num = atoi(argv[1]);
    uint8_t state = (strcmp(argv[2], "on") == 0) ? LED_ON : LED_OFF;
    LED_Control(num, state);

    char buf[64];
    int len = sprintf(buf, "LED %d set to %s\r\n", num, argv[2]);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 100);
}

static void cli_buzzer(int argc, char **argv) {
    if (argc < 2) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Usage: buzzer <on/off>\r\n", 24, 100);
        return;
    }
    uint8_t state = (strcmp(argv[1], "on") == 0) ? 1 : 0;
    Buzzer_Control(state);

    char buf[64];
    int len = sprintf(buf, "Buzzer set to %s\r\n", argv[1]);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 100);
}

static void cli_cls(int argc, char **argv) {
    HAL_UART_Transmit(&huart2, (uint8_t*)"\033[2J\033[H", 7, 100);
}
