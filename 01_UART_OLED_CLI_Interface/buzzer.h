/*
 * buzzer.h
 *
 *  Created on: 2026. 3. 3.
 *      Author: lenovo
 */

#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "main.h"

void Buzzer_Control(uint8_t state);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_SetFrequency(uint32_t freq);
void Buzzer_PlayScale(void);

#endif /* __BUZZER_H__ */
