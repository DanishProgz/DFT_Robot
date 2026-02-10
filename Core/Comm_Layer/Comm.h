/*
 * Comm.h
 *
 *  Created on: Feb 10, 2026
 *      Author: danish
 */

#ifndef UART_CONSOLE_H
#define UART_CONSOLE_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

void UartConsole_Init(UART_HandleTypeDef *huart);
void UartConsole_Task(void);                 // call in main loop
void UartConsole_Send(const uint8_t *data, uint16_t len);
void UartConsole_Printf(const char *fmt, ...);



void UartConsole_RxCpltCallback(UART_HandleTypeDef *huart);
#endif
