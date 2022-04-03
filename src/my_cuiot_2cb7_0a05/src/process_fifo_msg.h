#ifndef _VENC_FIFO_H_
#define _VENC_FIFO_H_

extern char sim_msg[20];
extern char rssi_msg[10];
extern char nettype_msg[10];
extern char netstatus_msg[20];
extern char imei_msg[25];

int lte_fifo_enter(void);
int lte_fifo_send(char *msg,int len);

#endif // !_VENC_FIFO_H_
