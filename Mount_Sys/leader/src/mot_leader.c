/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 08:41:49
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-09 14:36:01
 * @FilePath: /Mount_Sys/leader/src/mot_leader.c
 * @Description: 系统中的超级线程模块，此模块负责创建所有线程，管理其他模块，周期的发送
消息通知其他模块。
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>


#include "mot_leader.h"
#include "mot_ctl.h"
#include "mot_share_messages.h"
#include "mot_share_ipc.h"
#include "mot_share_conf.h"
#include "mot_share_daemon.h"
#include "mot_master.h"
#include "mot_master_chassis.h"
#include "mot_master_serial.h"

const char  MN_MASTER_NAME[2][24] = {"MN_MASTER_ARMS","MN_MASTER_CHASSIS"};

//socket
int32_t  mRecSoket;

uint16_t mFrameId;

//thread parame
#define MODULE_NUMS 2

 //文件锁描述符
static int g_single_proc_inst_lock_fd = -1;

static int leaderWorking = 1;

ARMS_MASTER_THREAD_INFO mArmsModule;
CHASSIS_MASTER_THREAD_INFO mChassisModule;
//串口线程，主要负责读写底盘器件并交换数据
SERIAL_THREAD_INFO mSerialWorker; 



static void single_proc_inst_lockfile_cleanup(void);
static int32_t prepareEnv(void);
static int32_t initLeader(void);
static void signalHandle(int mSignal);

static int32_t leaderMainLoop();

static int32_t deInitSupr(void);
static int32_t startModules(void);


static int32_t sendNoticeToMyself(const int32_t msMsgId, const int32_t mNotice);
static int32_t sendNoticeToThread(const int32_t msMsgId, const int32_t mNotice);
static int32_t sendNoticeToSupr(const int32_t msMsgId, const int32_t mNotice);


//关闭文件锁
static void single_proc_inst_lockfile_cleanup(void)
{
    if (g_single_proc_inst_lock_fd != -1) {
        close(g_single_proc_inst_lock_fd);
        g_single_proc_inst_lock_fd = -1;
    }
}

 int32_t is_single_proc_inst_running(const char *process_name)
{
    char lock_file[128];
    snprintf(lock_file, sizeof(lock_file), "/var/tmp/%s.lock", process_name);

    g_single_proc_inst_lock_fd = open(lock_file, O_CREAT|O_RDWR, 0644);
    if (-1 == g_single_proc_inst_lock_fd) {
        fprintf(stderr, "Fail to open lock file(%s). Error: %s\n",
            lock_file, strerror(errno));
        return -1;
    }

    if (0 == flock(g_single_proc_inst_lock_fd, LOCK_EX | LOCK_NB)) {
        atexit(single_proc_inst_lockfile_cleanup);
        return 0;
    }

    close(g_single_proc_inst_lock_fd);
    g_single_proc_inst_lock_fd = -1;
    return -1;
}

static int32_t sendNoticeToThread(const int32_t msMsgId, const int32_t mNotice)
{
    int iret = 0;
    for(int i=0; i<MODULE_NUMS; i++)
    {
        MsgSendNotice(mRecSoket,get_mn_master_port(i), MN_ID_LEADER, MN_ID_LEADER, msMsgId, mNotice, 0);
    }
    return iret;
}

static int32_t sendNoticeToSupr(const int32_t msMsgId, const int32_t mNotice)
{
  int iret = 0;
  iret = MsgSendNotice(mRecSoket,get_mn_port(MN_ID_SUPR), MN_ID_LEADER, MN_ID_SUPR, msMsgId, mNotice, 0);
  return iret;
}

////////////////////////////////////////////////////////////////////////////////
///////external interface //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
static void signalHandle(int mSignal)
{
  sendNoticeToMyself(MSG_ID_MODULE_EXIT_ALL,0);
  printf("oops! stop!!!\n");
}

static int32_t sendNoticeToMyself(const int32_t msMsgId, const int32_t mNotice)
{
    int iret = 0;
    iret = MsgSendNotice(mRecSoket,get_mn_port(MN_ID_LEADER), MN_ID_LEADER, MN_ID_LEADER, msMsgId, mNotice, 0);
    return iret;
}

/******************************************************************************
* 功能：各个线程模块初始化，并且启动个各模块线程
* @return Descriptions
******************************************************************************/
static int32_t startModules(void) {

  // arms thread
  mArmsModule.mWorking = true;
  mArmsModule.mCpuAffinity  = CPU_MASTER;
  strcpy(mArmsModule.mThreadName, MN_MASTER_NAME[0]);
  mArmsModule.mState = M_STATE_INIT;
  mArmsModule.mthreadMasterId = 0;
  mArmsModule.masterId = 0;
  mArmsModule.slaves_size = 0;
  mArmsModule.mSerialWorker = &mSerialWorker;
  mArmsModule.mIdentifier = mFrameId;
  mArmsModule.pTChassis = &mChassisModule;
  usleep(10000);
  //机械臂主站
  mArmsModule.mPid  = hiCreateThread(MN_MASTER_NAME[0], master_arms_threadEntry,PRI_MASTER, &mArmsModule);

  // chassis thread
  mChassisModule.mWorking = true;
  mChassisModule.mCpuAffinity  = CPU_MASTER;
  strcpy(mChassisModule.mThreadName, MN_MASTER_NAME[1]);
  mChassisModule.mState = M_STATE_INIT;
  mChassisModule.mthreadMasterId = 1;
  mChassisModule.masterId = 1;
  //拷贝串口线程指针
  mChassisModule.mSerialWorker = &mSerialWorker;
  mChassisModule.mIdentifier = mFrameId;
  mChassisModule.mArmsModule = &mArmsModule;
  mChassisModule.ArmsReferId = 0;
  usleep(10000);
  //机械臂主站
  mChassisModule.mPid  = hiCreateThread(MN_MASTER_NAME[1], master_chassis_threadEntry,PRI_MASTER, &mChassisModule);

  //seral串口线程创建
  mSerialWorker.mWorking = 1;
  strcpy(mSerialWorker.mThreadName,"serials");
  mSerialWorker.pTModule = &mArmsModule;
  mSerialWorker.pTChassisModule = &mChassisModule;
  mSerialWorker.mPid = hiCreateThread("serials", serialThread,PRI_MASTER, &mSerialWorker);

  return 0;
}

/******************************************************************************
* 功能：初始化整个系统的运行环境
* @return Descriptions
******************************************************************************/
static int32_t prepareEnv(void) {
  int32_t iRet = 0;

  printf("leader APP STARTING\n");
  //TODO:: msg and msg q need refactor after first version.

  if(initLeader() < 0)
    return -1;

  iRet = startModules();
  
  return iRet;
}

/******************************************************************************
* 功能：初始化supr变量
* @return Descriptions
******************************************************************************/
static int32_t initLeader(void) 
{
  //need handler error case
  int32_t iRet = 0;

  signal(SIGINT, signalHandle);

  //创建soket
  mRecSoket = createSoket(get_mn_port(MN_ID_LEADER), MN_LocalIpV4Str, 0, 50000);

  //supr run in cpu0
  hiSetCpuAffinity(CPU_LEAD);
  hiSetThreadsched(pthread_self(), PRI_LEAD);

  //向supr注册msg队列
  pid_t mpid = getpid();
  iRet = MsgSendRegisterMsgQ(mRecSoket,get_mn_port(MN_ID_SUPR), MN_ID_LEADER, MN_ID_SUPR, MSG_ID_MODULE_REGISTER_MSG_Q_ACK, mpid);

  return iRet;
}


/******************************************************************************
* 功能：释放、销毁线程资源
* @return Descriptions
******************************************************************************/
static int32_t deInitSupr(void)
{
  int32_t iRet = 0;

  single_proc_inst_lockfile_cleanup();

  //删除创建的soket
  if(mRecSoket>0)
  {
      close(mRecSoket);
      mRecSoket = -1;
  }
  printf("*********** leader deInitSupr\n");

  return iRet;
}

 /******************************************************************************
* 功能：系统入口函数，main函数调用
* @return Descriptions
******************************************************************************/
int32_t leaderAppStartUp(const char** mSuprMsgK) {

  int32_t iRet = 0;
  //创建互斥文件锁，防止多个进程实例运行
  if(is_single_proc_inst_running("leader")<0)
  {
    printf("已经存在\n");
    return -1;
  }

  iRet = prepareEnv();
  if (0 == iRet) {
    printf("leader runing now! ctrl+c to endup\n");
    iRet = leaderMainLoop();
  }
    //销毁串口线程
  mSerialWorker.mWorking = false;

  pthread_join(mArmsModule.mPid, NULL);
  pthread_join(mChassisModule.mPid, NULL);
  pthread_join(mSerialWorker.mPid, NULL);
  printf("quit timer module\n");


  deInitSupr();

  printf("quit sysArms all modules\n");
  return iRet;
}

/******************************************************************************
* 功能：leader线程陷入函数，周期性的控制master等线程运行
* @return Descriptions
******************************************************************************/
static int32_t leaderMainLoop(){
  //TODU
  char mData[Msg_T_S];
  leaderWorking = 1;

  struct sockaddr_in  mPeerAddr;
  socklen_t mun = sizeof(mPeerAddr);

  static int recThreadCnt = 0;

  //static uint32_t runCnt = 0;

  while(leaderWorking)
  {
    if(recvfrom(mRecSoket, mData, Msg_T_S, 0, (struct sockaddr*)&(mPeerAddr), &mun)>0)
    {
      MsgHeader *mMsg = (MsgHeader *)mData;
      switch (mMsg->mMsgId)
      {
        case MSG_ID_MODULE_EXIT_ALL:
        {
          recThreadCnt = 0;
          sendNoticeToThread(MSG_ID_MODULE_EXIT_ALL,0);
          break;
        }
        
        case MSG_ID_THREAD_REQUEST_EXIT_ALL_ACK:
        {
          recThreadCnt++;
          if(recThreadCnt == MODULE_NUMS)
          {
              sendNoticeToSupr(MSG_ID_MODULE_EXIT_ALL_ACK, 0);
              leaderWorking = 0;
          }
          break;
        }  

        default:
        {
          break;
        }
      } //end switch
    }//end if

   

    //checkTimerWorking();
  }// end while


  return 0;
}