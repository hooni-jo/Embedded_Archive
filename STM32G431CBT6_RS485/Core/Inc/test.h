/*
 * test.h
 *
 *  Created on: 2026. 3. 18.
 *      Author: lenovo
 */

#ifndef __TEST_H__
#define __TEST_H__

#include "main.h"

extern volatile uint8_t is_test_mode;

void CAN_Test_Init(void);
void CAN_Test_Run(void);   // main.c의 while(1) 가장 윗단에서 호출
void CAN_Test_Start(uint8_t pattern);
void CAN_Test_Stop(void);

#endif
