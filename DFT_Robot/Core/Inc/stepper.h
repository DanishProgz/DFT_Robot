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

/* Public functions */
void Stepper_Init(void);
void Stepper_Move(int32_t steps);
void Stepper_Stop(void);
void Stepper_Enable(void);
void Stepper_Disable(void);
uint8_t Stepper_IsBusy(void);

/* Call this from TIM2 Update IRQ handler */
void Stepper_IRQ_Handler(void);

#endif /* STEPPER_H */
