/*
 * ssd1322.h
 *
 *  Created on: 2026. 2. 27.
 *      Author: lenovo
 */

#ifndef __SSD1322_H__
#define __SSD1322_H__

#include "main.h"

// 함수 선언
void SSD1322_HW_Reset(void);
void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteData(uint8_t data);
void SSD1322_Init(void);
void SSD1322_Clear(void);
void SSD1322_DrawPixel(uint16_t x, uint16_t y, uint8_t gray);
void SSD1322_Update(void);
void SSD1322_PutChar(uint16_t x, uint16_t y, char c, uint8_t gray);
void SSD1322_PutString(uint16_t x, uint16_t y, char* str, uint8_t gray);

#endif
