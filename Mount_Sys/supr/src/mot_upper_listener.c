/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 08:46:35
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-16 16:27:31
 * @FilePath: /Mount_Sys/supr/src/mot_upper_listener.c
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "mot_upper_listener.h"
#include "mot_share_messages.h"
#include "mot_ctl.h"
#include "mot_share_conf.h"
#include "mot_share_ipc.h"



static int32_t mRecUpperSoket = -1;
static int32_t mLocalSoket = -1;


socklen_t upperNum;
struct sockaddr_in  mUpperPeerAddr;

socklen_t localNum;
struct sockaddr_in  mLocalPeerAddr;


//上位机的ip和port
static char MN_UpperIpV4Str[16] = "127.0.0.1";
static uint16_t mUpperSoketPort = 10001;

uint64_t lastRecHeartTime = 0;
//心跳包标志位
static uint16_t mUpperTimeOut = 0;












static int32_t initListener(LISTENER_THREAD_INFO *pTModule);


static void LoopRecUpperMsg();
static uint64_t getCurrentTime();

static void LoopRecLocalMsg();

static int32_t  sendAllMasterMsg(const int32_t mMsgId, const int32_t mNotice, const int32_t mValue,const char* mData, const int32_t mDSize);


static int32_t  sendAllMasterMsg(const int32_t mMsgId, const int32_t mNotice, const int32_t mValue,const char* mData, const int32_t mDSize)
{
  int iret = 0;

  iret = MsgSendLongMsg(mLocalSoket, get_mn_master_port(MN_ID_MASTER_ARMS), MN_ID_SUPR, MN_ID_LEADER, mMsgId, mNotice, mValue, mData, mDSize);
  iret = MsgSendLongMsg(mLocalSoket, get_mn_master_port(MN_ID_MASTER_CHASSIS), MN_ID_SUPR, MN_ID_LEADER, mMsgId, mNotice, mValue, mData, mDSize);
  
  return iret;
}




/******************************************************************************
* 功能：初始化supr变量
* @return Descriptions
******************************************************************************/
static int32_t initListener(LISTENER_THREAD_INFO *pTModule) {
  int32_t iRet = 0;

  //创建soket.非阻塞模式
  char eno1_ip[IP_C_SIZE];
  //get_local_ip("eno1",eno1_ip);
  //get_local_ip("enx562b8d9d5872",eno1_ip);

  mRecUpperSoket = createSoket(get_port("LISTENER_TO_UPPER_PORT"), MN_LocalIpV4Str, 0, 0);

  if(mRecUpperSoket  < 0) {
      printf( "listener socket creat Failed\n");
      return -1;
  }

  //创建soket.非阻塞模式
  mLocalSoket = createSoket(get_port("LISTENER_LOCAL_PORT"), MN_LocalIpV4Str, 0, 0);

  if(mLocalSoket  < 0) {
      printf( "local socket creat Failed");
      return -1;
  }

  return iRet;
}


static uint64_t getCurrentTime()
{
  uint64_t currT = 0;
  struct timespec  nowTime;
  clock_gettime(CLOCK_MONOTONIC, &nowTime);

  currT = ((int64_t)(nowTime.tv_sec))*1000 + (nowTime.tv_nsec/1000000);

  return currT;
}


/******************************************************************************
* 功能：此函数销毁初始化模块，释放线程指针里的内容
* @param pTModule : pTModule是线程信息结构体指针，里边存储的线程运行期间用到的数据和交换通信数据
* @return Descriptions
******************************************************************************/
static int moduleEndUp(LISTENER_THREAD_INFO *pTModule)
{
  //删除创建的soket
  if(mRecUpperSoket>0)
  {
      close(mRecUpperSoket);
      mRecUpperSoket = -1;
  }

  if(mLocalSoket>0)
  {
      close(mLocalSoket);
      mLocalSoket = -1;
  }

  printf("%s leader endup\n", pTModule->mThreadName);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
///////external interface //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************
* 功能：线程入口函数，此模块的生命周期就在此函数中。
* @param pTModule : pTModule是线程信息指针，里边包含发送/接收消息，socket等信息
* @return Descriptions
******************************************************************************/
void* upper_threadEntry(void* pModule)
{
  LISTENER_THREAD_INFO * pTModule =(LISTENER_THREAD_INFO *) pModule;
  if(NULL == pTModule)
  {
    printf("null timer\n");
    return 0;
  }
  
  if(initListener(pTModule)<0)
    return 0; 

  //初始化完成
  pTModule->mInitOk = true;

  upperNum = sizeof(mUpperPeerAddr);
  localNum = sizeof(mLocalPeerAddr);
  uint32_t static runCnt = 0;

  while(pTModule->mWorking)
  {
      //由于接收上位机和本地消息的套接字设置非阻塞模式，必须要sleep函数，避免cpu一直运行
      usleep(2000);

      //循环接收上位机和本地的消息。
      LoopRecUpperMsg();

      //循环接收上位机和本地的消息。
      LoopRecLocalMsg();

      //心跳包判断是否断开,2s判断一次心跳包
      if(runCnt % 2000 == 0)
      {
        if(getCurrentTime()- lastRecHeartTime > 2000)
        {
          printf("***************%d abs(getCurrentTime()- lastRecHeartTime)>1000\n",runCnt);
          if(mUpperTimeOut < 3)
          {
            //销毁主站。
            //sendAllMasterMsg(CMD_DESTROY_MASTER,0,0,0,0);
            mUpperTimeOut++;
          }
        }else{
            mUpperTimeOut = 0;
        }
      }
      runCnt++;
  }
  sleep(1);
  moduleEndUp(pTModule);
}


/******************************************************************************
* 函数名称:  LoopRecUpperMsg
* 功能说明:  循环接收上位机消息
            该函数通过udp接收
* 输入参数:  pucByteArray: 掩码字节数组
            ucByteNum   : 掩码字节数组待转换的有效字节数目
            ucBaseVal   : 掩码字符串起始字节对应的数值
* 输出参数:  pStrSeq     ：掩码字符串，以','、'-'间隔
            形如0xD7(0b'11010111)  ---> "0-1,3,5-7"
* 返 回 值:  pStr        ：pStrSeq的指针备份，可用于strlen等链式表达式
* 用法示例:  INT8U aucByteArray[8] = {0xD7, 0x8F, 0xF5, 0x73};
            CHAR szSeq[64] = {0};
            ByteArray2StrSeq(aucByteArray, 4, 0, szSeq);
               ----> "0-1,3,5-8,12-19,21,23,25-27,30-31"
            memset(szSeq, 0, sizeof(szSeq));
            ByteArray2StrSeq(aucByteArray, 4, 1, szSeq);
               ----> "1-2,4,6-9,13-20,22,24,26-28,31-32"
* 注意事项:  因本函数内含strcat，故调用前应按需初始化pStrSeq
******************************************************************************/
static void LoopRecUpperMsg()
{
    char mUpperData[Msg_T_S];

    ssize_t recSize = 0;
    while((recSize=recvfrom(mRecUpperSoket, mUpperData, Msg_T_S, 0, (struct sockaddr*)&(mUpperPeerAddr), &upperNum))>0)
    {
      SUPR_R_MSG *mMsg = (SUPR_R_MSG *)mUpperData;
      switch (mMsg->mCmd)
      {
        //链接主站：
        case CMD_LINK:{
          uint16_t *pPort = (uint16_t*)mMsg->mData;
          mUpperSoketPort = *pPort;
          strcpy(MN_UpperIpV4Str, mMsg->mData+2);
          printf("ip: %s", MN_UpperIpV4Str);
          break;
        }
        //断开链接主站
        case CMD_UNLINK:{
          sendAllMasterMsg(CMD_DESTROY_MASTER,0,0,0,0);
          break;
        }

                //扫描从站
        case CMD_IGH_CREATE_MASTERS:
        case CMD_DESTROY_MASTER:
        case CMD_IGH_SCAN_SLAVES:
        {
          sendAllMasterMsg(mMsg->mCmd,0,0,0,0);
          break;
        }
       
        default:{
          break;
        }
      } //end switch
      lastRecHeartTime = getCurrentTime();
    }

  
}



static void LoopRecLocalMsg()
{
    char mLocalData[Msg_T_S];

    while(recvfrom(mLocalSoket, mLocalData, Msg_T_S, 0, (struct sockaddr*)&(mLocalPeerAddr), &localNum)>0)
    {
      MsgHeader *mMsg = (MsgHeader *)mLocalData;

      switch (mMsg->mMsgId)
      {
       
        default:
        {
          break;
        }
      } //end switch

    }
}


