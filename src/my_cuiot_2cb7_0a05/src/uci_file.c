
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <uci.h>
#include "uci_file.h"

profile_t f_profile_5g;

#define UCI_CONFIG_FILE         "module_info"
#define UCI_FIBO_PROFILE_5G     "fibocom_profile_5G"

int set_sim_info(char *value)
{
    return set_uci("fibocom_5g","sim",value);
}

int set_imei_info(char *value)
{
    return set_uci("fibocom_5g","imei",value);
}

int set_rssi_info(char *value)
{
    return set_uci("fibocom_5g","rssi",value);
}

int set_nettype_info(char *value)
{
    return set_uci("fibocom_5g","nettype",value);
}

int set_netstatus_info(char *value)
{
    return set_uci("fibocom_5g","netstatus",value);
}


int set_uci(char *section,char *option,char *value)
{
    struct uci_ptr ptr;
    memset(&ptr,0,sizeof(ptr));

    struct uci_context * _ctx = uci_alloc_context(); //申请上下文
    if (UCI_OK != uci_load(_ctx, UCI_CONFIG_FILE, &ptr.p))
    {
        printf("open set_uci UCI_CONFIG_FILE fail exit\n");
        goto set_cleanup; //如果打开UCI文件失败,则跳到末尾 清理 UCI 上下文. 

    }

    ptr.package ="config";
    ptr.section = section;
    ptr.option = option;
    ptr.value = value;
    uci_set(_ctx,&ptr); //写入配置 
    uci_commit(_ctx, &ptr.p, false); //提交保存更改  
    uci_unload(_ctx,ptr.p); //卸载包  

set_cleanup:  
    printf("uci set_uci exit\n");
    uci_free_context(_ctx);  //释放上下文
    return 0;
}

/********************************************* 
*   载入配置文件,并遍历Section. 
*/ 

int load_fibocom_profile(profile_t *profile_5g)
{  
    struct uci_package * pkg = NULL;  
    struct uci_element *e;
    char *value=NULL;

    struct uci_context *  ctx = uci_alloc_context(); // 申请一个UCI上下文.  
    if (UCI_OK != uci_load(ctx, UCI_FIBO_PROFILE_5G, &pkg))
    {
        printf("open file UCI_CONFIG_FILE fail exit\n");
        goto cleanup; //如果打开UCI文件失败,则跳到末尾 清理 UCI 上下文. 
    }

    /*遍历UCI的每一个节*/  
    uci_foreach_element(&pkg->sections, e)  
    {  
        struct uci_section *s = uci_to_section(e);  
        // 将一个 element 转换为 section类型, 如果节点有名字,则 s->anonymous 为 false.  
        // 此时通过 s->e.name 来获取.
        if(s->anonymous ==false)
        {
            printf("uci name [%s]\n",s->e.name);
        }
        // 此时 您可以通过 uci_lookup_option()来获取 当前节下的一个值.  
        if (NULL != (value = uci_lookup_option_string(ctx, s, "pin")))  
        {
            strcpy(profile_5g->pin,value);
            // value_sim = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim pin [%s]\n",profile_5g->pin);
        }
        if (NULL != (value = uci_lookup_option_string(ctx, s, "user")))  
        {
            strcpy(profile_5g->user,value);
            // value_imei = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim user [%s]\n",profile_5g->user);
        }

        if (NULL != (value = uci_lookup_option_string(ctx, s, "pwd")))  
        {
            strcpy(profile_5g->pwd,value);
            // config_rssi = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim pwd [%s]\n",profile_5g->pwd);
        }

        if (NULL != (value = uci_lookup_option_string(ctx, s, "apn")))  
        {
            strcpy(profile_5g->apn,value);
            // config_rssi = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim apn [%s]\n",profile_5g->apn);
        }

        if (NULL != (value = uci_lookup_option_string(ctx, s, "enable_5g")))  
        {
            strcpy(profile_5g->enable_5g,value);
            // config_rssi = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim enable_5g [%s]\n",profile_5g->enable_5g);
        }
        // 如果您不确定是 string类型 可以先使用 uci_lookup_option() 函数得到Option 然后再判断.  
        // Option 的类型有 UCI_TYPE_STRING 和 UCI_TYPE_LIST 两种.
    }

    uci_unload(ctx, pkg); // 释放 pkg   
cleanup:  
    printf("uci load_config exit\n");
    uci_free_context(ctx);  
    ctx = NULL;
    return 0;
}

int load_fibo_info(module_info_t *module_info)
{  
    struct uci_package * pkg = NULL;  
    struct uci_element *e;
    char *value=NULL;

    struct uci_context *ctx = uci_alloc_context(); // 申请一个UCI上下文.  
    if (UCI_OK != uci_load(ctx, UCI_CONFIG_FILE, &pkg))
    {
        printf("open file UCI_CONFIG_FILE fail exit\n");
        goto cleanup; //如果打开UCI文件失败,则跳到末尾 清理 UCI 上下文. 

    }

    /*遍历UCI的每一个节*/  
    uci_foreach_element(&pkg->sections, e)  
    {  
        struct uci_section *s = uci_to_section(e);  
        // 将一个 element 转换为 section类型, 如果节点有名字,则 s->anonymous 为 false.  
        // 此时通过 s->e.name 来获取.
        if(s->anonymous ==false)
        {
            printf("uci name [%s]\n",s->e.name);
        }

        // 此时 您可以通过 uci_lookup_option()来获取 当前节下的一个值.  
        if (NULL != (value = uci_lookup_option_string(ctx, s, "sim")))  
        {
            
            // value_sim = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            
            strcpy(module_info->sim,value);
            printf("sim value_sim [%s]\n",module_info->sim);
        }
        if (NULL != (value = uci_lookup_option_string(ctx, s, "imei")))  
        {
            strcpy(module_info->imei,value);
            // value_imei = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim value_imei [%s]\n",module_info->imei);
        }
        if (NULL != (value = uci_lookup_option_string(ctx, s, "rssi")))  
        {

            strcpy(module_info->rssi,value);
            // config_rssi = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim rssi [%s]\n",module_info->rssi);
        }
        if (NULL != (value = uci_lookup_option_string(ctx, s, "nettype")))  
        {

            strcpy(module_info->nettype,value);
            // config_rssi = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim nettype [%s]\n",module_info->nettype);
        }
        if (NULL != (value = uci_lookup_option_string(ctx, s, "netstatus")))  
        {

            strcpy(module_info->netstatus,value);
            // config_rssi = strdup(value); //如果您想持有该变量值，一定要拷贝一份。当 pkg销毁后value的内存会被释放。  
            printf("sim netstatus [%s]\n",module_info->netstatus);
        }
        // 如果您不确定是 string类型 可以先使用 uci_lookup_option() 函数得到Option 然后再判断.  
        // Option 的类型有 UCI_TYPE_STRING 和 UCI_TYPE_LIST 两种.
    }

    uci_unload(ctx, pkg); // 释放 pkg   
cleanup:  
    printf("uci load_config exit\n");
    uci_free_context(ctx);  
    ctx = NULL;
    return 0;
}


void uci_load_file(void)
{
    // module_info_t f_module_info;

    // memset(&f_module_info,0,sizeof(profile_t));
    // load_fibo_info(&f_module_info);

    // printf("f_module_info.sim[%s]\n",f_module_info.sim);
    // printf("f_module_info.rssi[%s]\n",f_module_info.rssi);
    // printf("f_module_info.nettype[%s]\n",f_module_info.nettype);
    // printf("f_module_info.netstatus[%s]\n",f_module_info.netstatus);
    // printf("f_module_info.imei[%s]\n",f_module_info.imei);

    memset(&f_profile_5g,0,sizeof(profile_t));
    load_fibocom_profile(&f_profile_5g);

    printf("f_profile_5g.pin[%s]\n",f_profile_5g.pin);
    printf("f_profile_5g.user[%s]\n",f_profile_5g.user);
    printf("f_profile_5g.pwd[%s]\n",f_profile_5g.pwd);
    printf("f_profile_5g.apn[%s]\n",f_profile_5g.apn);
    printf("f_profile_5g.enable_5g[%s]\n",f_profile_5g.enable_5g);


}
// uci_add_list()  // 添加一个list 值  
// uci_del_list()  // 删除一个list 值  
// uci_delete()    // 删除一个option值  

