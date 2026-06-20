/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 11:10:59
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-16 16:29:27
 * @FilePath: /Mount_Sys/share/mot_share_messages.h
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */
#ifndef SYS_ARMS_MESSAGES_HPP
#define SYS_ARMS_MESSAGES_HPP


#include <stdbool.h>

#include <pthread.h>

#pragma pack(1)

///////////////////////////////////////////////////////////////////////////////
#define Msg_T_S (2048)

//listener线程结构体信息 thread info
typedef struct
{
  bool              mWorking;
  char              mThreadName[15];
  bool              mInitOk;

  pthread_t         mthreadPid;

  //cpu mask
  uint8_t           mCpuAffinity;
  
} LISTENER_THREAD_INFO;


//msg header
typedef struct {
  int32_t mSender;
  int32_t mRecer;
  int32_t mMsgId;

  //可以作为msgId的子消息ID,或者通知
  int32_t mNotice;

  int32_t mValue1;
  int32_t mValue2;
  int32_t mCrc;
}MsgHeader;


//module send to supr
typedef struct  {
  int32_t mSender;
  int32_t mRecer;
  int32_t mMsgId;

  //可以作为msgId的子消息ID,或者通知
  int32_t mNotice;

  int32_t mValue1;
  int32_t mValue2;
  int32_t mCrc;

  pid_t mPid;
}MsgRegisterMsgQ;

//long msg.(***MsgLongMsg + MsgHeader < Msg_T_S***)
#define LONG_MSG_SIZE  2048
typedef struct  {
   int32_t mSender;
  int32_t mRecer;
  int32_t mMsgId;

  //可以作为msgId的子消息ID,或者通知
  int32_t mNotice;

  int32_t mValue1;
  int32_t mValue2;
  int32_t mCrc;
  
  char mData[LONG_MSG_SIZE];
}MsgLongMsg;


//定义底盘控制板发送消息
#define MOTEC_SIZE 9

//发送到底盘主站的控制命令消息
typedef struct
{
  //表示吊杆是否存在
  uint8_t   mModuleFlag[8];
  //吊杆X向控制命令
  float     mXMv[8];
  float     mYv[8];    
}ARMS_MAGIC_CTRL_V;
////////////////////////////////upper cmd msg//////////////////////////////
#define MSG_ID_BASE_FEATURE  0


#define MSG_ID_FIRE_IN_THE_HOLE          (MSG_ID_BASE_FEATURE + 5)

#define MSG_ID_MODULE_REGISTER_MSG_Q_ACK  (MSG_ID_BASE_FEATURE+10)
#define MSG_ID_MODULE_REQUEST_EXIT_ALL    (MSG_ID_BASE_FEATURE + 12)
#define MSG_ID_MODULE_EXIT_ALL            (MSG_ID_BASE_FEATURE + 13)
#define MSG_ID_MODULE_EXIT_ALL_ACK        (MSG_ID_BASE_FEATURE + 14)


//thread
#define MSG_ID_THREAD_START_ACK           (MSG_ID_BASE_FEATURE + 34)

#define MSG_ID_THREAD_REQUEST_EXIT_ALL_ACK (MSG_ID_BASE_FEATURE + 36)

//DEFINE recer msg
//定义机械臂、底盘控制板接收消息
typedef struct
{
  //cmd 控制命令
  int32_t   mCmd;

  //frame unique dev 0-3
  int32_t   mFrameId;

  //module id,0-6
  uint8_t   mModuleFlag[8];

  //数据长度
  uint16_t   mDataLength;

  //16位的crc累加校验
  uint16_t   mCRC;

  //可变长的数据,最短都是8位
  char       *mData;

} SUPR_R_MSG;


/******************************************upper udp msg start****************************************/


////////////////////////////////upper cmd msg//////////////////////////////
#define MSG_UPPER_CMD_DATA_SIZE 80
#define MSG_SERVER_ACK_DATA_SIZE 10240

//upper cmd mCmd
#define CMD_BASE    100
#define CMD_HEARTBEAT            (CMD_BASE + 1)
#define CMD_LINK                 (CMD_BASE + 2)
#define CMD_UNLINK               (CMD_BASE + 3)

#define CMD_IGH_CREATE_MASTERS   (CMD_BASE + 5)
#define CMD_IGH_SCAN_SLAVES      (CMD_BASE + 6)


#define CMD_DESTROY_MASTER       (CMD_BASE + 19)

#pragma pack()



 #endif