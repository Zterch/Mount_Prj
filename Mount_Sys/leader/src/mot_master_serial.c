/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-06-05 14:32:08
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-09 14:47:33
 * @FilePath: /Mount_Sys/leader/src/mot_master_serial.c
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "mot_master_serial.h"
#include "mot_leader_defs.h"

static inline void tsnorm(struct timespec *ts);



/******************************************************************************
* 功能：tsnorm函数将timespec中的纳秒，妙格式化。按照1m = 1000000000ns，
* @param ts : ts是时间结构体，包含：妙 纳秒
* @return Descriptions
******************************************************************************/
static inline void tsnorm(struct timespec *ts)
{
    while (ts->tv_nsec >= NSEC_PER_SEC) {
        ts->tv_nsec -= NSEC_PER_SEC;
        ts->tv_sec++;
    }
}
 /******************************************************************************
* 功能：子线程：master使用serial线程入口函数，此模块的生命周期就在此函数中。
* @param pTModule : args
* @return Descriptions
******************************************************************************/
void* serialThread(void* pModule)
{
    SERIAL_THREAD_INFO *pSModule = (SERIAL_THREAD_INFO *) pModule;
    //ARMS_MASTER_THREAD_INFO *pTModule = (ARMS_MASTER_THREAD_INFO *)pSModule->pTModule;

    unsigned int nDelay = 1000;        /* usec */
    //timer 定时器
    struct timespec  next, interval;
    interval.tv_sec  = nDelay / USEC_PER_SEC;
    interval.tv_nsec = (nDelay % USEC_PER_SEC) * 1000;
    clock_gettime(CLOCK_MONOTONIC, &next);
    int ret = 0;
    uint32_t  runCnt = 0;

    pSModule->mState = SR_OPEN;
    pSModule->newUpperMsg = false;
    //uint32_t maxE = 0;
    while (pSModule->mWorking)
    {
        next.tv_sec  += interval.tv_sec;
        next.tv_nsec += interval.tv_nsec;
        tsnorm(&next);
        if ((ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL)))
        {
            if (ret != EINTR)
                printf("clock_nanosleep failed. errno:");
            continue;
        }
 
        

        //处理上位机发送的消息
        if(pSModule->newUpperMsg)
        {
            if(pSModule->mState ==SR_RUN)
            {
                //handle_upper_msg(pSModule);
            }
            pSModule->newUpperMsg = false;
        }
        
        runCnt++;
    }
    sleep(1);
    //handle_close(pSModule);
    printf("serialThread exit\n\r");

    return NULL;
}
