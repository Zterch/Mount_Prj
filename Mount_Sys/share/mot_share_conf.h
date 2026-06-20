/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 10:54:39
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-12 08:33:25
 * @FilePath: /Mount_Sys/share/mot_share_conf.h
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

 //系统中所有线程模块ID,增加模块时要做对应修改
#ifndef SYS_SHARE_CONF_H
#define SYS_SHARE_CONF_H

//定义线程模块优先级
#define PRI_SUPR    60
#define PRI_MASTER  50
#define PRI_LEAD    60

//定义线程模块的CPU亲和度，绑定核
#define CPU_SUPR    1
#define CPU_MASTER  2
#define CPU_LEAD    3

//////////////////////////////////// System supr conf  ///////////////////////////////////////
//最大的模块数量,leader、timer、loger
#define MODULES_NUMS (1)

/********************************leader conf************************************/
//master的名字

//一个主站下，最多拥有的机械臂个数
#define MASTER_ARMS_MAX_SIZE 1

typedef enum {
  MN_ID_SUPR,
  MN_ID_LEADER,
  MN_ID_MAX,

} MODULE_NAME_ID;

/****************************supr leader使用ip及端口*************************/
#define    MN_LocalIpV4Str "127.0.0.1"

/****************************leader下的master线程使用*************************/
//master线程的id，0 1 2：上层、中层、底层
typedef enum {
  MN_ID_MASTER_ARMS,
  MN_ID_MASTER_CHASSIS,
} MASTER_ID;


#endif