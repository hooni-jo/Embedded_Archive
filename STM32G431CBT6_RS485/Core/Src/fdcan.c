/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "fdcan.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 17;
  hfdcan1.Init.NominalSyncJumpWidth = 2;
  hfdcan1.Init.NominalTimeSeg1 = 15;
  hfdcan1.Init.NominalTimeSeg2 = 4;
  hfdcan1.Init.DataPrescaler = 17;
  hfdcan1.Init.DataSyncJumpWidth = 4;
  hfdcan1.Init.DataTimeSeg1 = 15;
  hfdcan1.Init.DataTimeSeg2 = 4;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */
  // 1. 초기화 직후 바로 필터 및 인터럽트 활성화를 위해 커스텀 함수 호출
    if (CAN_Config_Init() != HAL_OK)
    {
        Error_Handler();
    }
  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PB8-BOOT0     ------> FDCAN1_RX
    PB9     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PB8-BOOT0     ------> FDCAN1_RX
    PB9     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
volatile uint8_t can_rx_data[8];
volatile uint32_t can_rx_id;
volatile uint8_t can_rx_flag = 0;

FDCAN_TxHeaderTypeDef TxHeader;
FDCAN_RxHeaderTypeDef RxHeader;

HAL_StatusTypeDef CAN_Config_Init(void)
{
    // [1] 트랜시버 Standby 해제 (PA10 Low)
    HAL_GPIO_WritePin(CAN_STB_GPIO_Port, CAN_STB_Pin, GPIO_PIN_RESET);

    // [2] ★★★ 글로벌 필터 설정 (필수! 없으면 수신 거부될 수 있음)
    //     Non-matching Standard → RXFIFO0으로 받기
    //     Non-matching Extended → 거부
    //     Remote Standard/Extended → 거부
    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
            FDCAN_ACCEPT_IN_RX_FIFO0,  // Non-matching Standard → FIFO0
            FDCAN_REJECT,              // Non-matching Extended → 거부
            FDCAN_FILTER_REMOTE,       // Remote Standard → 필터로
            FDCAN_FILTER_REMOTE        // Remote Extended → 필터로
    ) != HAL_OK) return HAL_ERROR;

    // [3] 개별 필터 설정 (Standard ID 전체 수신)
    FDCAN_FilterTypeDef sFilterConfig = {0};
    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = 0x000;
    sFilterConfig.FilterID2 = 0x7FF;

    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK) return HAL_ERROR;
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) return HAL_ERROR;
    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) return HAL_ERROR;

    return HAL_OK;
}

HAL_StatusTypeDef CAN_Send_Test(void)
{
    uint8_t TxData[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    TxHeader.Identifier = 0x321;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF; // ★ Classic CAN: BRS 끄기
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;  // ★ Classic CAN 프레임 포맷
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    return HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData);
}

HAL_StatusTypeDef CAN_Send_Data(uint8_t *pData, uint8_t len)
{
    if (len > 8) len = 8; // Classic CAN은 8바이트까지만 지원
    
    TxHeader.Identifier = 0x321;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    
    // 길이에 따른 DLC 설정
    switch(len) {
        case 0: TxHeader.DataLength = FDCAN_DLC_BYTES_0; break;
        case 1: TxHeader.DataLength = FDCAN_DLC_BYTES_1; break;
        case 2: TxHeader.DataLength = FDCAN_DLC_BYTES_2; break;
        case 3: TxHeader.DataLength = FDCAN_DLC_BYTES_3; break;
        case 4: TxHeader.DataLength = FDCAN_DLC_BYTES_4; break;
        case 5: TxHeader.DataLength = FDCAN_DLC_BYTES_5; break;
        case 6: TxHeader.DataLength = FDCAN_DLC_BYTES_6; break;
        case 7: TxHeader.DataLength = FDCAN_DLC_BYTES_7; break;
        default: TxHeader.DataLength = FDCAN_DLC_BYTES_8; break;
    }
    
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;  // ★ Classic CAN: BRS 끄기
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;   // ★ Classic CAN 프레임 포맷
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    uint8_t txData[8] = {0};
    for(int i=0; i<len; i++) {
        txData[i] = pData[i];
    }

    return HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, txData);
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
    {
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, (uint8_t *)can_rx_data) == HAL_OK)
        {
            can_rx_id = RxHeader.Identifier;
            can_rx_flag = 1;
        }
    }
}

// ★ 폴링 방식 수신 (인터럽트가 안 먹힐 때 대비)
HAL_StatusTypeDef CAN_Poll_Receive(void)
{
    if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0)
    {
        if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, (uint8_t *)can_rx_data) == HAL_OK)
        {
            can_rx_id = RxHeader.Identifier;
            can_rx_flag = 1;
            return HAL_OK;
        }
    }
    return HAL_ERROR;
}

// ★ FDCAN 상태 진단 (OLED 디버그용) - 2줄 출력
void CAN_Get_Diag(char *line1, uint8_t size1, char *line2, uint8_t size2)
{
    FDCAN_ProtocolStatusTypeDef ps;
    FDCAN_ErrorCountersTypeDef ec;
    HAL_FDCAN_GetProtocolStatus(&hfdcan1, &ps);
    HAL_FDCAN_GetErrorCounters(&hfdcan1, &ec);
    
    uint32_t fifoLevel = HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0);
    
    // 줄1: FDCAN 버스 상태
    snprintf(line1, size1, "BO:%d EP:%d LEC:%d F:%lu",
        (int)ps.BusOff, (int)ps.ErrorPassive, (int)ps.LastErrorCode, fifoLevel);
    
    // 줄2: HAL상태 + STB핀 + CCCR INIT비트
    uint8_t stb_val = HAL_GPIO_ReadPin(CAN_STB_GPIO_Port, CAN_STB_Pin);
    uint32_t cccr = FDCAN1->CCCR;
    uint8_t init_bit = (cccr & 0x01) ? 1 : 0;  // CCCR.INIT
    uint8_t cce_bit  = (cccr & 0x02) ? 1 : 0;  // CCCR.CCE
    
    snprintf(line2, size2, "ST:%d STB:%d I:%d C:%d T:%lu R:%lu",
        (int)hfdcan1.State, (int)stb_val, init_bit, cce_bit,
        ec.TxErrorCnt, ec.RxErrorCnt);
}

// ★★★ FDCAN 내부 루프백 테스트
// 트랜시버를 거치지 않고 칩 내부에서 TX→RX 연결하여 테스트
// 반환: 1 = PASS, 0 = FAIL
uint8_t CAN_Loopback_Test(void)
{
    uint8_t result = 0;
    
    // [1] 현재 FDCAN 정지
    HAL_FDCAN_Stop(&hfdcan1);
    HAL_FDCAN_DeInit(&hfdcan1);
    
    // [2] 내부 루프백 모드로 재초기화
    hfdcan1.Instance = FDCAN1;
    hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
    hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    hfdcan1.Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;  // ★ 핵심: 내부 루프백
    hfdcan1.Init.AutoRetransmission = ENABLE;
    hfdcan1.Init.TransmitPause = DISABLE;
    hfdcan1.Init.ProtocolException = DISABLE;
    hfdcan1.Init.NominalPrescaler = 17;
    hfdcan1.Init.NominalSyncJumpWidth = 4;
    hfdcan1.Init.NominalTimeSeg1 = 15;
    hfdcan1.Init.NominalTimeSeg2 = 4;
    hfdcan1.Init.DataPrescaler = 17;
    hfdcan1.Init.DataSyncJumpWidth = 4;
    hfdcan1.Init.DataTimeSeg1 = 15;
    hfdcan1.Init.DataTimeSeg2 = 4;
    hfdcan1.Init.StdFiltersNbr = 1;
    hfdcan1.Init.ExtFiltersNbr = 0;
    hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    
    if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK) return 0;
    
    // [3] 글로벌 필터 + 필터 설정
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
        FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_REJECT,
        FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
    
    FDCAN_FilterTypeDef sf = {0};
    sf.IdType = FDCAN_STANDARD_ID;
    sf.FilterIndex = 0;
    sf.FilterType = FDCAN_FILTER_RANGE;
    sf.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sf.FilterID1 = 0x000;
    sf.FilterID2 = 0x7FF;
    HAL_FDCAN_ConfigFilter(&hfdcan1, &sf);
    
    // [4] FDCAN Start
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) return 0;
    
    // [5] 테스트 메시지 전송
    FDCAN_TxHeaderTypeDef txh;
    txh.Identifier = 0x7FF;
    txh.IdType = FDCAN_STANDARD_ID;
    txh.TxFrameType = FDCAN_DATA_FRAME;
    txh.DataLength = FDCAN_DLC_BYTES_4;
    txh.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txh.BitRateSwitch = FDCAN_BRS_OFF;
    txh.FDFormat = FDCAN_CLASSIC_CAN;
    txh.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txh.MessageMarker = 0;
    
    uint8_t txData[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txh, txData) != HAL_OK) return 0;
    
    // [6] 수신 대기 (최대 50ms)
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 50)
    {
        if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0)
        {
            FDCAN_RxHeaderTypeDef rxh;
            uint8_t rxData[4] = {0};
            if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rxh, rxData) == HAL_OK)
            {
                // 검증: ID와 데이터가 일치하는지
                if (rxh.Identifier == 0x7FF &&
                    rxData[0] == 0xDE && rxData[1] == 0xAD &&
                    rxData[2] == 0xBE && rxData[3] == 0xEF)
                {
                    result = 1; // PASS!
                }
            }
            break;
        }
    }
    
    // [7] 정지 후 Normal 모드로 복원
    HAL_FDCAN_Stop(&hfdcan1);
    HAL_FDCAN_DeInit(&hfdcan1);
    
    hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;  // Normal 복원
    if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK) return 0;
    if (CAN_Config_Init() != HAL_OK) return 0;  // 필터+Start+인터럽트 재설정
    
    return result;
}
/* USER CODE END 1 */
