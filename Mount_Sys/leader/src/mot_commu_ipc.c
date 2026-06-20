/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 08:43:37
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-09 13:58:29
 * @FilePath: /Mount_Sys/leader/src/mot_commu_ipc.c
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

#include "mot_commu_ipc.h"
#include "mot_share_conf.h"
#include "mot_share_ipc.h"

int32_t sendNoticeToLeader(int32_t mSocket,const int32_t mMsgId, const int32_t mNotice, const int32_t mValue)
{
  int iRet = 0;

  if(mSocket<0)
    return -1;

  iRet = MsgSendNotice(mSocket, get_mn_port(MN_ID_LEADER),MN_ID_LEADER, MN_ID_LEADER,mMsgId, mNotice, mValue);

  return iRet;
}