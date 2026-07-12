/**
 * @file    limits.h
 * @brief   Drum limit switch driver for homing
 *
 * @details NC limit switch on PA8, EXTI falling edge.
 *          When triggered during homing, immediately stops drum motor.
 *
 *          CubeMX must configure:
 *            - PA8: GPIO_EXTI8, falling edge, pull-up (NC switch)
 *            - NVIC: EXTI9_5_IRQn enabled (shared with encoder on PA5)
 */

#ifndef LIMITS_H
#define LIMITS_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Limit switch pin — PA8 */
#define DRUM_LIM_PORT   GPIOA
#define DRUM_LIM_PIN    GPIO_PIN_8

/**
 * @brief  Initialize limit switch GPIO (PA8 EXTI falling edge, pull-up).
 */
void limits_init(void);

/**
 * @brief  Arm the limit switch for homing. While armed, the EXTI callback
 *         will immediately stop the drum motor and set the triggered flag.
 */
void limits_arm(void);

/**
 * @brief  Disarm the limit switch (normal operation — switch ignored).
 */
void limits_disarm(void);

/**
 * @brief  Check if limit switch was triggered since last arm.
 * @return true if triggered.
 */
bool limits_triggered(void);

/**
 * @brief  Clear the triggered flag.
 */
void limits_clear(void);

/**
 * @brief  Read current switch state (for diagnostics).
 * @return true if switch is currently pressed (pin LOW for NC switch).
 */
bool limits_read_raw(void);

/**
 * @brief  EXTI callback — call from HAL_GPIO_EXTI_Callback when pin is GPIO_PIN_8.
 */
void limits_exti_handler(void);

#endif /* LIMITS_H */
