
#include <stdio.h>
#include <string.h> 
#include <unistd.h>
#include <fcntl.h> 
#include <termios.h>

#define DATA_LEN	0xFF		/* test data's len */

//#define DEBUG		1

static int openSerial(char *cSerialName)
{
    int iFd;

    struct termios opt; 

    iFd = open(cSerialName, O_RDWR | O_NOCTTY);                        
    if(iFd < 0) {
        perror(cSerialName);
        return -1;
    }

    tcgetattr(iFd, &opt);      

    //cfsetispeed(&opt, B57600);
    //cfsetospeed(&opt, B57600);

    cfsetispeed(&opt, B115200);
    cfsetospeed(&opt, B115200);

    /* raw mode */
    opt.c_lflag   &=   ~(ECHO   |   ICANON   |   IEXTEN   |   ISIG);
    opt.c_iflag   &=   ~(BRKINT   |   ICRNL   |   INPCK   |   ISTRIP   |   IXON);
    opt.c_oflag   &=   ~(OPOST);
    opt.c_cflag   &=   ~(CSIZE   |   PARENB);
    opt.c_cflag   |=   CS8;

    /* 'DATA_LEN' bytes can be read by serial */
    opt.c_cc[VMIN]   =   DATA_LEN;                                      
    opt.c_cc[VTIME]  =   150;

    if (tcsetattr(iFd,   TCSANOW,   &opt)<0) {
        return   -1;
    }

    return iFd;
}

int main(int argc, char **argv) 
{
    int		fd, i, len;
    char	buffer[DATA_LEN];

    char *	dev		= NULL;
    char *	string	= NULL;

    dev		= argv[1];
    string	= argv[2];

    fd = openSerial(dev);
    printf("Starting send data...");
    write(fd, string, strlen(string)+1);
    printf("finish\n");

    printf("Starting receive data:\n");
    while (1) {
        len = read(fd, buffer, 0x01);	
        for(i = 0; i < len; i++)
            printf("ASCII: 0x%x \t Character: %c \n", buffer[i], buffer[i]);
    }
}
