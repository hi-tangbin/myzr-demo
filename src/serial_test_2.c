
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define DATA_LEN    255
#define SERIAL_NUM  4
int tty_fd[SERIAL_NUM];

static int open_serial(char *dev_serial)
{
    int fd;
    struct termios opt;

    fd = open(dev_serial, O_RDWR | O_NOCTTY);
    if(fd < 0) {
        perror(dev_serial);
        return -1;
    }

    tcgetattr(fd, &opt);
    cfsetispeed(&opt, B115200);
    cfsetospeed(&opt, B115200);

    /* raw mode */
    opt.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    opt.c_oflag &= ~(OPOST);
    opt.c_cflag &= ~(CSIZE | PARENB);
    opt.c_cflag |= CS8;

    /* 'DATA_LEN' bytes can be read by serial */
    opt.c_cc[VMIN] = DATA_LEN;
    opt.c_cc[VTIME] = 150;

    if (tcsetattr(fd, TCSANOW, &opt) < 0)
        return -1;

    return fd;
}

int main(int argc, char **argv)
{
    int i, str_len, size_ret, w_index, temp;

    char tty_dev[16]        = {0};
    char w_buf[DATA_LEN]    = {0};
    char r_buf[DATA_LEN]    = {0};

    str_len = strlen(argv[1]);
    if (str_len != 12) {
        printf("Error: device is %s ???\n", argv[1]);
        return -1;
    } else {
        w_index = argv[1][11] - '1';
        strncpy(tty_dev, "/dev/ttymxc1", str_len + 1);
    }

    for (i = 0; i < SERIAL_NUM; i++) {
        printf("Open Serial: %s\n", tty_dev);
        tty_fd[i] = open_serial(tty_dev);
        if (tty_fd[i] < 0)
            return -1;

        if (fcntl(tty_fd[i], F_SETFL, FNDELAY) < 0) {
            printf("Set F_SETFL Error\n");
            return -1;
        }

        tcflush(tty_fd[i], TCIFLUSH);
        tty_dev[str_len-1]++;
    }

    temp = tty_fd[0];
    tty_fd[0] = tty_fd[w_index];
    tty_fd[w_index] = temp;

    str_len = strlen(argv[2]) + 1;
    strncpy(w_buf, argv[2], str_len);

    while(1) {
        printf("Send(%d.%d): %s\n", tty_fd[0]+1-temp, str_len, w_buf);
        tcflush(tty_fd[0], TCIFLUSH);
        write(tty_fd[0], w_buf, str_len);
        usleep(6000);

        for (i = 1; i < SERIAL_NUM; i++) {
            memset(r_buf, 0, sizeof(r_buf));
            size_ret = read(tty_fd[i], r_buf, sizeof(r_buf));
            tcflush(tty_fd[i], TCIFLUSH);
            printf("Recv(%d.%d): %s\n", tty_fd[i]+1-temp, size_ret, r_buf);
        }

        sleep(1);
    }

    for (i = 0; i < SERIAL_NUM; i++)
        close(tty_fd[i]);

    return 0;
}
