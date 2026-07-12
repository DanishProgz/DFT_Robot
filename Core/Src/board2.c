/**
 * @file    board2.c
 * @brief   Board 2 — Legs + Drum Controller (addr 02)
 *
 * @details Two motors:
 *            Motor 1 (LegMotor_1) — Leg height adjustment
 *            Motor 2 (LegMotor_2) — Drum rotation for DFT inspection
 *
 *          Rotary encoder on drum (PA5 CLK, PA6 DT) for angle readback.
 *          Limit switch on drum (PA8) for homing reference.
 *
 *          State machine tracks leg and drum independently:
 *            - Legs: IDLE / MOVING / RUNNING
 *            - Drum: IDLE / MOVING / RUNNING / HOMING
 *
 * Commands:
 *   @02:M1_CW:steps     Legs CW N steps          → DONE
 *   @02:M1_CCW:steps    Legs CCW N steps          → DONE
 *   @02:M1_RUN_CW       Legs continuous CW        → ACK
 *   @02:M1_RUN_CCW      Legs continuous CCW       → ACK
 *   @02:M2_CW:steps     Drum CW N steps           → DONE
 *   @02:M2_CCW:steps    Drum CCW N steps           → DONE
 *   @02:M2_RUN_CW       Drum continuous CW        → ACK
 *   @02:M2_RUN_CCW      Drum continuous CCW       → ACK
 *   @02:HOME            Home drum to limit switch  → HOMED
 *   @02:ENC             Read encoder degrees       → degrees
 *   @02:ENC_RST         Reset encoder to zero      → ACK
 *   @02:STOP            Stop all motors            → ACK
 *   @02:PING            Health check               → ACK
 */

#include "board2.h"
#include "main.h"
#include "system.h"
#include "rs485.h"
#include "stepper.h"
#include "encoder.h"
#include "limits.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BOARD_ID    2

/* Homing timeout — if limit switch not hit within this time, abort */
#define HOME_TIMEOUT_MS  15000

/* ================================================================
 * Independent State Machines for Legs and Drum
 * ================================================================ */

typedef enum {
    MSTATE_IDLE,
    MSTATE_MOVING,      /* Precise N-step move in progress */
    MSTATE_RUNNING,     /* Free run until STOP */
    MSTATE_HOMING       /* Drum only: running CCW to limit switch */
} MotorState_t;

static MotorState_t legs_state = MSTATE_IDLE;
static MotorState_t drum_state = MSTATE_IDLE;

static char legs_active_cmd[16] = "";
static char drum_active_cmd[16] = "";

static uint32_t home_start_tick = 0;

/* ================================================================
 * Command Handler
 * ================================================================ */

static void handle_command(RS485_Frame_t *f)
{
    /* ---- STOP: always accepted, stops everything ---- */
    if (strcmp(f->cmd, "STOP") == 0) {
        Stepper_Stop(LegMotor_1);
        Stepper_Stop(LegMotor_2);
        limits_disarm();
        legs_state = MSTATE_IDLE;
        drum_state = MSTATE_IDLE;
        rs485_send_ack("STOP", "");
        return;
    }

    /* ---- PING ---- */
    if (strcmp(f->cmd, "PING") == 0) {
        rs485_send_ack("PING", "");
        return;
    }

    /* ---- ENC: read encoder (always available) ---- */
    if (strcmp(f->cmd, "ENC") == 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%ld", (long)encoder_get_degrees());
        rs485_send_ack("ENC", buf);
        return;
    }

    /* ---- ENC_RST: reset encoder (always available) ---- */
    if (strcmp(f->cmd, "ENC_RST") == 0) {
        encoder_reset();
        rs485_send_ack("ENC_RST", "");
        return;
    }

    /* ---- HOME: drum homing sequence ---- */
    if (strcmp(f->cmd, "HOME") == 0) {
        if (drum_state != MSTATE_IDLE) {
            rs485_send_err("HOME", "DRUM_BUSY");
            return;
        }
        /* Arm limit switch, start drum CCW */
        limits_clear();
        limits_arm();
        Stepper_Run(LegMotor_2, STEPPER_CCW);
        drum_state = MSTATE_HOMING;
        strncpy(drum_active_cmd, "HOME", sizeof(drum_active_cmd) - 1);
        home_start_tick = HAL_GetTick();
        /* Response sent when limit triggers or timeout — see board2_loop */
        return;
    }

    /* ================================================================
     * Motor 1 (Legs) Commands
     * ================================================================ */

    if (strcmp(f->cmd, "M1_CW") == 0) {
        if (legs_state != MSTATE_IDLE) { rs485_send_err("M1_CW", "BUSY"); return; }
        int32_t steps = atoi(f->param);
        if (steps <= 0) { rs485_send_err("M1_CW", "BAD_PARAM"); return; }
        strncpy(legs_active_cmd, "M1_CW", sizeof(legs_active_cmd) - 1);
        legs_state = MSTATE_MOVING;
        Stepper_Move(LegMotor_1, steps);
        return;
    }

    if (strcmp(f->cmd, "M1_CCW") == 0) {
        if (legs_state != MSTATE_IDLE) { rs485_send_err("M1_CCW", "BUSY"); return; }
        int32_t steps = atoi(f->param);
        if (steps <= 0) { rs485_send_err("M1_CCW", "BAD_PARAM"); return; }
        strncpy(legs_active_cmd, "M1_CCW", sizeof(legs_active_cmd) - 1);
        legs_state = MSTATE_MOVING;
        Stepper_Move(LegMotor_1, -steps);
        return;
    }

    if (strcmp(f->cmd, "M1_RUN_CW") == 0) {
        if (legs_state != MSTATE_IDLE) { rs485_send_err("M1_RUN_CW", "BUSY"); return; }
        strncpy(legs_active_cmd, "M1_RUN_CW", sizeof(legs_active_cmd) - 1);
        legs_state = MSTATE_RUNNING;
        Stepper_Run(LegMotor_1, STEPPER_CW);
        rs485_send_ack("M1_RUN_CW", "");
        return;
    }

    if (strcmp(f->cmd, "M1_RUN_CCW") == 0) {
        if (legs_state != MSTATE_IDLE) { rs485_send_err("M1_RUN_CCW", "BUSY"); return; }
        strncpy(legs_active_cmd, "M1_RUN_CCW", sizeof(legs_active_cmd) - 1);
        legs_state = MSTATE_RUNNING;
        Stepper_Run(LegMotor_1, STEPPER_CCW);
        rs485_send_ack("M1_RUN_CCW", "");
        return;
    }

    /* ================================================================
     * Motor 2 (Drum) Commands
     * ================================================================ */

    if (strcmp(f->cmd, "M2_CW") == 0) {
        if (drum_state != MSTATE_IDLE) { rs485_send_err("M2_CW", "BUSY"); return; }
        int32_t steps = atoi(f->param);
        if (steps <= 0) { rs485_send_err("M2_CW", "BAD_PARAM"); return; }
        strncpy(drum_active_cmd, "M2_CW", sizeof(drum_active_cmd) - 1);
        drum_state = MSTATE_MOVING;
        Stepper_Move(LegMotor_2, steps);
        return;
    }

    if (strcmp(f->cmd, "M2_CCW") == 0) {
        if (drum_state != MSTATE_IDLE) { rs485_send_err("M2_CCW", "BUSY"); return; }
        int32_t steps = atoi(f->param);
        if (steps <= 0) { rs485_send_err("M2_CCW", "BAD_PARAM"); return; }
        strncpy(drum_active_cmd, "M2_CCW", sizeof(drum_active_cmd) - 1);
        drum_state = MSTATE_MOVING;
        Stepper_Move(LegMotor_2, -steps);
        return;
    }

    if (strcmp(f->cmd, "M2_RUN_CW") == 0) {
        if (drum_state != MSTATE_IDLE) { rs485_send_err("M2_RUN_CW", "BUSY"); return; }
        strncpy(drum_active_cmd, "M2_RUN_CW", sizeof(drum_active_cmd) - 1);
        drum_state = MSTATE_RUNNING;
        Stepper_Run(LegMotor_2, STEPPER_CW);
        rs485_send_ack("M2_RUN_CW", "");
        return;
    }

    if (strcmp(f->cmd, "M2_RUN_CCW") == 0) {
        if (drum_state != MSTATE_IDLE) { rs485_send_err("M2_RUN_CCW", "BUSY"); return; }
        strncpy(drum_active_cmd, "M2_RUN_CCW", sizeof(drum_active_cmd) - 1);
        drum_state = MSTATE_RUNNING;
        Stepper_Run(LegMotor_2, STEPPER_CCW);
        rs485_send_ack("M2_RUN_CCW", "");
        return;
    }

    /* ---- Unknown command ---- */
    rs485_send_err(f->cmd, "UNKNOWN_CMD");
}

/* ================================================================
 * Public API
 * ================================================================ */

void board2_init(void)
{
    system_led_init();
    system_dwt_init();

    /* Init Motor 1 (Legs) and Motor 2 (Drum) only — no Motor 3 */
    Stepper_Init(LegMotor_1);
    Stepper_Init(LegMotor_2);

    encoder_init();
    limits_init();

    rs485_init(BOARD_ID);

    HAL_Delay(100);
    rs485_send_ack("BOOT", "");
}

void board2_loop(void)
{
    /* ---- Process RS-485 commands ---- */
    RS485_Frame_t f = rs485_poll();
    if (f.valid) {
        system_led_toggle();
        handle_command(&f);
    }

    /* ---- Check legs move completion ---- */
    if (legs_state == MSTATE_MOVING && !Stepper_IsBusy(LegMotor_1)) {
        rs485_send_ack(legs_active_cmd, "DONE");
        legs_state = MSTATE_IDLE;
    }

    /* ---- Check drum move completion ---- */
    if (drum_state == MSTATE_MOVING && !Stepper_IsBusy(LegMotor_2)) {
        rs485_send_ack(drum_active_cmd, "DONE");
        drum_state = MSTATE_IDLE;
    }

    /* ---- Check homing completion ---- */
    if (drum_state == MSTATE_HOMING) {
        if (limits_triggered()) {
            /* Limit switch stopped the motor (done in ISR) */
            encoder_reset();
            limits_disarm();
            drum_state = MSTATE_IDLE;
            rs485_send_ack("HOME", "HOMED");
        }
        else if ((HAL_GetTick() - home_start_tick) > HOME_TIMEOUT_MS) {
            /* Timeout — limit switch never hit */
            Stepper_Stop(LegMotor_2);
            limits_disarm();
            drum_state = MSTATE_IDLE;
            rs485_send_err("HOME", "TIMEOUT");
        }
    }

    /* ---- Heartbeat LED (1 Hz) ---- */
    static uint32_t last_hb = 0;
    if (HAL_GetTick() - last_hb > 1000) {
        system_led_toggle();
        last_hb = HAL_GetTick();
    }
}
