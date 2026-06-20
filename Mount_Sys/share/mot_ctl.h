/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 10:49:19
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-08 09:50:15
 * @FilePath: /Mount_Sys/share/mot_ctl.h
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */



 #ifndef MOT_CTL_H
 #define MOT_CTL_H

#include <stdint.h>

#define IP_C_SIZE     16


// 定义接口时 , 如果函数形参用作输入数据时 , 可以在形参名很后面添加 /*in*/ 注释
//
typedef struct param_dic
{
  char key[64];
  float value;
  char description[256];
}param_dic;


 //得到模块:supr  leader ...的端口
int get_mn_port(int index);
uint32_t get_port(char *key);
int get_local_ip(const char *eth_inf, char *ip);
int get_supr_port();
int get_mn_master_port(int index);

#endif