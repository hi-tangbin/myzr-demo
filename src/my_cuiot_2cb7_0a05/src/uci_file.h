#ifndef _UCI_FILE_H_
#define _UCI_FILE_H_

typedef struct {
    char pin[50];
    char user[50];
    char pwd[128];
    char apn[128];
    char enable_5g[10];
}profile_t;

typedef struct {
    char sim[20];
    char rssi[10];
    char nettype[10];
    char netstatus[20];
    char imei[20];
}module_info_t;


extern profile_t f_profile_5g;

int load_fibo_info(module_info_t *module_info);
int load_fibocom_profile(profile_t *profile_5g);
int set_uci(char *section,char *option,char *value);
void uci_load_file(void);

// value:NO SIM         SIM READY
int set_sim_info(char *value);

//value :
int set_imei_info(char *value);

//value 0-31
int set_rssi_info(char *value);

//value :   4G   5G
int set_nettype_info(char *value);

//value:    CONNECT   DISCONECT
int set_netstatus_info(char *value);

#endif // !_UCI_FILE_H_


