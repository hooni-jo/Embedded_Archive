/*
 * cli.h
 *
 *  Created on: 2026. 3. 3.
 *      Author: lenovo
 */

#ifndef __CLI_H__
#define __CLI_H__

#include "main.h"

// CLI 엔진 초기화 및 메인 처리 함수
void CLI_Init(void);
void CLI_Process(void);
void CLI_TabAutocomplete(void);
void CLI_HistoryNavigate(int direction);

#endif /* __CLI_H__ */
