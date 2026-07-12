/**
 * @file    board2.h
 * @brief   Board 2 — Legs + Drum Controller (addr 02)
 */

#ifndef BOARD2_H
#define BOARD2_H

/**
 * @brief  Initialize Board 2 subsystems (steppers, RS-485, encoder, limits).
 *         Call from main() after CubeMX init.
 */
void board2_init(void);

/**
 * @brief  Main loop tick. Call from while(1) in main().
 */
void board2_loop(void);

#endif /* BOARD2_H */
