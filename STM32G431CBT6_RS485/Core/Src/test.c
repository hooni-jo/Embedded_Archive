#include "test.h"
#include "fdcan.h"
#include <stdio.h>
#include <string.h>

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_TxHeaderTypeDef TxHeader;

volatile uint8_t is_test_mode = 0;
static uint32_t msg_count = 0;
static uint32_t last_tick = 0;
static uint8_t current_pattern = 0; // [추가] 사용자가 입력한 패턴 저장용

void CAN_Test_Init(void) {
    is_test_mode = 0;
}

void CAN_Test_Start(uint8_t pattern) {
    msg_count = 0;
    last_tick = HAL_GetTick();
    current_pattern = pattern;  // 입력받은 패턴 저장
    is_test_mode = 1;           // 테스트 활성화 (0이 아니면 시작)
    printf("\r\n[SYSTEM] Throughput Test Start! Pattern: 0x%02X\r\n", pattern);
}

void CAN_Test_Stop(void) {
    is_test_mode = 0;
    printf("\r\n[SYSTEM] Test Stopped. Final Count: %lu\r\n", msg_count);
}

void CAN_Test_Run(void) {
    if (!is_test_mode) return;

    if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0) {
        uint8_t tx_data[8];
        static uint32_t seq = 0;

        // [수정] 하드코딩된 0x55, 0x00 대신 사용자가 입력한 패턴으로 채움
        memset(tx_data, current_pattern, 8);

        // 시퀀스 번호는 유지 (파이썬에서 유실률 계산 시 필요함)
        // 앞 4바이트를 seq로 덮어쓰고 나머지 4바이트는 입력한 패턴 유지
        memcpy(tx_data, &seq, 4);

        // TxHeader 설정 (Classic CAN 500k 기준 유지)
        TxHeader.Identifier = 0x123;
        TxHeader.IdType = FDCAN_STANDARD_ID;
        TxHeader.TxFrameType = FDCAN_DATA_FRAME;
        TxHeader.DataLength = FDCAN_DLC_BYTES_8;
        TxHeader.FDFormat = FDCAN_CLASSIC_CAN;

        if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, tx_data) == HAL_OK) {
            msg_count++;
            seq++;
        }
    }

    // 1초마다 통계 출력
    if (HAL_GetTick() - last_tick >= 1000) {
        uint32_t ecr = FDCAN1->ECR;
        printf("\r[RUNNING] FPS: %lu | TEC: %lu | REC: %lu",
               msg_count, (ecr & 0xFF), ((ecr >> 8) & 0x7F));

        msg_count = 0;
        last_tick = HAL_GetTick();
    }
}
