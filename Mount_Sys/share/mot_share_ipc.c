/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 10:29:14
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-11 17:32:22
 * @FilePath: /Mount_Prj/Mount_Sys/share/mot_share_ipc.c
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "mot_share_ipc.h"
#include "mot_share_messages.h"



int32_t MsgMgrSendMsg(const int32_t mSenderSoket, const int mRecerPort, const void *ptr, size_t length)
{
  int32_t iRet = 0;
  //peer
  struct  sockaddr_in  mPeerAddr;
  bzero(&(mPeerAddr), sizeof(struct sockaddr_in));
  mPeerAddr.sin_family = AF_INET;
  mPeerAddr.sin_port = htons(mRecerPort);
  mPeerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  iRet = sendto(mSenderSoket, ptr, length, 0,(struct sockaddr *)&(mPeerAddr), sizeof(mPeerAddr));

  //printf("*********%d  %d %d\n",mSenderSoket,iRet, mRecerPort);
  return iRet;
}


void setFdNonblocking(int sockfd)
{
    int flag = fcntl(sockfd, F_GETFL, 0);
    if (flag < 0) {
        printf("setFdNonblocking fcntl F_GETFL fail");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0) {
        printf("setFdNonblocking fcntl F_SETFL fail");
    }
}

void setFdTimeout(int sockfd, const int mSec, const int mUsec)
{
  struct timeval timeout;
  timeout.tv_sec = mSec;//秒
  timeout.tv_usec = mUsec;//微秒
  
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    printf( "setsockopt failed:\n");
}

int32_t createSoket(uint32_t mMyPort,const char *mMyIpV4Str, int mWaitSec, int mWaitUsec) {
  int32_t iRet = 0;

  if((iRet = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      printf( "socket creat Failed");
      return -1;
  }

  if((mWaitSec <=0) && (mWaitUsec <= 0))
  {
      setFdNonblocking(iRet);
  } else
  {
      setFdTimeout(iRet, mWaitSec, mWaitUsec);
  }
  struct sockaddr_in  mMyAddr;
  //my
  bzero(&(mMyAddr), sizeof(mMyAddr));
  mMyAddr.sin_family = AF_INET;
  mMyAddr.sin_port = htons(mMyPort);
  mMyAddr.sin_addr.s_addr = inet_addr(mMyIpV4Str);

  if(bind(iRet, (struct sockaddr*)&(mMyAddr), sizeof(mMyAddr))<0)
  {
    printf("error= %d,err_str=%s,%s\n",errno,strerror(errno),mMyIpV4Str);
    return -1;
  }

  return iRet;
}

int32_t MsgSendNotice(const int32_t mSenderSoket, const int mRecerPort, const int32_t mSender, const int32_t mRecer, const int32_t mMsgId, const int32_t mNotice, const int32_t mValue)
{
  int32_t iRet = 0;
  MsgHeader mMsg;
  mMsg.mSender = mSender;
  mMsg.mRecer  = mRecer;
  mMsg.mMsgId  = mMsgId;
  mMsg.mNotice  = mNotice;
  mMsg.mValue1  = mValue;
  iRet = MsgMgrSendMsg(mSenderSoket,mRecerPort, (char*)&mMsg, sizeof(MsgHeader));
  return  iRet;
}


int32_t MsgSendRegisterMsgQ(const int32_t mSenderSoket, const int mRecerPort, const int32_t mSender, const int32_t mRecer, const int32_t mMsgId, const pid_t mPid)
{
  int32_t iRet = 0;
  MsgRegisterMsgQ mMsg;
  mMsg.mSender = mSender;
  mMsg.mRecer  = mRecer;
  mMsg.mMsgId  = mMsgId;
  mMsg.mPid  = mPid;
  iRet = MsgMgrSendMsg(mSenderSoket, mRecerPort, (char*)&mMsg, sizeof(MsgRegisterMsgQ));
  return  iRet;
}

int32_t MsgSendLongMsg(const int32_t mSenderSoket, const int mRecerPort, const int32_t mSender, const int32_t mRecer, const int32_t mMsgId,
                       const int32_t mNotice, const int32_t mValue, const char* mShortData, const int32_t mDSize)
{
  int32_t iRet = 0;
  MsgLongMsg mMsg;
  mMsg.mSender = mSender;
  mMsg.mRecer  = mRecer;
  mMsg.mMsgId  = mMsgId;
  mMsg.mNotice  = mNotice;
  mMsg.mValue1  = mValue;

  if(mDSize>0 && NULL != mShortData)
    memcpy(mMsg.mData, mShortData, mDSize);

  iRet = MsgMgrSendMsg(mSenderSoket, mRecerPort, (char*)&mMsg, sizeof(MsgLongMsg));
  return  iRet;
}
