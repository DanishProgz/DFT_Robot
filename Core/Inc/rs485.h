/**
 * @file    rs485.h
 * @brief   RS-485 half-duplex communication layer
 *
 * @details CubeMX handles USART2 init, GPIO, and NVIC config.
 *          This module provides:
 *            - DE/RE direction control for half-duplex
 *            - Interrupt-driven ring buffer reception
 *            - Frame parsing (@XX:CMD:PARAM\n)
 *            - ACK/ERR response formatting
 *
 *          The UART ISR chain is handled by CubeMX:
 *            USART2_IRQHandler() -> HAL_UART_IRQHandler()
 *              -> HAL_UART_RxCpltCallback() [in this module]
 *              -> stores byte in ring buffer
 *
 *          No custom IRQ handler needed. Just call rs485_init()
 *          after CubeMX MX_USART2_UART_Init() to start reception.
 */

#ifndef RS485_H
#define RS485_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define RS485_BUF_SIZE  128

/**
 * @brief  Parsed command frame from the Pi.
 */
typedef struct {
    char cmd[16];       /**< Command string, e.g. "M1_CW", "STOP", "LIGHTS" */
    char param[64];     /**< Parameter string, e.g. "1000", "ON", "" */
    bool valid;         /**< true if a complete, correctly-addressed frame was received */
} RS485_Frame_t;

/**
 * @brief  Initialize RS-485 layer. Call after CubeMX MX_USART2_UART_Init().
 *         Sets board address, ensures DE/RE is in RX mode, starts
 *         interrupt-driven reception.
 * @param  board_id  This board's address (1, 2, or 3)
 */
void rs485_init(uint8_t board_id);

/**
 * @brief  Poll for incoming frames. Call from main loop.
 * @return Parsed frame if valid. Frame with valid=false if nothing ready.
 */
RS485_Frame_t rs485_poll(void);

/**
 * @brief  Send an ACK response: @XX:ACK:CMD:RESULT\n
 */
void rs485_send_ack(const char *cmd, const char *result);

/**
 * @brief  Send an ERR response: @XX:ERR:CMD:REASON\n
 */
void rs485_send_err(const char *cmd, const char *reason);

/**
 * @brief  Send raw bytes on the RS-485 bus (handles DE/RE toggling).
 */
void rs485_send_raw(const uint8_t *data, uint16_t len);

#endif /* RS485_H */
