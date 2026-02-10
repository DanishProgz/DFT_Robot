#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUD B115200

struct termios orig_term;

/* ================= UART SETUP ================= */

int uart_open(const char *port)
{
    int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    tcgetattr(fd, &tty);

    cfsetospeed(&tty, BAUD);
    cfsetispeed(&tty, BAUD);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tcsetattr(fd, TCSANOW, &tty);

    return fd;
}

void uart_send(int fd, const char *msg)
{
    write(fd, msg, strlen(msg));
}

/* ================= RAW KEYBOARD ================= */

void enable_raw_mode()
{
    tcgetattr(STDIN_FILENO, &orig_term);

    struct termios raw = orig_term;
    raw.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

/* ================= MAIN ================= */

int main()
{
    int fd = uart_open(SERIAL_PORT);

    printf("\nUART Jog Controller Ready\n");
    printf("Press Up arrow to Move Forward\n");
    printf("Press Down Arrow to Move Reverse");

    enable_raw_mode();

    while (1)
    {
        char c = getchar();

        if (c == 27)   // ESC detected → arrow key sequence
        {
            char seq1 = getchar();
            char seq2 = getchar();

            if (seq1 == '[')
            {
                if (seq2 == 'A')   // UP arrow
                {
                    uart_send(fd, "UP\n");
                }
                else if (seq2 == 'B')  // DOWN arrow
                {
                    uart_send(fd, "DOWN\n");
                }
            }
        }

        else if (c == 'q')
        {
            break;
        }
    }

    disable_raw_mode();
    close(fd);

    printf("\nExited cleanly\n");

    return 0;
}

