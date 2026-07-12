/**
 * @file    encoder.h
 * @brief   Rotary encoder driver for drum position sensing
 *
 * @details Quadrature encoder on PA5 (CLK, EXTI rising edge) and PA6 (DT, input).
 *          EXTI5 ISR reads DT to determine direction and increments/decrements
 *          a tick counter. Degree conversion uses a configurable PPR value.
 *
 *          CubeMX must configure:
 *            - PA5: GPIO_EXTI5, rising edge, pull-up
 *            - PA6: GPIO_Input, pull-up
 *            - NVIC: EXTI9_5_IRQn enabled
 */

#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @brief  Pulses per revolution of the encoder.
 *         Adjust this after calibration (rotate drum 360°, read tick count).
 */
#define ENCODER_PPR     600

/**
 * @brief  Initialize encoder GPIO (PA5 EXTI, PA6 input).
 *         Call after HAL_Init() and clock enables.
 */
void encoder_init(void);

/**
 * @brief  Get current position in degrees (0–359).
 * @return Encoder angle in degrees, wrapped to 0–359.
 */
int32_t encoder_get_degrees(void);

/**
 * @brief  Get raw tick count (signed, not wrapped).
 * @return Raw encoder ticks since last reset.
 */
int32_t encoder_get_ticks(void);

/**
 * @brief  Reset encoder position to zero.
 */
void encoder_reset(void);

/**
 * @brief  EXTI callback — call from HAL_GPIO_EXTI_Callback when pin is GPIO_PIN_5.
 */
void encoder_exti_handler(void);

#endif /* ENCODER_H */
