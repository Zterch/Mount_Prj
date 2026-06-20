/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-26 18:02:17
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-09 14:23:21
 * @FilePath: /Mount_Sys/leader/include/mot_leader.h
 * @Description: 系统中的超级线程模块，此模块负责创建所有线程，管理其他模块，周期的发送
消息通知其他模块。
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */


#ifndef MOT_LEADER_H
#define MOT_LEADER_H

#include <stdint.h>
#include "mot_leader_defs.h"

int32_t leaderAppStartUp(const char** mSuprMsgK);

#endif
