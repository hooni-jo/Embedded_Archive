/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "fdcan.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "font.h"
#include "ssd1322.h"
#include "uart_driver.h"
#include "led.h"
#include "buzzer.h"
#include "cli.h"
#include "fdcan.h"
#include "test.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI3_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_FDCAN1_Init();
  /* USER CODE BEGIN 2 */
    // OLED 초기화
    SSD1322_Init();
    UART_Driver_Init();
    LED_All_Off();
    CLI_Init();
      
    SSD1322_Clear();
    SSD1322_Update();
  
    SSD1322_PutString(0, 0, "SYSTEM READY", 15);
    SSD1322_Update();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    char can_msg_buf[40];

    while (1)
    {
        // [1] CAN Throughput 테스트 핸들러 (최상단 배치)
        // is_test_mode가 0일 때는 내부에서 아무것도 하지 않고 즉시 반환됩니다.
        CAN_Test_Run();

        // [2] CLI 처리는 상시 가동
        // 테스트 도중에도 'can_test stop' 명령을 즉시 처리할 수 있도록 블록 밖에 위치합니다.
        CLI_Process();

        // [3] 리소스 격리(Isolation) 전략
        // 테스트 모드(1:Best, 2:Worst)가 아닐 때만 무거운 OLED/수신 로직을 수행합니다.
        if (is_test_mode == 0)
        {
            // [A] 폴링 방식 CAN 수신
            CAN_Poll_Receive();

            // [B] CAN 수신 데이터 처리 및 디스플레이 업데이트
            if (can_rx_flag)
            {
                can_rx_flag = 0;
                SSD1322_Clear();

                // 1. CAN ID 및 HEX 데이터 변환
                sprintf(can_msg_buf, "CAN RX ID: 0x%03lX", can_rx_id);
                SSD1322_PutString(0, 0, can_msg_buf, 15);

                sprintf(can_msg_buf, "HEX: %02X %02X %02X %02X %02X %02X %02X %02X",
                        can_rx_data[0], can_rx_data[1], can_rx_data[2], can_rx_data[3],
                        can_rx_data[4], can_rx_data[5], can_rx_data[6], can_rx_data[7]);
                SSD1322_PutString(0, 32, can_msg_buf, 15);

                // 2. ASCII 데이터 변환 및 출력
                char ascii_buf[9] = {0};
                memcpy(ascii_buf, (const char*)can_rx_data, 8);
                sprintf(can_msg_buf, "TXT: %s", ascii_buf);
                SSD1322_PutString(0, 48, can_msg_buf, 15);

                // [핵심] SPI 전송 부하가 큰 함수이므로 테스트 중에는 실행을 차단합니다.
                SSD1322_Update();
                LED_Toggle(1);
            }
        }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
