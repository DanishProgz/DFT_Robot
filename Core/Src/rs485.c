/**
 * @file    rs485.c
 * @brief   RS-485 communication layer implementation
 *
 * @details Uses CubeMX-initialized USART2 (extern huart2 from main.c).
 *          UART peripheral, GPIO, and NVIC are all configured by CubeMX.
 *
 *          This module only handles:
 *            - Starting the first HAL_UART_Receive_IT (rs485_init)
 *            - Ring buffer filling (HAL_UART_RxCpltCallback)
 *            - DE/RE toggling for TX (rs485_send_raw)
 *            - Frame parsing (rs485_poll)
 *
 *          ISR chain (no custom handler needed):
 *            CubeMX USART2_IRQHandler -> HAL_UART_IRQHandler
 *              -> HAL_UART_RxCpltCallback (defined here)
 */

#include "rs485.h"
#include "pins.h"
#include <string.h>
#include <stdio.h>

/* ================================================================
 * CubeMX UART Handle
 *
 * Declared in CubeMX-generated main.c as:
 *   UART_HandleTypeDef huart2;
 * ================================================================ */
extern UART_HandleTypeDef huart2;

/* ================================================================
 * Private State
 * ================================================================ */

static uint8_t board_addr;

/* ISR ring buffer */
static volatile uint8_t rx_ring[RS485_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static uint16_t rx_tail = 0;
static volatile uint8_t rx_byte;

/* Frame assembly buffer */
static uint8_t frame_buf[RS485_BUF_SIZE];
static uint16_t frame_len = 0;

/* ================================================================
 * Initialization
 *
 * CubeMX already configured:
 *   - USART2 peripheral (baud, 8N1, TX+RX)
 *   - PA2/PA3 as USART2_TX/RX (alternate function)
 *   - PA4 as GPIO output (DE/RE)
 *   - NVIC priority for USART2
 *
 * We just need to:
 *   - Store the board address
 *   - Ensure DE/RE starts in RX mode
 *   - Kick off interrupt-driven reception
 * ================================================================ */

void rs485_init(uint8_t board_id)
{
    board_addr = board_id;

    /* Ensure DE/RE is LOW (RX mode) */
    HAL_GPIO_WritePin(RS485_DERE_PORT, RS485_DERE_PIN, GPIO_PIN_RESET);

    /* Start interrupt-driven reception — first byte */
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_byte, 1);
}

/* ================================================================
 * UART RX Callback
 *
 * Called by HAL when a byte is received. Stores it in the ring
 * buffer and re-arms the interrupt for the next byte.
 *
 * ISR chain: USART2_IRQHandler (CubeMX) -> HAL_UART_IRQHandler
 *            -> this callback. No custom IRQ handler needed.
 * ================================================================ */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        rx_ring[rx_head] = rx_byte;
        rx_head = (rx_head + 1) % RS485_BUF_SIZE;
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_byte, 1);
    }
}

/* ================================================================
 * Transmission
 * ================================================================ */

void rs485_send_raw(const uint8_t *data, uint16_t len)
{
    /* Switch to TX mode */
    HAL_GPIO_WritePin(RS485_DERE_PORT, RS485_DERE_PIN, GPIO_PIN_SET);
    for (volatile int i = 0; i < 100; i++) {} /* DE settle time */

    /* Blocking transmit */
    HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 100);

    /* Wait for last byte to fully shift out */
    while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET) {}

    /* Switch back to RX mode */
    HAL_GPIO_WritePin(RS485_DERE_PORT, RS485_DERE_PIN, GPIO_PIN_RESET);
}

void rs485_send_ack(const char *cmd, const char *result)
{
    char buf[RS485_BUF_SIZE];
    int n;
    if (result && result[0]) {
        n = snprintf(buf, sizeof(buf), "@%02d:ACK:%s:%s\n", board_addr, cmd, result);
    } else {
        n = snprintf(buf, sizeof(buf), "@%02d:ACK:%s\n", board_addr, cmd);
    }
    rs485_send_raw((uint8_t *)buf, n);
}

void rs485_send_err(const char *cmd, const char *reason)
{
    char buf[RS485_BUF_SIZE];
    int n = snprintf(buf, sizeof(buf), "@%02d:ERR:%s:%s\n", board_addr, cmd, reason);
    rs485_send_raw((uint8_t *)buf, n);
}

/* ================================================================
 * Frame Parsing
 *
 * Input format:  @XX:CMD:PARAM\n
 * Only frames addressed to this board are returned as valid.
 * ================================================================ */

static RS485_Frame_t parse_frame(uint8_t *buf, uint16_t len)
{
    RS485_Frame_t f = { .valid = false };

    /* Strip trailing \r\n */
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        len--;

    /* Minimum valid: @XX:C = 5 chars */
    if (len < 5 || buf[0] != '@') return f;

    /* Check address */
    uint8_t addr = (buf[1] - '0') * 10 + (buf[2] - '0');
    if (addr != board_addr) return f;
    if (buf[3] != ':') return f;

    /* Split CMD and PARAM at the next ':' after position 4 */
    char *start     = (char *)&buf[4];
    uint16_t remaining = len - 4;

    char *colon = NULL;
    for (uint16_t i = 0; i < remaining; i++) {
        if (start[i] == ':') {
            colon = &start[i];
            break;
        }
    }

    if (colon) {
        uint16_t cmd_len   = colon - start;
        uint16_t param_len = remaining - cmd_len - 1;
        if (cmd_len >= sizeof(f.cmd))     cmd_len   = sizeof(f.cmd) - 1;
        if (param_len >= sizeof(f.param)) param_len = sizeof(f.param) - 1;
        memcpy(f.cmd, start, cmd_len);
        f.cmd[cmd_len] = '\0';
        memcpy(f.param, colon + 1, param_len);
        f.param[param_len] = '\0';
    } else {
        uint16_t cmd_len = remaining;
        if (cmd_len >= sizeof(f.cmd)) cmd_len = sizeof(f.cmd) - 1;
        memcpy(f.cmd, start, cmd_len);
        f.cmd[cmd_len] = '\0';
        f.param[0] = '\0';
    }

    f.valid = true;
    return f;
}

RS485_Frame_t rs485_poll(void)
{
    RS485_Frame_t empty = { .valid = false };

    while (rx_tail != rx_head) {
        uint8_t c = rx_ring[rx_tail];
        rx_tail = (rx_tail + 1) % RS485_BUF_SIZE;

        if (c == '\n') {
            frame_buf[frame_len++] = c;
            RS485_Frame_t f = parse_frame(frame_buf, frame_len);
            frame_len = 0;
            if (f.valid) return f;
        } else {
            if (frame_len < RS485_BUF_SIZE - 1) {
                frame_buf[frame_len++] = c;
            }
        }
    }

    return empty;
}
