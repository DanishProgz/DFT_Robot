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


extern uint8_t rx_byte;



void Process_Command(char *cmd);
#endif
