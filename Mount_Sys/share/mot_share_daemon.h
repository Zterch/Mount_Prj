/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 11:16:48
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-05 16:34:19
 * @FilePath: /Mount_Sys/share/mot_share_daemon.h
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

#ifndef MOT_SHARE_DAEMON_H
#define MOT_SHARE_DAEMON_H



#include <stdint.h>



void hiSetCpuAffinity(int mCpuI);


void hiSetThreadsched(pthread_t pPid, const int32_t iPriority);



pthread_t hiCreateThread(const char *cThreadName,
                         void *(*thread_start)(void *),
                         int32_t iPriority,
                         void* pModule); 

 #endif
