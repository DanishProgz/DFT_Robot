/**
 * @file    system.h
 * @brief   System utilities — DWT timer, LED
 *
 * @details Clock configuration is handled by CubeMX-generated
 *          SystemClock_Config(). This file provides supplementary
 *          utilities shared across all boards.
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @brief  Enable DWT cycle counter for microsecond-precision delays.
 *         Call after HAL_Init().
 */
void system_dwt_init(void);

/**
 * @brief  Blocking microsecond delay using DWT cycle counter.
 */
void delay_us(uint32_t us);

/** @brief Initialize PC13 LED as output. */
void system_led_init(void);

/** @brief Toggle PC13 LED. */
void system_led_toggle(void);

#endif /* SYSTEM_H */
