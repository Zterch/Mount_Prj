/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 08:46:20
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-10 13:55:17
 * @FilePath: /Mount_Sys/supr/src/mot_supr.c
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdarg.h>
#include <glob.h>

#include "mot_supr.h"
#include "mot_ctl.h"
#include "mot_share_ipc.h"
#include "mot_share_conf.h"
#include "mot_share_messages.h"
#include "mot_share_daemon.h"
#include "mot_upper_listener.h"


//定义线程模块运行的状态
typedef enum
{
  M_STATE_START = 1,
  M_STATE_SDLE ,
  M_STATE_FIREUP,
  M_STATE_RUNNING,
  M_STATE_STOP,
  M_STATE_QUIT,
} M_RUN_STATE;

//模块结构体，线程名字，入口函数，pid
typedef struct _MODULEINFOS
{
  char    mName[64];
  int32_t mPri;
  pid_t   mPid;
  M_RUN_STATE mState;
} MODULEINFOS;

//cpu latency 禁止cpu进入节能模式
static int latency_target_fd = -1;
static int32_t latency_target_value = 0;

//arms, threads and supr
MODULEINFOS gHiMInfo[MODULES_NUMS+1] = {0};


//socket
int32_t mSurpSoket;
static int suprWorking = 1;

//thread info
LISTENER_THREAD_INFO mUpperListener;
 
static int32_t prepareEnv(void);
static int32_t initSupr(void);
static int32_t startModules(void);
static int32_t suprMainLoop();
static void signalHandle(int mSignal);


static void set_latency_target(void);
static int is_xenomai(void)
{
    /* Xenomai 内核会在 /proc 下创建 version 文件 */
    return (access("/proc/xenomai/version", F_OK) == 0);
}

static int32_t triggerSuprChangeState(const int32_t mMsgId);
static int32_t registMsgQForModules(int mModuleId, pid_t mPid);
static int32_t checkModuleStates(M_RUN_STATE mState);
static int32_t changeModuleStates(int mModuleId, M_RUN_STATE mState);


static int32_t  noticeAllModules(const int32_t mMsgId);

static void PrintfLog(const char *fm, ...);

static int32_t deInitSupr(void);



static int32_t registMsgQForModules(int mModuleId, pid_t mPid)
{
  int iRet = 0;

  gHiMInfo[mModuleId].mPid  = mPid;
  gHiMInfo[mModuleId].mState = M_STATE_START;

  //检查是否都注册完毕
  iRet = checkModuleStates(M_STATE_START);

  printf("registMsgQForModules mID:%d iRet:%d\n",mModuleId,iRet);
  //进入init状态
  if(iRet == MODULES_NUMS)
  {
    triggerSuprChangeState(MSG_ID_FIRE_IN_THE_HOLE);
  }
  return iRet;
}

static void PrintfLog(const char *fm, ...)
{
  char mStr[256] = {0};
  va_list args;
  va_start(args, fm);
  vsnprintf(mStr, 255, fm, args);
  va_end( args );

  struct timespec  logtime;
  clock_gettime(CLOCK_MONOTONIC, &logtime);
  char mLogStr[300] = {0};
  sprintf(mLogStr, "%ld %ld %s",logtime.tv_sec,logtime.tv_nsec/1000,mStr);

  //BASE::MsgSendPriLog(mSurpSoket, BASE::MN_PORTS[BASE::MN_ID_LOGER],BASE::MN_ID_SUPR, BASE::MN_ID_LOGER,BASE::MSG_ID_LOGER_SAVE_LOG, mLogStr);
}

static int32_t triggerSuprChangeState(const int32_t mMsgId)
{
  int iRet = 0;
  iRet = MsgSendNotice(mSurpSoket,get_supr_port(), MN_ID_SUPR, MN_ID_SUPR, mMsgId, 0, 0);
  return iRet;
}

static int32_t  noticeAllModules(const int32_t mMsgId)
{
  int iret = 0;
  //const int mMsgQ, const int32_t mSender, const int32_t mRecer, const int32_t mMsgId, const int32_t mNotice, const int32_t mValue
  for (int i=1;i<MODULES_NUMS+1;++i)
  {
    MsgSendNotice(mSurpSoket,get_mn_port(i), MN_ID_SUPR, i, mMsgId, 0, 0);
  }
  return iret;
}


static int32_t checkModuleStates(M_RUN_STATE mState)
{
  int32_t iRet = 0;

  for (int i=1; i<MODULES_NUMS+1; ++i) {
    if(gHiMInfo[i].mState == mState)
      iRet++;
  }
  return iRet;
}

static int32_t changeModuleStates(int mModuleId, M_RUN_STATE mState)
{
  int32_t iRet = 0;
  gHiMInfo[mModuleId].mState = mState;

  return iRet;
}


/******************************************************************************
* 功能：设置电源管理策略，此模块比避免cpu进入节能模式，影响定时器精度
* @return Descriptions
******************************************************************************/
static void set_latency_target(void)
{
    struct stat s;
    int err;

    /* 1. Xenomai 环境：无需任何设置，直接返回 */
    if (is_xenomai()) {
        /* 如果想保留一条提示，可以取消注释下行，但这不是警告 */
        /* printf("# Running on Xenomai, skip cpu_dma_latency setting\n"); */
        return;
    }

    /* 2. 普通 Linux 环境：尝试标准的 /dev/cpu_dma_latency */
    errno = 0;
    err = stat("/dev/cpu_dma_latency", &s);
    if (err == -1) {
        /* 文件不存在，静默返回（不再打印警告） */
        return;
    }

    errno = 0;
    latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
    if (latency_target_fd == -1) {
        /* 打开失败，静默返回 */
        return;
    }

    errno = 0;
    err = write(latency_target_fd, &latency_target_value, 4);
    if (err < 4) {
        /* 写入失败，关闭文件并返回 */
        close(latency_target_fd);
        latency_target_fd = -1;
        return;
    }

    /* 成功时打印信息（可选） */
    printf("# /dev/cpu_dma_latency set to %d\n", latency_target_value);
}

/******************************************************************************
* 功能：初始化整个系统的运行环境
* @return Descriptions
******************************************************************************/
static int32_t prepareEnv(void) {
  int32_t iRet = 0;
  printf("supr ARMS APP STARTING\n");

  set_latency_target();
  //init_params();
  //TODO:: msg and msg q need refactor after first version.
  if(initSupr() < 0)
    return -1;

  iRet = startModules();

  return iRet;
}

/******************************************************************************
* 功能：初始化supr变量
* @return Descriptions
******************************************************************************/
static int32_t initSupr(void) {
  //need handler error case
  int32_t iRet = 0;

  //创建soket
  mSurpSoket = createSoket(get_mn_port(MN_ID_SUPR), MN_LocalIpV4Str, 0, 5000);

  printf("createSoket:%d\n",mSurpSoket);

  iRet = mSurpSoket;
  //TUDO
  signal(SIGINT, signalHandle);

  //supr run in cpu0
  hiSetCpuAffinity(CPU_SUPR);
  hiSetThreadsched(pthread_self(), PRI_SUPR);

  return iRet;
}


/******************************************************************************
* 功能：各个线程模块初始化，并且启动个各模块线程
* @return Descriptions
******************************************************************************/
static int32_t startModules(void) {

  //上位机线程 thread
  mUpperListener.mWorking = true;
  mUpperListener.mInitOk  =false;
  mUpperListener.mCpuAffinity  = 1;
  strcpy(mUpperListener.mThreadName, "upper");
  mUpperListener.mthreadPid = hiCreateThread(mUpperListener.mThreadName, upper_threadEntry,50, &mUpperListener);
  
  //等待upper初始化完成，supr才进入循环
  int32_t waitTime = 0;
  while (!mUpperListener.mInitOk)
  {
    printf("supr waiting upper init...\n");
    sleep(1);
    waitTime++;
    if(waitTime>10)
      return -1;
  }

  
  return 0;
}


/******************************************************************************
* 功能：supr线程陷入函数，周期性的控制leader等线程运行
* @return Descriptions
******************************************************************************/
static int32_t suprMainLoop(){
  //TODU
  MsgHeader *mMsg;
  char mData[Msg_T_S];
  suprWorking = 1;
  mMsg = (MsgHeader *)mData;
  struct sockaddr_in  mPeerAddr;
  socklen_t mun = sizeof(mPeerAddr);

  while(suprWorking)
  {
      struct timespec  logS,logE;
      clock_gettime(CLOCK_MONOTONIC, &logS);
      //printf("suprMainLoop (%d:%d)\n\r",logS.tv_sec,logS.tv_nsec/1000);

    if((recvfrom(mSurpSoket, mData, Msg_T_S, 0, (struct sockaddr*)&(mPeerAddr), &mun)>0) && (MN_ID_SUPR == mMsg->mRecer))
    {
      switch (mMsg->mMsgId)
      {
        case MSG_ID_MODULE_REGISTER_MSG_Q_ACK:
        {
          MsgRegisterMsgQ *mRegisMsg = (MsgRegisterMsgQ *)mMsg;
          registMsgQForModules(mRegisMsg->mSender, mRegisMsg->mPid);
          printf("register ok\n");
          break;
        }
        case MSG_ID_MODULE_REQUEST_EXIT_ALL:
        {
          PrintfLog("supr send exit\n");
          noticeAllModules(MSG_ID_MODULE_EXIT_ALL);
          break;
        }
        case MSG_ID_MODULE_EXIT_ALL_ACK:
        {
          changeModuleStates(mMsg->mSender, M_STATE_QUIT);
          if(checkModuleStates(M_STATE_QUIT) == MODULES_NUMS)
          {
            printf("all modules quited\n");
            suprWorking = 0;
          }
          break;
        }
        default:
        {
          break;
        }
      } //end switch

    }//end if

  }// end while

  mUpperListener.mWorking = 0;

} //end suprMainLoop


/******************************************************************************
* 功能：按键中断函数，ctrl+c结束系统运行
* @return Descriptions
******************************************************************************/
//TUDO
static void signalHandle(int mSignal)
{
  //mSysState = BASE::M_STATE_STOP;
  //mSysState = BASE::M_STATE_QUIT;
  //suprWorking = 0;
  triggerSuprChangeState(MSG_ID_MODULE_REQUEST_EXIT_ALL);
  printf("oops! stop!!!\n");
}

/******************************************************************************
* 功能：释放、销毁线程资源
* @return Descriptions
******************************************************************************/
static int32_t deInitSupr(void)
{
  int32_t iRet = 0;
  /* close the latency_target_fd if it's open */
  if (latency_target_fd >= 0)
    close(latency_target_fd);

  if(mSurpSoket>0)
  {
      close(mSurpSoket);
      mSurpSoket = -1;
  }
  printf("**********surp deInitSupr \n");

  return iRet;
}

/******************************************************************************
* 功能：系统入口函数，main函数调用
* @return Descriptions
******************************************************************************/

int32_t suprAppStartUp()
{
    int32_t iRet = 0;
    //父进程
    iRet = prepareEnv();

    get_mn_master_port(0);

    if (0 == iRet) {
    printf("supr runing now! ctrl+c to endup\n");
    iRet = suprMainLoop();

    pthread_join(mUpperListener.mthreadPid, NULL);

  }

  deInitSupr();
  printf("quit sysArms all modules,%d\n", iRet);
  return iRet;
    
}