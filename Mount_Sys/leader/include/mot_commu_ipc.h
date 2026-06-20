/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-26 18:02:04
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-08 10:32:47
 * @FilePath: /Mount_Sys/leader/include/mot_commu_ipc.h
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */
#ifndef __MOT_COMMU_IPC_H
#define __MOT_COMMU_IPC_H

#include "mot_ctl.h"

//发送给leader通知
int32_t sendNoticeToLeader(int32_t mSocket,const int32_t mMsgId, const int32_t mNotice, const int32_t mValue);



#endif
