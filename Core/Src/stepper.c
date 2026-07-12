/*
 * stepper.c
 * TB6560 Stepper Motor Driver for STM32F401CDU6
 *
 * Uses TIM2 CH1 (PA0) for Motor 1 PWM pulse generation.
 * Uses TIM4 CH4 (PB9) for Motor 2 PWM pulse generation.
 * Uses TIM3 CH2 (PA7) for Motor 3 PWM pulse generation.
 * TIM Update Interrupts count pulses and stop at target (0 = free run).
 *
 * Timer Config (from CubeMX):
 *   - PSC  = 83   -> 84MHz / 84 = 1MHz tick
 *   - ARR  = 499  -> 1MHz / 500 = 2kHz step rate
 *   - CCR  = 255  -> ~50% duty cycle
 */

#include "stepper.h"
#include "main.h"
#include <string.h>

/* Timer handles from CubeMX */
extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim4;

/* Private handles */
static Stepper_Handle_t stepper1, stepper2, stepper3;

/* ================================================================
 * Pin Definitions (matching your working hardware config)
 * ================================================================ */

/* Motor 1 */
#define M1_CWP_Port   GPIOB
#define M1_CWP_Pin    GPIO_PIN_15
#define M1_CWN_Port   GPIOB
#define M1_CWN_Pin    GPIO_PIN_14
#define M1_ENP_Port   GPIOB
#define M1_ENP_Pin    GPIO_PIN_13
#define M1_ENN_Port   GPIOB
#define M1_ENN_Pin    GPIO_PIN_12
#define M1_CLKN_Port  GPIOA
#define M1_CLKN_Pin   GPIO_PIN_1

/* Motor 2 */
#define M2_CWP_Port   GPIOB
#define M2_CWP_Pin    GPIO_PIN_6
#define M2_CWN_Port   GPIOB
#define M2_CWN_Pin    GPIO_PIN_7
#define M2_ENP_Port   GPIOB
#define M2_ENP_Pin    GPIO_PIN_8
#define M2_ENN_Port   GPIOC
#define M2_ENN_Pin    GPIO_PIN_14
#define M2_CLKN_Port  GPIOB
#define M2_CLKN_Pin   GPIO_PIN_5

/* Motor 3 */
#define M3_CWP_Port   GPIOB
#define M3_CWP_Pin    GPIO_PIN_0  /* PB0 = CW+ */
#define M3_CWN_Port   GPIOB
#define M3_CWN_Pin    GPIO_PIN_1
#define M3_ENP_Port   GPIOB
#define M3_ENP_Pin    GPIO_PIN_2
#define M3_ENN_Port   GPIOB
#define M3_ENN_Pin    GPIO_PIN_10
#define M3_CLKN_Port  GPIOA
#define M3_CLKN_Pin   GPIO_PIN_6

/* ================================================================
 * GPIO Helpers
 * ================================================================ */

void Stepper_SetDirection(Motor_ID M, uint8_t dir)
{
    GPIO_PinState pin_state = (dir == STEPPER_CW) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    switch (M) {
        case LegMotor_1: HAL_GPIO_WritePin(M1_CWP_Port, M1_CWP_Pin, pin_state); break;
        case LegMotor_2: HAL_GPIO_WritePin(M2_CWP_Port, M2_CWP_Pin, pin_state); break;
        case LegMotor_3: HAL_GPIO_WritePin(M3_CWP_Port, M3_CWP_Pin, pin_state); break;
        default: break;
    }
}

static void Stepper_SetEnable(Motor_ID M, uint8_t enable)
{
    /* TB6560 active LOW enable: RESET = enabled, SET = disabled */
    GPIO_PinState pin_state = enable ? GPIO_PIN_RESET : GPIO_PIN_SET;
    switch (M) {
        case LegMotor_1: HAL_GPIO_WritePin(M1_ENP_Port, M1_ENP_Pin, pin_state); break;
        case LegMotor_2: HAL_GPIO_WritePin(M2_ENP_Port, M2_ENP_Pin, pin_state); break;
        case LegMotor_3: HAL_GPIO_WritePin(M3_ENP_Port, M3_ENP_Pin, pin_state); break;
        default: break;
    }
}

/* ================================================================
 * Public API
 * ================================================================ */

void Stepper_Init(Motor_ID M)
{
    Stepper_Handle_t *h;
    switch (M) {
        case LegMotor_1: h = &stepper1; break;
        case LegMotor_2: h = &stepper2; break;
        case LegMotor_3: h = &stepper3; break;
        default: return;
    }

    h->target_steps  = 0;
    h->current_steps = 0;
    h->state         = STEPPER_IDLE;
    h->direction     = STEPPER_CW;

    /* Set CLK-, CW-, EN- all LOW (your proven working init) */
    switch (M) {
        case LegMotor_1:
            HAL_GPIO_WritePin(M1_CLKN_Port, M1_CLKN_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M1_CWN_Port,  M1_CWN_Pin,  GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M1_ENN_Port,  M1_ENN_Pin,  GPIO_PIN_RESET);
            break;
        case LegMotor_2:
            HAL_GPIO_WritePin(M2_CLKN_Port, M2_CLKN_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_CWN_Port,  M2_CWN_Pin,  GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_ENN_Port,  M2_ENN_Pin,  GPIO_PIN_RESET);
            break;
        case LegMotor_3:
            HAL_GPIO_WritePin(M3_CLKN_Port, M3_CLKN_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M3_CWN_Port,  M3_CWN_Pin,  GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M3_ENN_Port,  M3_ENN_Pin,  GPIO_PIN_RESET);
            break;
        default: break;
    }

    Stepper_Disable(M);
}

void Stepper_Run(Motor_ID M, uint8_t dir)
{
    Stepper_Handle_t *h;
    TIM_HandleTypeDef *tim;
    uint32_t ch;

    switch (M) {
        case LegMotor_1: h = &stepper1; tim = &htim2; ch = TIM_CHANNEL_1; break;
        case LegMotor_2: h = &stepper2; tim = &htim4; ch = TIM_CHANNEL_4; break;
        default: return;
    }

    if (h->state == STEPPER_RUNNING) Stepper_Stop(M);

    h->direction     = dir;
    h->target_steps  = 0;   /* 0 = free run */
    h->current_steps = 0;

    Stepper_SetDirection(M, dir);
    Stepper_SetEnable(M, 1);
    h->state = STEPPER_RUNNING;

    HAL_TIM_PWM_Start(tim, ch);
    __HAL_TIM_ENABLE_IT(tim, TIM_IT_UPDATE);
}

void Stepper_Move(Motor_ID M, int32_t steps)
{
    if (steps == 0) return;

    Stepper_Handle_t *h;
    TIM_HandleTypeDef *tim;
    uint32_t ch;

    switch (M) {
        case LegMotor_1: h = &stepper1; tim = &htim2; ch = TIM_CHANNEL_1; break;
        case LegMotor_2: h = &stepper2; tim = &htim4; ch = TIM_CHANNEL_4; break;
        default: return;
    }

    if (h->state == STEPPER_RUNNING) return;

    uint8_t  dir       = (steps > 0) ? STEPPER_CW : STEPPER_CCW;
    uint32_t abs_steps = (steps > 0) ? (uint32_t)steps : (uint32_t)(-steps);

    h->direction     = dir;
    h->target_steps  = abs_steps;
    h->current_steps = 0;
    h->state         = STEPPER_RUNNING;

    Stepper_SetDirection(M, dir);
    Stepper_SetEnable(M, 1);

    HAL_Delay(100);

    HAL_TIM_PWM_Start(tim, ch);
    __HAL_TIM_ENABLE_IT(tim, TIM_IT_UPDATE);
}

uint32_t Stepper_Stop(Motor_ID M)
{
    Stepper_Handle_t *h;
    TIM_HandleTypeDef *tim;
    uint32_t ch;

    switch (M) {
        case LegMotor_1: h = &stepper1; tim = &htim2; ch = TIM_CHANNEL_1; break;
        case LegMotor_2: h = &stepper2; tim = &htim4; ch = TIM_CHANNEL_4; break;
        default: return 0;
    }

    HAL_TIM_PWM_Stop(tim, ch);
    __HAL_TIM_DISABLE_IT(tim, TIM_IT_UPDATE);

    uint32_t steps_taken = h->current_steps;
    h->current_steps = 0;
    h->target_steps  = 0;
    h->state         = STEPPER_IDLE;

    return steps_taken;
}

void Stepper_Enable(Motor_ID M)  { Stepper_SetEnable(M, 1); }
void Stepper_Disable(Motor_ID M) { Stepper_SetEnable(M, 0); }

uint8_t Stepper_IsBusy(Motor_ID M)
{
    switch (M) {
        case LegMotor_1: return (stepper1.state == STEPPER_RUNNING) ? 1 : 0;
        case LegMotor_2: return (stepper2.state == STEPPER_RUNNING) ? 1 : 0;
        case LegMotor_3: return (stepper3.state == STEPPER_RUNNING) ? 1 : 0;
        default: return 0;
    }
}

/* ================================================================
 * IRQ Handler — called from HAL_TIM_PeriodElapsedCallback
 * ================================================================ */

void Stepper_IRQ_Handler(Motor_ID M)
{
    Stepper_Handle_t *h;

    switch (M) {
        case LegMotor_1: h = &stepper1; break;
        case LegMotor_2: h = &stepper2; break;
        case LegMotor_3: h = &stepper3; break;
        default: return;
    }

    if (h->state != STEPPER_RUNNING) return;

    h->current_steps++;

    /* If target_steps > 0: precise move, auto-stop at target */
    if (h->target_steps > 0 && h->current_steps >= h->target_steps) {
        Stepper_Stop(M);
        /* board1.c polls Stepper_IsBusy and sends DONE response */
    }
}
