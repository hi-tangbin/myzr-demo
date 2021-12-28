#include "std_c.h"
#include <getopt.h>
#include "usbfs/usbfs.h"

extern int dloader_main(int usbfd, FILE *fw_fp, const char *modem_name);
extern int fbfdownloader_main(int usbfd, FILE *fw_fp, const char *modem_name);
extern int aboot_main(int usbfd, FILE *fw_fp, const char *modem_name);

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *log_fp = NULL;
char log_buf[1024];
char g_specify_port[64] = {0};

static const struct quectel_modem_info quectel_modems[] =
{
    {"RG500U", "2c7c/900/",  4, "AT+QDownLOAD=1\r"},
    {"RG500U", "1782/4d00/", 0, NULL},
    {"EC200U", "2c7c/901/",  2, "AT+QDownLOAD=1\r"},
    {"EC200U", "525/a4a7/",  1, NULL},
    {"EC200T", "2c7c/6026/", 3, "AT+CFUN=1,1\r"},
    {"EC200T", "1286/812a/", 0, NULL},
    {"EC200S", "2c7c/6002/", 3, "AT+QDownLOAD=1\r"},
    {"EC200S", "2c7c/6001/", 3, "AT+QDownLOAD=1\r"},
    {"EC200S", "2ecc/3017/", 1, NULL},
    {"EC200S", "2ecc/3004/", 1, NULL},
    {"EC200T", "3763/3c93/", 1, "AT+CFUN=1,1\r"},
    {"EC200T", "3c93/ffff/", 3, "AT+CFUN=1,1\r"},
    {"EC200A", "2c7c/6005/", 3, "AT+CFUN=1,1\r"},
    {"EC200A", "2ecc/3001/", 0, NULL},
    {"RM500U", "3c93/ffff/", 3, "AT+QDownLOAD=1\r"},
    {"RM500U", "1782/4d00/", 0, NULL},
    {NULL,      NULL,        0, NULL},
};

const struct quectel_modem_info * find_quectel_modem(const char *PRODUCT) {
    int i = 0;
    while (quectel_modems[i].modem_name) {
        if (strStartsWith(PRODUCT, quectel_modems[i].PRODUCT)) {
            if (strstr(PRODUCT,"404") && (strstr(PRODUCT,"3c93") && strstr(PRODUCT,"ffff")))
            {
                if (strncasecmp(quectel_modems[i].modem_name, "RM500U", 6))
                {
                    i++;
                    continue;
                }
            }else if (strstr(PRODUCT,"100") && (strstr(PRODUCT,"3c93") && strstr(PRODUCT,"ffff")))
            {
                if (strncasecmp(quectel_modems[i].modem_name, "EC200T", 6))
                {
                    i++;
                    continue;
                }
            }
            return &quectel_modems[i];
        }
        i++;
    }
    return NULL;
}
int verbose = 0;
static USBFS_T usbfs_dev[8];

int strStartsWith(const char *line, const char *prefix)
{
    for ( ; *line != '\0' && *prefix != '\0' ; line++, prefix++) {
        if (*line != *prefix) {
            return 0;
        }
    }

    return *prefix == '\0';
}

const char *get_time(void) {
    static char str[64];
    static unsigned start = 0;
    unsigned now;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    now = (unsigned)ts.tv_sec*1000 + (unsigned)(ts.tv_nsec / 1000000);
    if (start == 0)
        start = now;

    now -= start;
    snprintf(str, sizeof(str), "[%03d:%03d]", now/1000, now%1000);

    return str;
}

static void switch_to_download(int usbfd, const char *download_atc, const char *dev_modem_name)
{
    sendSync(usbfd, "ATI\r", 4, 0);
    sendSync(usbfd, "AT+EGMR=0,5\r", 12, 0);
    sendSync(usbfd, "AT+EGMR=0,7\r", 12, 0);

    while (!access(dev_modem_name, F_OK)) {
        int ret;
        char at_rsp[256];

        dprintf("AT > %s\n", download_atc);
        ret = sendSync(usbfd, download_atc, strlen(download_atc), 0);
        if (ret < 1) break;

        do  {
            ret = recvSync(usbfd, at_rsp, sizeof(at_rsp), 1000);
            if (ret > 0) {
                int i, len = 0;

                for (i = 0; i < ret; i++) {
                    uint8_t ch = at_rsp[i];

                    if (ch != '\r' && ch != '\n')
                        at_rsp[len++] = ch;
                    else if (len > 1) {
                        at_rsp[len++] = 0;
                        dprintf("AT < %s\n", at_rsp);
                        len = 0;
                    }
                }
            }
        } while (ret > 0);

        if (errno != ETIMEDOUT) {sleep(1); break;};
    }
}

static int usage(const char *self) {
    printf("%s -f fw_image\n", self);
    printf("for EC200U, fw image is '.pac',         can be found in the FW package\n");
    printf("for RG500U, fw image is '.pac',         can be found in the FW package\n");
    printf("for EC200S, fw image is 'firmware.bin', can be found in the FW package\n");
    printf("for EC200T, fw image is 'BinFile.bin',  create by windows tool 'FBFMake.exe -r update.blf -f output', .blf file can be found in the FW package\n");

    return 0;
}

static int get_port_from_cmd(char* g_specify_port)
{
    if (g_specify_port[0] == '\0' || strncasecmp(g_specify_port, "/dev", 4) != 0)
        return -1;
    FILE* fp = NULL;
    char cmd_buf[80] = {0};
    char recv_buf[256] = {0};
    snprintf(cmd_buf, sizeof(cmd_buf), "ls -l /sys/class/tty/%s", g_specify_port+5);
    fp = popen(cmd_buf, "r");
    if (fp == NULL)
        return -1;
    int recv_length = fread(recv_buf, 1, sizeof(recv_buf), fp);
    if (recv_length <= 0)
        return -1;
    char* ptr = strstr(recv_buf, "/usb");
    if (ptr == NULL)
        return -1;
    dprintf("ptr:%s\n",ptr);
    memset(g_specify_port, 0, 64);
    if (ptr[13] == '.')
    {
        strncpy(g_specify_port, ptr+10, 5);
        dprintf("g_specify_port:%s\n",g_specify_port);
    }else if (ptr[13] == ':')
    {
        strncpy(g_specify_port, ptr+10, 3);
        dprintf("g_specify_port:%s\n",g_specify_port);
    }
	
    return 0;
}

int main(int argc, char *argv[])
{
    int i, ret = -1, opt, count, usbfd;
    const USBFS_T *udev;
    const quectel_modem_info *mdev;
    const char *fw_file = NULL;
    FILE *fw_fp = NULL;

#ifdef EC200T_SHINCO
    log_fp = fopen("/data/update/meig_linux_update_for_arm/update_log.txt", "wb");
    dprintf("EC200T_SHINCO 2021/4/29\n");
#endif
    dprintf("Version: QDloader_Linux_Android_V1.0.1\n");

    optind = 1; //call by popen(), optind mayby is not 1
    while (-1 != (opt = getopt(argc, argv, "s:f:vh")))
    {
        switch (opt)
        {
        case 's':
            snprintf(g_specify_port, sizeof(g_specify_port), "%s", optarg);
            printf("optarg:%s\n",optarg);
            if(get_port_from_cmd(g_specify_port) < 0)
                memset(g_specify_port, 0, sizeof(g_specify_port));
        break;
        case 'f':
            fw_file = strdup(optarg);
        break;
        case 'v':
            verbose++;
            printf("v=%d\n", verbose);
        break;
        default:
            return usage(argv[0]);
        break;
         }
    }

    if (!fw_file) {
        return usage(argv[0]);
    }

    fw_fp = fopen(fw_file, "rb");
    if (!fw_fp) {
  	    dprintf("fopen(%s) fail, errno: %d (%s)\n", fw_file, errno, strerror(errno));
        return -1;
    }

_scan_modem:
    dprintf("scan quectel modem\n");
    for (i = 0; i < 50; i++) {
        count = usbfs_find_quectel_modules(usbfs_dev, 8);
        if (count > 0) break;
        usleep(200*1000);
    }
    if (count == 0) {
        dprintf("not quectel modem find!\n");
        return -1;
    }
    udev = &usbfs_dev[0];

    mdev = find_quectel_modem(udev->PRODUCT);
    if (!mdev)
    {    goto _scan_modem;}

    usbfd = usbfs_open(udev, mdev->usb_interface);
    if (usbfd == -1)
        goto _scan_modem; //return -1;

    if (mdev->download_atc) {
        switch_to_download(usbfd, mdev->download_atc, udev->dev_name);
        usbfs_close(usbfd, mdev->usb_interface);
        goto _scan_modem;
    }

    update_transfer_bytes(0);
    if (!strcmp(mdev->modem_name, "RG500U") || !strcmp(mdev->modem_name, "RM500U")) {
        ret = dloader_main(usbfd, fw_fp, mdev->modem_name);
    }
    else if (!strcmp(mdev->modem_name, "EC200U")) {
        ret = dloader_main(usbfd, fw_fp, mdev->modem_name);
    }
    else if (!strcmp(mdev->modem_name, "EC200T") || !strcmp(mdev->modem_name, "EC200A")) {
        ret = fbfdownloader_main(usbfd, fw_fp, mdev->modem_name);
    }
    else if (!strcmp(mdev->modem_name, "EC200S")) {
        ret = aboot_main(usbfd, fw_fp, mdev->modem_name);
    }

    fclose(fw_fp);
    usbfs_close(usbfd, mdev->usb_interface);

    if (!ret) {
        for (i = 0; i < 100; i++) {
            if (access(udev->dev_name, F_OK)) break; //wait reboot
            usleep(10*1000);
        }
    }

    if (ret == 0)
        update_transfer_bytes(0xffffffff);
    update_transfer_result(ret == 0);
    dprintf("Upgrade module %s\n", (ret == 0) ? "successfully" : "fail");
    if (log_fp && log_fp != stdout)
        fclose(log_fp);
    return ret;
}
