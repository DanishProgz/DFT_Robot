#!/usr/bin/env python3
"""
Stepper Motor Controller
========================
Controls a stepper motor via serial UART using curses (no pynput needed).

Usage:
    python3 stepper_control.py [port]

    Default port: /dev/ttyUSB0

Controls:
    UP arrow    -> Send UP   to STM32
    DOWN arrow  -> Send DOWN to STM32
    Q / ESC     -> Quit

Requirements:
    pip install pyserial
"""

import sys
import serial
import curses
import threading

# --- Configuration ---
DEFAULT_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200

# Commands matching STM32 firmware
CMD_FORWARD = b'UP\r\n'
CMD_REVERSE = b'DOWN\r\n'
CMD_STOP    = b'STOP\r\n'

ser = None
running = True
last_response = ""


def connect(port):
    try:
        s = serial.Serial(
            port=port,
            baudrate=BAUD_RATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1
        )
        return s
    except serial.SerialException as e:
        print(f"Error: Could not open {port}: {e}")
        return None


def send_command(cmd):
    global ser
    if ser and ser.is_open:
        try:
            ser.write(cmd)
            ser.flush()
        except serial.SerialException:
            pass


def response_reader():
    global last_response, running
    while running:
        try:
            if ser and ser.is_open and ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='replace').strip()
                if line:
                    last_response = line
        except Exception:
            pass


def main(stdscr):
    global ser, running, last_response

    port = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_PORT

    # Setup curses
    curses.cbreak()
    stdscr.keypad(True)
    stdscr.nodelay(True)
    curses.noecho()
    curses.curs_set(0)

    # Connect
    ser = connect(port)
    if not ser:
        stdscr.addstr(0, 0, f"Failed to open {port}. Press any key to exit.")
        stdscr.nodelay(False)
        stdscr.getch()
        return

    # Send initial stop
    send_command(CMD_STOP)

    # Start response reader thread
    t = threading.Thread(target=response_reader, daemon=True)
    t.start()

    current_state = "STOPPED"
    prev_key_up   = False
    prev_key_down = False

    while running:
        stdscr.clear()
        stdscr.addstr(0, 0, "========================================")
        stdscr.addstr(1, 0, "  Stepper Motor Controller")
        stdscr.addstr(2, 0, "========================================")
        stdscr.addstr(3, 0, "  UP arrow   : Send UP   -> STM32")
        stdscr.addstr(4, 0, "  DOWN arrow : Send DOWN -> STM32")
        stdscr.addstr(5, 0, "  Q / ESC    : Quit")
        stdscr.addstr(6, 0, "========================================")
        stdscr.addstr(8, 0, f"  Motor State : {current_state:<20}")
        stdscr.addstr(9, 0, f"  STM32 says  : {last_response:<30}")
        stdscr.refresh()

        key = stdscr.getch()

        key_up   = (key == curses.KEY_UP)
        key_down = (key == curses.KEY_DOWN)

        if key_up and not prev_key_up:
            send_command(CMD_FORWARD)
            current_state = "UP (CW)"
        elif key_down and not prev_key_down:
            send_command(CMD_REVERSE)
            current_state = "DOWN (CCW)"

        if not key_up and not key_down and (prev_key_up or prev_key_down):
            send_command(CMD_STOP)
            current_state = "STOPPED"

        if key in (ord('q'), ord('Q'), 27):
            running = False
            break

        prev_key_up   = key_up
        prev_key_down = key_down

        curses.napms(50)

    send_command(CMD_STOP)
    if ser and ser.is_open:
        ser.close()


if __name__ == "__main__":
    curses.wrapper(main)
    print("Disconnected. Motor stopped.")
