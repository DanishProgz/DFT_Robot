/*
 * Comm.c
 * UART Command Handler for DFT Robot - Leg Motor Board
 *
 * Command Set:
 *   RUN_CW\r\n        -> All 3 motors free run CW
 *   RUN_CCW\r\n       -> All 3 motors free run CCW
 *   STOP\r\n          -> All 3 motors stop immediately
 *                        Response: "OK STOP <m1_steps> <m2_steps> <m3_steps>\r\n"
 *   MOVE <steps>\r\n  -> All 3 motors move exactly N steps
 *                        + = CW, - = CCW
 *                        Response on start: "OK MOVE <steps>\r\n"
 *                        Response on done:  "DONE <m1> <m2> <m3>\r\n"
 *
 * Notes:
 *   - STOP is always handled regardless of motor state
 *   - DONE is sent from IRQ when all motors finish a MOVE
 */

#include "main.h"
#include "Comm.h"
#include "stepper.h"
#include "usart.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CMD_BUF_SIZE 64

uint8_t rx_byte;
char    cmd_buf[CMD_BUF_SIZE];
uint8_t cmd_index = 0;

/* ---------------------------------------------------------- */

void send_uart(const char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
}

/* ---------------------------------------------------------- */

void Process_Command(char *cmd)
{
    char buf[64];

    /* ---- RUN_CW ---- */
    if (strcmp(cmd, "RUN_CW") == 0)
    {
        Stepper_Run(LegMotor_1, STEPPER_CW);
        Stepper_Run(LegMotor_2, STEPPER_CW);
        Stepper_Run(LegMotor_3, STEPPER_CW);
        send_uart("OK RUN_CW\r\n");
    }

    /* ---- RUN_CCW ---- */
    else if (strcmp(cmd, "RUN_CCW") == 0)
    {
        Stepper_Run(LegMotor_1, STEPPER_CCW);
        Stepper_Run(LegMotor_2, STEPPER_CCW);
        Stepper_Run(LegMotor_3, STEPPER_CCW);
        send_uart("OK RUN_CCW\r\n");
    }

    /* ---- STOP (always handled) ---- */
    else if (strcmp(cmd, "STOP") == 0)
    {
        uint32_t s1 = Stepper_Stop(LegMotor_1);
        uint32_t s2 = Stepper_Stop(LegMotor_2);
        uint32_t s3 = Stepper_Stop(LegMotor_3);
        snprintf(buf, sizeof(buf), "OK STOP %lu %lu %lu\r\n", s1, s2, s3);
        send_uart(buf);
    }

    /* ---- MOVE <steps> ---- */
    else if (strncmp(cmd, "MOVE ", 5) == 0)
    {
        int32_t steps = atoi(cmd + 5);
        if (steps == 0)
        {
            send_uart("ERR MOVE requires non-zero steps\r\n");
            return;
        }
        Stepper_Move(LegMotor_1, steps);
        Stepper_Move(LegMotor_2, steps);
        Stepper_Move(LegMotor_3, steps);
        snprintf(buf, sizeof(buf), "OK MOVE %ld\r\n", steps);
        send_uart(buf);
    }

    /* ---- Unknown ---- */
    else
    {
        send_uart("ERR\r\n");
    }
}

/* ---------------------------------------------------------- */

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == USART2)
//    {
//        if (rx_byte == '\r' || rx_byte == '\n')
//        {
//            if (cmd_index > 0)
//            {
//                cmd_buf[cmd_index] = '\0';
//                Process_Command(cmd_buf);
//                cmd_index = 0;
//            }
//        }
//        else
//        {
//            if (cmd_index < CMD_BUF_SIZE - 1)
//                cmd_buf[cmd_index++] = rx_byte;
//        }
//
//        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
//    }
//}
