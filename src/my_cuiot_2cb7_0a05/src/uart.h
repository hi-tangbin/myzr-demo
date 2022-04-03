#ifndef _UART_H_
#define _UART_H_
#include"stdint.h"

#define UART0_EN    0    

//此uart 使用的NMEA的uart  測試可以正常收發，外部使用GNSS時需注意，uart是否回衝突

int uart_open(char* port); 
void uart_close(int fd);  
int uart_set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int uart_init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity);
int uart_recv(int fd, char *rcv_buf,int data_len,int timeout_ms);
int uart_send(int fd, char *send_buf,int data_len);
int uart_send_cmd(int fd,char *cmd,char *result_buf,int result_len,int timeout_ms);

void *uart_pthread(void* arg); 

#endif // !_UART_H_