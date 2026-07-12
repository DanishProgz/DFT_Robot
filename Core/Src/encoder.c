/**
 * @file    encoder.c
 * @brief   Rotary encoder driver implementation
 *
 * @details Quadrature decoding via EXTI on PA5 (CLK).
 *          On rising edge of CLK: read PA6 (DT).
 *            DT LOW  → CW  → ticks++
 *            DT HIGH → CCW → ticks--
 *
 *          Software debounce: ignore edges within 1ms of the last.
 */

#include "encoder.h"
#include "pins.h"

/* ================================================================
 * Private State
 * ================================================================ */

static volatile int32_t enc_ticks = 0;
static volatile uint32_t last_edge_tick = 0;

#define DEBOUNCE_MS  1

/* ================================================================
 * Initialization
 *
 * Configures PA5 as EXTI rising edge (encoder CLK)
 * and PA6 as floating input (encoder DT).
 * CubeMX should also configure these, but this ensures
 * correct config if CubeMX project is cloned from Board 1.
 * ================================================================ */

void encoder_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA6 — DT input with pull-up */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = ENC_DT_PIN;
    gpio.Mode  = GPIO_MODE_INPUT;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ENC_DT_PORT, &gpio);

    /* PA5 — CLK with EXTI rising edge, pull-up */
    gpio.Pin   = ENC_CLK_PIN;
    gpio.Mode  = GPIO_MODE_IT_RISING;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ENC_CLK_PORT, &gpio);

    /* Enable EXTI9_5 interrupt */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    enc_ticks = 0;
}

/* ================================================================
 * EXTI Handler — called from HAL_GPIO_EXTI_Callback
 * ================================================================ */

void encoder_exti_handler(void)
{
    uint32_t now = HAL_GetTick();
    if ((now - last_edge_tick) < DEBOUNCE_MS) return;
    last_edge_tick = now;

    /* Read DT to determine direction */
    if (HAL_GPIO_ReadPin(ENC_DT_PORT, ENC_DT_PIN) == GPIO_PIN_RESET) {
        enc_ticks++;   /* CW */
    } else {
        enc_ticks--;   /* CCW */
    }
}

/* ================================================================
 * Public Getters
 * ================================================================ */

int32_t encoder_get_degrees(void)
{
    int32_t ticks = enc_ticks;
    /* Map ticks to degrees, handle negative wrap */
    int32_t deg = (ticks * 360) / ENCODER_PPR;
    deg = deg % 360;
    if (deg < 0) deg += 360;
    return deg;
}

int32_t encoder_get_ticks(void)
{
    return enc_ticks;
}

void encoder_reset(void)
{
    enc_ticks = 0;
}
