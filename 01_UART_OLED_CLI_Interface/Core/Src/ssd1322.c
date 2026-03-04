/*
 * ssd1322.c
 *
 *  Created on: 2026. 2. 27.
 *      Author: lenovo
 */

#include "ssd1322.h"
#include "spi.h"
#include "font.h"
#include <string.h>

// 전역 버퍼 (static으로 선언하여 이 파일 내에서만 접근)
static uint8_t OLED_Buffer[8192];

void SSD1322_HW_Reset(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
    HAL_Delay(100);
}

void OLED_WriteCmd(uint8_t cmd) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // DC Low
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); // CS Low
    HAL_SPI_Transmit(&hspi3, &cmd, 1, 10);
    while(__HAL_SPI_GET_FLAG(&hspi3, SPI_FLAG_BSY));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
}

void OLED_WriteData(uint8_t data) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);   // DC High
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); // CS Low
    HAL_SPI_Transmit(&hspi3, &data, 1, 10);
    while(__HAL_SPI_GET_FLAG(&hspi3, SPI_FLAG_BSY));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
}

void SSD1322_Init(void) {
    SSD1322_HW_Reset();
    OLED_WriteCmd(0xFD); OLED_WriteData(0x12);
    OLED_WriteCmd(0xAE);
    OLED_WriteCmd(0xB3); OLED_WriteData(0x91);
    OLED_WriteCmd(0xCA); OLED_WriteData(0x3F);
    OLED_WriteCmd(0xA2); OLED_WriteData(0x00);
    OLED_WriteCmd(0xA1); OLED_WriteData(0x00);
    OLED_WriteCmd(0xA0); OLED_WriteData(0x14); OLED_WriteData(0x11);
    OLED_WriteCmd(0xAB); OLED_WriteData(0x01);
    OLED_WriteCmd(0xB1); OLED_WriteData(0xE2);
    OLED_WriteCmd(0xBC); OLED_WriteData(0x1F);
    OLED_WriteCmd(0xBE); OLED_WriteData(0x07);
    OLED_WriteCmd(0xC1); OLED_WriteData(0x9F);
    OLED_WriteCmd(0xC7); OLED_WriteData(0x0F);
    OLED_WriteCmd(0xB9);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0xAF);
    HAL_Delay(100);
}

void SSD1322_Clear(void) {
    memset(OLED_Buffer, 0, sizeof(OLED_Buffer));
}

void SSD1322_DrawPixel(uint16_t x, uint16_t y, uint8_t gray) {
    if (x >= 256 || y >= 64) return;
    uint16_t idx = (y * 128) + (x / 2);
    if (x % 2 == 0) OLED_Buffer[idx] = (OLED_Buffer[idx] & 0x0F) | (gray << 4);
    else OLED_Buffer[idx] = (OLED_Buffer[idx] & 0xF0) | (gray & 0x0F);
}

void SSD1322_Update(void) {
    OLED_WriteCmd(0x15); OLED_WriteData(0x1C); OLED_WriteData(0x5B);
    OLED_WriteCmd(0x75); OLED_WriteData(0x00); OLED_WriteData(0x3F);
    OLED_WriteCmd(0x5C);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi3, OLED_Buffer, 8192, 100);
    while(__HAL_SPI_GET_FLAG(&hspi3, SPI_FLAG_BSY));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
}

void SSD1322_PutChar(uint16_t x, uint16_t y, char c, uint8_t gray) {
    if (c < 32 || c > 126) return;
    const uint8_t* glyph = &Font5x7[(c - 32) * 5];
    for (int i = 0; i < 5; i++) {
        uint8_t line = glyph[i];
        for (int j = 0; j < 7; j++) {
            if (line & (1 << j)) SSD1322_DrawPixel(x + i, y + j, gray);
        }
    }
}

void SSD1322_PutString(uint16_t x, uint16_t y, char* str, uint8_t gray) {
    while (*str) {
        SSD1322_PutChar(x, y, *str++, gray);
        x += 6;
        if (x > 250) break;
    }
}
