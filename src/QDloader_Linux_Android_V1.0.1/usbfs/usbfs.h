#ifndef _USBFS_H_
#define _USBFS_H_
typedef struct {
/*
MAJOR=189
MINOR=1
DEVNAME=bus/usb/001/002
DEVTYPE=usb_device
DRIVER=usb
PRODUCT=2c7c/415/318
TYPE=239/2/1
BUSNUM=001
*/
    char sys_path[64];
    char dev_name[32];
    char PRODUCT[32];
} USBFS_T;

extern int usbfs_find_quectel_modules(USBFS_T *usbfs_dev, size_t max);
extern int sendSync(int usbfd, const void *pBuf, size_t len, int need_zlp);
extern int recvSync(int usbfd, void *data, size_t len, unsigned timeout);
extern int usbfs_open(const USBFS_T *usbfs_dev, int interface_no);
extern int usbfs_close(int usbfd, int interface_no);
int strStartsWith(const char *line, const char *prefix);

typedef struct quectel_modem_info
{
    const char *modem_name;
    const char *PRODUCT;
    int usb_interface;
    const char *download_atc;
} quectel_modem_info;
extern const struct quectel_modem_info * find_quectel_modem(const char *PRODUCT);
#endif
