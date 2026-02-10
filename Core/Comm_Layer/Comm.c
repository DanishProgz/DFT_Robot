


#include "main.h"
#include "Comm.h"
#include "stepper.h"
#include "usart.h"
#include "string.h"


#define CMD_BUF_SIZE 64

uint8_t rx_byte;
char cmd_buf[CMD_BUF_SIZE];
uint8_t cmd_index = 0;



void send_uart(const char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
}

int steps;
void Process_Command(char *cmd)
{
    if (strcmp(cmd, "UP") == 0)
    {
        Stepper_Move(10);
        send_uart("OK UP\r\n");
    }
    else if (strcmp(cmd, "DOWN") == 0)
    {
        Stepper_Move(-10);
        send_uart("OK DOWN\r\n");
    }
    else
    {
        send_uart("ERR\r\n");
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (rx_byte == '\r' || rx_byte == '\n')
        {
            if (cmd_index > 0)
            {
                cmd_buf[cmd_index] = '\0';

                Process_Command(cmd_buf);

                cmd_index = 0;
            }
        }
        else
        {
            if (cmd_index < CMD_BUF_SIZE - 1)
            {
                cmd_buf[cmd_index++] = rx_byte;
            }
        }

        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}


