/*
 * stepper.h
 * TB6560 Stepper Motor Driver for STM32F401CDU6
 */

#ifndef STEPPER_H
#define STEPPER_H

#include "stm32f4xx_hal.h"

/* ---- Direction ---- */
#define STEPPER_CW   1
#define STEPPER_CCW  0

/* ---- State ---- */
typedef enum {
    STEPPER_IDLE    = 0,
    STEPPER_RUNNING = 1,
} Stepper_State_t;

/* ---- Motor IDs ---- */
typedef enum {
    LegMotor_1 = 0,
    LegMotor_2 = 1,
    LegMotor_3 = 2,
} Motor_ID;

/* ---- Handle ---- */
typedef struct {
    uint32_t        target_steps;   /* 0 = free run */
    uint32_t        current_steps;
    Stepper_State_t state;
    uint8_t         direction;
} Stepper_Handle_t;

/* ---- Public API ---- */
void     Stepper_Init(Motor_ID M);
void     Stepper_Run(Motor_ID M, uint8_t dir);
void     Stepper_Move(Motor_ID M, int32_t steps);
uint32_t Stepper_Stop(Motor_ID M);
void     Stepper_Enable(Motor_ID M);
void     Stepper_Disable(Motor_ID M);
uint8_t  Stepper_IsBusy(Motor_ID M);
void     Stepper_IRQ_Handler(Motor_ID M);

#endif /* STEPPER_H */
