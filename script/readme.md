# UART Jog Controller — PC Side

## Overview

`uart_jog.c` is a lightweight Linux terminal application used to control an STM32-based robot over UART.

It allows real-time keyboard jogging of motors by sending simple command strings to the STM32 firmware.

The STM32 receives these commands and moves the motors accordingly.

This tool is intended for:

* Hardware bring-up
* Motion testing
* Manual jogging
* Development diagnostics

---

## Requirements

* Linux PC (Ubuntu recommended)
* USB-to-TTL serial adapter
* STM32 board running compatible firmware
* GCC compiler installed

Install GCC if needed:

```bash
sudo apt install build-essential
```

---

## Hardware Connection

Connect your USB-TTL adapter:

```
PC USB adapter TX → STM32 RX
PC USB adapter RX → STM32 TX
GND → GND
```

After connecting, verify the serial device:

```bash
ls /dev/ttyUSB*
```

Typical result:

```
/dev/ttyUSB0
```

---

## Build Instructions

Compile the program:

```bash
gcc uart_jog.c -o uart_jog
```

If you see permission issues later:

```bash
sudo usermod -a -G dialout $USER
```

Log out and back in.

---

## Running the Program

Start the jog controller:

```bash
./uart_jog
```

You should see:

```
UART Jog Controller Ready
Press arrow keys to jog motor
Press Q to quit
```

---

## Controls

| Key          | Action       |
| ------------ | ------------ |
| ↑ Up Arrow   | Jog forward  |
| ↓ Down Arrow | Jog reverse  |
| Q            | Exit program |

Holding a key sends repeated jog commands to the STM32.

Releasing the key stops motion.

---

## Communication Behavior

When keys are pressed:

```
PC → sends: "UP\n" or "DOWN\n"
STM32 → executes Stepper_Move()
```

This creates smooth incremental jogging.

---

## Troubleshooting

### Serial device not found

Check available ports:

```bash
ls /dev/ttyUSB*
dmesg | tail
```

Update `SERIAL_PORT` inside `uart_jog.c` if needed.

---

### Device busy error

Close other serial programs:

```bash
sudo fuser -k /dev/ttyUSB0
```

---

### Keys not responding

Ensure terminal raw mode is enabled and STM32 firmware is running.

---

## Notes

* This program is designed for development and testing.
* It is not intended for high-speed motion streaming.
* For advanced control, extend the command protocol.

---

## Future Extensions

Possible upgrades include:

* Multi-motor control
* Speed scaling
* Continuous motion mode
* Status feedback
* Joystick input

---

## Author

Danish Mehboob — Embedded motion control development
