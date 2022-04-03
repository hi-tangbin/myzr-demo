#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>


char sim_msg[20]={0};
char rssi_msg[10]={0};
char nettype_msg[10]={0};
char netstatus_msg[20]={0};
char imei_msg[25]={0};


static char at_lte_name[]="/tmp/at_lte.fifo";
static char lte_at_name[]="/tmp/lte_at.fifo";

int lte_fifo_send(char *msg,int len)
{
    int ret=-1;
    int file_id=0;

    file_id=open(lte_at_name,O_RDWR);
    if(file_id<0)
    {
        printf("fifo1 open %s fail ret[%d]\n",lte_at_name,file_id);
        return -1;
    }
    ret=write(file_id,msg,strlen(msg));
    printf("fifo1 write ret [%d] [%s]\n",ret,msg);
    if(ret<0)
    {
        printf("fifo1 write %s fail ret[%d]\n",lte_at_name,file_id); 
    }
    close(file_id);

    return 0;
}


int lte_fifo_recv(char *recv_msg,int len)
{
    int ret=0;
    int file_id=0;

    file_id=open(at_lte_name,O_RDONLY);
    if(file_id<0)
    {
        printf("fifo1 open %s fail ret[%d]\n",at_lte_name,file_id);
        return -1;
    }
  
    ret=read(file_id,recv_msg,len);
    printf("fifo1 recv %s [%d] [%s]\n",at_lte_name,ret,recv_msg);
    if(ret<0)
    {
        printf("fifo1 recv %s fail f2tof1_id[%d]\n",at_lte_name,file_id); 
        sleep(3);
    }
    close(file_id);
    return ret;
}


void * lte_fifo_task(void* arg)
{

    int pipe_fd=0;
    char recv_buff[1024]={0};
    int ret=0;

    if(access(at_lte_name,F_OK) == -1)
    {
        if(mkfifo(at_lte_name,0777)<0)
        {
            printf(" mkfifo %s fial !!!\n",at_lte_name);
        }
    }
    if(access(lte_at_name,F_OK) == -1)
    {
        if(mkfifo(lte_at_name,0777)<0)
        {

        }
    }

    while(1)
    {
        memset(recv_buff,0,sizeof(recv_buff));
        ret = lte_fifo_recv(recv_buff,sizeof(recv_buff));
        usleep(1000*100);
        if(strstr(recv_buff,"CPIN"))
        {
            lte_fifo_send(sim_msg,strlen(sim_msg));
            printf("sim_msg[%s]\n",sim_msg);
        }
        else if(strstr(recv_buff,"COPS"))
        {
            lte_fifo_send(netstatus_msg,strlen(netstatus_msg));
            printf("netstatus_msg[%s]\n",netstatus_msg);
        }
        else if(strstr(recv_buff,"CSQ"))
        {
            lte_fifo_send(rssi_msg,strlen(rssi_msg));
            printf("rssi_msg[%s]\n",rssi_msg);

        }
        else if(strstr(recv_buff,"CREG"))
        {
            lte_fifo_send(nettype_msg,strlen(nettype_msg));
            printf("nettype_msg[%s]\n",nettype_msg);

        }
        else if(strstr(recv_buff,"GSN"))
        {
            
            lte_fifo_send(imei_msg,strlen(imei_msg));
            printf("imei_msg[%s]\n",imei_msg);

        }
        else if(strstr(recv_buff,"QENG"))
        {

            lte_fifo_send(nettype_msg,strlen(nettype_msg));
            printf("nettype_msg[%s]\n",nettype_msg);
        }
        else
        {
            lte_fifo_send("LTE Invalid input",strlen("LTE Invalid input"));
            printf("LTE Invalid input\n");
        }

    }
}


int lte_fifo_enter(void)
{
    int ret =-1;
    pthread_t  lte_fifo_task_id;
    ret = pthread_create(&lte_fifo_task_id,NULL,lte_fifo_task,NULL);
    return ret;
}
    

