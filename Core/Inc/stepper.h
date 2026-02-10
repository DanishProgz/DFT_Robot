/*
 * stepper.h
 * TB6560 Stepper Motor Driver for STM32F401CDU6
 *
 * Pin Mapping:
 *   CLK+  -> PA0  (TIM2_CH1 PWM output)
 *   CLK-  -> PA1  (GPIO LOW)
 *   CW+   -> PB15 (GPIO direction)
 *   CW-   -> PB14 (GPIO LOW)
 *   EN+   -> PB13 (GPIO enable)
 *   EN-   -> PB12 (GPIO LOW)
 *
 * Steps per revolution: 3200
 */

#ifndef STEPPER_H
#define STEPPER_H

#include "main.h"

/* TIM2 handle - defined in tim.c by CubeMX */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
/* Configuration */
#define STEPPER_STEPS_PER_REV   3200

/* Direction */
#define STEPPER_CW              0
#define STEPPER_CCW             1

/* Motor state */
typedef enum {
    STEPPER_IDLE = 0,
    STEPPER_RUNNING
} Stepper_State_t;

/* Stepper handle */
typedef struct {
    volatile uint32_t   target_steps;
    volatile uint32_t   current_steps;
    volatile Stepper_State_t state;
    uint8_t             direction;
} Stepper_Handle_t;


typedef enum {
	LegMotor_1,
	LegMotor_2,
	LegMotor_3,
}Motor_ID;
/* Public functions */

void Stepper_Init(Motor_ID M);
void Stepper_Move(Motor_ID M, int32_t steps);
void Stepper_Stop(Motor_ID M);
uint8_t Stepper_IsBusy(Motor_ID M);
void Stepper_Enable(Motor_ID M);
void Stepper_Disable(Motor_ID M);


/* Call this from TIM2 Update IRQ handler */
void Stepper_IRQ_Handler(Motor_ID M);

#endif /* STEPPER_H */
