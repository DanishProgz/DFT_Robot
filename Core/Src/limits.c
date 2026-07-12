/**
 * @file    limits.c
 * @brief   Drum limit switch driver implementation
 *
 * @details NC switch on PA8. When armed (during homing), falling edge
 *          triggers immediate drum motor stop via Stepper_Stop().
 *
 *          The switch is normally closed — pin reads HIGH when not pressed.
 *          Falling edge = switch opened = actuator hit the switch body.
 *
 *          PA8 uses EXTI line 8, which shares EXTI9_5_IRQn with the
 *          encoder on PA5. HAL_GPIO_EXTI_Callback dispatches by pin.
 */

#include "limits.h"
#include "stepper.h"

/* ================================================================
 * Private State
 * ================================================================ */

static volatile bool armed     = false;
static volatile bool triggered = false;

/* ================================================================
 * Initialization
 * ================================================================ */

void limits_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = DRUM_LIM_PIN;
    gpio.Mode  = GPIO_MODE_IT_FALLING;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DRUM_LIM_PORT, &gpio);

    /* EXTI9_5 may already be enabled by encoder_init — safe to call again */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);  /* Higher priority than encoder */
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    armed     = false;
    triggered = false;
}

/* ================================================================
 * Arm / Disarm
 * ================================================================ */

void limits_arm(void)
{
    triggered = false;
    armed     = true;
}

void limits_disarm(void)
{
    armed = false;
}

bool limits_triggered(void)
{
    return triggered;
}

void limits_clear(void)
{
    triggered = false;
}

bool limits_read_raw(void)
{
    /* NC switch: LOW = pressed/triggered */
    return (HAL_GPIO_ReadPin(DRUM_LIM_PORT, DRUM_LIM_PIN) == GPIO_PIN_RESET);
}

/* ================================================================
 * EXTI Handler — called from HAL_GPIO_EXTI_Callback
 * ================================================================ */

void limits_exti_handler(void)
{
    if (armed) {
        /* Immediate hard stop of drum motor */
        Stepper_Stop(LegMotor_2);
        triggered = true;
        armed     = false;
    }
}
