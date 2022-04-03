#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include <unistd.h>

#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */
#include <errno.h>   /* ERROR Number Definitions           */
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libubox/uloop.h>

#include "uart.h"
#include "lte_dial.h"
#include "uci_file.h"
#include "process_fifo_msg.h"


int main(void)
{

    pid_t pid=fork();
    if(pid<0)
    {
      //The child thread
      printf("\n app lte fork fail pid[%d]\n",pid);
      return -1;
    }
    else if(pid>0)
    {
      printf("\n app fork The parent thread exit pid [%d] \n",pid);
      exit(0); //退出父线程
    }
    printf("\n app fork The child thread init pid [%d] \n",pid);

    int ret =0;
    printf("*************************************\n");
    printf("user app versions:V100.20.06.22\n");
    printf("*************************************\n");

    printf("\n app lte pthread_create 123! \n");
  
    lte_fifo_enter();
    lte_dial_task(NULL);

    sleep(1);
    printf("\n app lte pthread_end!\n");
    return 1;
}
