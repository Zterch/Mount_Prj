/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 10:49:12
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-09 14:52:31
 * @FilePath: /Mount_Sys/share/mot_ctl.c
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "mot_ctl.h"
#include "mot_share_conf.h"


/*以下是不可修改的参数,供多线程使用*/
static param_dic const_params[] = 
{
    {"SUPR_PORTS",0,"30001"},
    {"LEADER_PORTS",0,"30002"},
    {"MASTER0_PORTS",0,"30011"},
    {"MASTER1_PORTS",0,"30012"},
    {"LISTENER_LOCAL_PORT",0,"30021"},
    {"LOGGER_LOCAL_PORT",0,"30022"},
    {"DB_WORKER_PORT",0,"30023"},
    //{"LISTENER_TO_UPPER_IP",0,"192.168.1.11"},
    {"LISTENER_TO_UPPER_PORT",0,"33333"},
   
    {"VISION_IP",0,"192.168.1.41"},
    {"VISION_PORT",0,"39001"},
    {"MASTER_60A3D_IP",0,"192.168.1.41"},
    {"MASTER_60A3D_PORT",0,"13131"},
    {"DAM_60A3D1_IP",0,"192.168.1.250"},
    {"DAM_60A3D1_PORT",0,"1030"},
    {"DAM_60A3D2_IP",0,"192.168.1.251"},
    {"DAM_60A3D2_PORT",0,"1030"},
    
    {"SERIAL_WORKER_IP",0,"192.168.1.41"},
    {"SERIAL_WORKER_PORT",0,"13132"},

    {"DAM_GW1800_IP",0,"192.168.1.253"},
    {"DAM_GW1800_PORT",0,"1030"},

    {"",0,""}
};

char * get_const_param_value_str(char *key)
{
    int pI = 0;
    char *p = NULL;
    while (1)
    {
        if(strlen(const_params[pI].key)>0 && (strcmp(key,const_params[pI].key) == 0))
        {
            p = const_params[pI].description;
            break;
        }
        pI++;
    }

    return p;
}


/**
 * @brief get_config_mn_port 得到mn_port配置值
 * 遍历每行数据 , 检查 key 键 是否存在
 * 如果存在 , 就更新对应的 value 值
 * 如果不存在 , 在文件末尾添加该键值对信息
 * 格式为 :
 * key = value
 *
 * @param filename 文件名称
 * @param key 键
 * @param value 值
 * @param value_len 值的长度
 * @return
 */
int get_mn_port(int index)
{
    int iRet = 0;
    if(MN_ID_SUPR == index)
        iRet = strtol(get_const_param_value_str("SUPR_PORTS"),NULL,10);
    if(MN_ID_LEADER == index)
        iRet = strtol(get_const_param_value_str("LEADER_PORTS"),NULL,10);

    return iRet;
}


int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
 
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }
 
    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
 
    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }
 
    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, IP_C_SIZE, "%s", inet_ntoa(sin.sin_addr));
 
    close(sd);
    return 0;
}


uint32_t get_port(char *key)
{
    return strtol(get_const_param_value_str(key),NULL,10);
}

int get_supr_port()
{
    return strtol(get_const_param_value_str("SUPR_PORTS"),NULL,10);
}

/**
 * @brief get_mn_master_port 得到mn_port配置值
 * 遍历每行数据 , 检查 key 键 是否存在
 * 如果存在 , 就更新对应的 value 值
 * 如果不存在 , 在文件末尾添加该键值对信息
 * 格式为 :
 * key = value
 *
 * @param filename 文件名称
 * @param key 键
 * @param value 值
 * @param value_len 值的长度
 * @return
 */
int get_mn_master_port(int index)
{
    int iRet = 0;
    if(0 == index)
        iRet = strtol(get_const_param_value_str("MASTER0_PORTS"),NULL,10);
    if(1 == index)
        iRet = strtol(get_const_param_value_str("MASTER1_PORTS"),NULL,10);
    
    return iRet;
}
