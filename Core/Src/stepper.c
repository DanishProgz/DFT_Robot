/*
 * stepper.c
 * TB6560 Stepper Motor Driver for STM32F401CDU6
 *
 * Uses TIM2 CH1 (PA0) for hardware PWM pulse generation.
 * TIM2 Update Interrupt counts pulses and stops at target.
 *
 * Timer Config (from CubeMX):
 *   - PSC  = 83   -> 84MHz / 84 = 1MHz tick
 *   - ARR  = 499  -> 1MHz / 500 = 2kHz step rate
 *   - CCR1 = 250  -> 50% duty cycle
 *
 * At 2kHz step rate with 3200 steps/rev:
 *   Full rotation = 3200 / 2000 = 1.6 seconds
 */

#include "stepper.h"

/* Private handle */
static Stepper_Handle_t stepper = {0};

/* ---------- GPIO helpers ---------- */

static void Stepper_SetDirection(uint8_t dir)
{
    if (dir == STEPPER_CW)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);    /* CW+ HIGH */
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);  /* CW+ LOW */
    }
}

static void Stepper_SetEnable(uint8_t enable)
{
    if (enable)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);  /* EN+ LOW = enabled (active low) */
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);    /* EN+ HIGH = disabled */
    }
}

/* ---------- Public functions ---------- */

void Stepper_Init(void)
{
    stepper.target_steps  = 0;
    stepper.current_steps = 0;
    stepper.state         = STEPPER_IDLE;
    stepper.direction     = STEPPER_CW;

    /* Set fixed-low pins */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,  GPIO_PIN_RESET);  /* CLK- LOW */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);  /* CW-  LOW */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);  /* EN-  LOW */

    /* Disable motor by default */
    Stepper_Disable();
}

void Stepper_Move(int32_t steps)
{
    if (steps == 0) return;
    if (stepper.state == STEPPER_RUNNING) return;

    /* Set direction based on sign */
    if (steps > 0)
    {
        stepper.direction = STEPPER_CW;
        stepper.target_steps = (uint32_t)steps;
    }
    else
    {
        stepper.direction = STEPPER_CCW;
        stepper.target_steps = (uint32_t)(-steps);
    }

    stepper.current_steps = 0;
    stepper.state = STEPPER_RUNNING;

    /* Set direction pin */
    Stepper_SetDirection(stepper.direction);

    /* Enable driver */
    Stepper_SetEnable(1);

    /* Small delay for driver to settle */
//    HAL_Delay(1);

    /* Start PWM on TIM2 CH1 (PA0) - no MOE needed, TIM2 is general purpose */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    /* Enable TIM2 update interrupt to count pulses */
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
}

void Stepper_Stop(void)
{
    /* Stop PWM */
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

    /* Disable update interrupt */
    __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);

    stepper.current_steps = 0;
    stepper.target_steps  = 0;
    stepper.state = STEPPER_IDLE;
}

void Stepper_Enable(void)
{
    Stepper_SetEnable(1);
}

void Stepper_Disable(void)
{
    Stepper_SetEnable(0);
}

uint8_t Stepper_IsBusy(void)
{
    return (stepper.state == STEPPER_RUNNING) ? 1 : 0;
}

/* ---------- IRQ Handler ---------- */

void Stepper_IRQ_Handler(void)
{
    if (stepper.state != STEPPER_RUNNING) return;

    stepper.current_steps++;

    if (stepper.current_steps >= stepper.target_steps)
    {
        Stepper_Stop();
    }
}
