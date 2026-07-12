/**
 * @file    pins.h
 * @brief   Pin definitions for all DFT Robot boards
 *
 * @details Timer-based step pulse generation:
 *            Motor 1 CLK+ (PA0) = TIM2_CH1, AF1
 *            Motor 2 CLK+ (PB9) = TIM4_CH4, AF2
 *            Motor 3 CLK+ (PA7) = TIM3_CH2, AF2  [Board 1 only]
 *
 *          CLK- pins are GPIO held LOW permanently (TB6560
 *          differential ground reference — no complement needed).
 *
 *          Pin allocation:
 *            PA0  - Motor 1 CLK+ (TIM2_CH1)
 *            PA1  - Motor 1 CLK- (GPIO LOW)
 *            PA2  - USART2 TX (RS-485)
 *            PA3  - USART2 RX (RS-485)
 *            PA4  - RS-485 DE/RE
 *            PA5  - Board 2: Encoder CLK  |  Board 3: Brush limit
 *            PA6  - Board 2: Encoder DT   |  Board 3: Probe limit
 *            PA7  - Motor 3 CLK+ (TIM3_CH2, Board 1 only)
 *            PA8  - Board 3: Lights relay
 *            PB0  - Motor 3 CW+
 *            PB1  - Motor 3 CW-
 *            PB2  - Motor 3 EN+
 *            PB5  - Motor 2 CLK- (GPIO LOW)
 *            PB6  - Motor 2 CW+
 *            PB7  - Motor 2 CW-
 *            PB8  - Motor 2 EN+
 *            PB9  - Motor 2 CLK+ (TIM4_CH4)
 *            PB10 - Motor 3 EN-
 *            PB12 - Motor 1 EN-
 *            PB13 - Motor 1 EN+
 *            PB14 - Motor 1 CW-
 *            PB15 - Motor 1 CW+
 *            PC13 - On-board LED
 *            PC14 - Motor 2 EN-
 */

#ifndef PINS_H
#define PINS_H

#include "stm32f4xx_hal.h"

/* ================================================================
 * RS-485 Interface
 * ================================================================ */
#define RS485_USART         USART2
#define RS485_BAUD          115200
#define RS485_TX_PORT       GPIOA
#define RS485_TX_PIN        GPIO_PIN_2
#define RS485_RX_PORT       GPIOA
#define RS485_RX_PIN        GPIO_PIN_3
#define RS485_DERE_PORT     GPIOA
#define RS485_DERE_PIN      GPIO_PIN_4

/* ================================================================
 * On-board LED
 * ================================================================ */
#define LED_PORT            GPIOC
#define LED_PIN             GPIO_PIN_13

/* ================================================================
 * Motor 1 (all boards)
 * CLK+ = PA0, TIM2_CH1, AF1 (configured by CubeMX)
 * CLK- = PA1, GPIO held LOW
 * ================================================================ */
#define M1_CLK_N_PORT   GPIOA
#define M1_CLK_N_PIN    GPIO_PIN_1

#define M1_EN_P_PORT    GPIOB
#define M1_EN_P_PIN     GPIO_PIN_13
#define M1_EN_N_PORT    GPIOB
#define M1_EN_N_PIN     GPIO_PIN_12

#define M1_CW_P_PORT    GPIOB
#define M1_CW_P_PIN     GPIO_PIN_15
#define M1_CW_N_PORT    GPIOB
#define M1_CW_N_PIN     GPIO_PIN_14

/* ================================================================
 * Motor 2 (all boards)
 * CLK+ = PB9, TIM4_CH4, AF2 (configured by CubeMX)
 * CLK- = PB5, GPIO held LOW
 * ================================================================ */
#define M2_CLK_N_PORT   GPIOB
#define M2_CLK_N_PIN    GPIO_PIN_5

#define M2_EN_P_PORT    GPIOB
#define M2_EN_P_PIN     GPIO_PIN_8
#define M2_EN_N_PORT    GPIOC
#define M2_EN_N_PIN     GPIO_PIN_14

#define M2_CW_P_PORT    GPIOB
#define M2_CW_P_PIN     GPIO_PIN_6
#define M2_CW_N_PORT    GPIOB
#define M2_CW_N_PIN     GPIO_PIN_7

/* ================================================================
 * Motor 3 (Board 1 only)
 * CLK+ = PA7, TIM3_CH2, AF2 (configured by CubeMX)
 * CLK- = PA6, GPIO held LOW
 * ================================================================ */
#define M3_CLK_N_PORT   GPIOA
#define M3_CLK_N_PIN    GPIO_PIN_6

#define M3_EN_P_PORT    GPIOB
#define M3_EN_P_PIN     GPIO_PIN_2
#define M3_EN_N_PORT    GPIOB
#define M3_EN_N_PIN     GPIO_PIN_10

#define M3_CW_P_PORT    GPIOB
#define M3_CW_P_PIN     GPIO_PIN_0
#define M3_CW_N_PORT    GPIOB
#define M3_CW_N_PIN     GPIO_PIN_1

/* ================================================================
 * Board 2 - Rotary Encoder (Drum)
 * CLK = PA5 (EXTI5 rising edge), DT = PA6 (input)
 * ================================================================ */
#define ENC_CLK_PORT        GPIOA
#define ENC_CLK_PIN         GPIO_PIN_5
#define ENC_DT_PORT         GPIOA
#define ENC_DT_PIN          GPIO_PIN_6

/* ================================================================
 * Board 3 - Limit Switches (NC, pull-up, falling edge EXTI)
 * Brush = PA5, Probe = PA6
 * ================================================================ */
#define BRUSH_LIM_PORT      GPIOA
#define BRUSH_LIM_PIN       GPIO_PIN_5
#define PROBE_LIM_PORT      GPIOA
#define PROBE_LIM_PIN       GPIO_PIN_6

/* ================================================================
 * Board 3 - Lights Relay (PA8, HIGH = on)
 * ================================================================ */
#define LIGHTS_PORT         GPIOA
#define LIGHTS_PIN          GPIO_PIN_8

#endif /* PINS_H */
