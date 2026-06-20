/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 10:26:50
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-11 17:32:03
 * @FilePath: /Mount_Prj/Mount_Sys/share/mot_share_ipc.h
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

 #ifndef MOT_SHARE_IPC_H
 #define MOT_SHARE_IPC_H
 
#include <stdint.h>
#include <fcntl.h>

///////////////////////////////////////////////////////////////////////////////
int32_t createSoket(uint32_t mMyPort,const char *mMyIpV4Str, int mWaitSec, int mWaitUsec);
void setFdNonblocking(int sockfd);
void setFdTimeout(int sockfd, const int mSec, const int mUsec);
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
int32_t MsgSendNotice(const int32_t mSenderSoket, const int mRecerPort, const int32_t mSender, const int32_t mRecer, const int32_t mMsgId, const int32_t mNotice, const int32_t mValue);
int32_t MsgSendRegisterMsgQ(const int32_t mSenderSoket, const int mRecerPort, const int32_t mSender, const int32_t mRecer, const int32_t mMsgId, const pid_t mPid);
int32_t MsgMgrSendMsg(const int32_t mSenderSoket, const int mRecerPort, const void *ptr, size_t length);
int32_t MsgSendLongMsg(const int32_t mSenderSoket, const int mRecerPort, const int32_t mSender, const int32_t mRecer, const int32_t mMsgId,
                       const int32_t mNotice, const int32_t mValue, const char* mShortData, const int32_t mDSize);

///////////////////////////////////////////////////////////////////////////////

 #endif
