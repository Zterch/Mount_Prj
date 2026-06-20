/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-06-05 14:20:41
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-16 16:31:36
 * @FilePath: /Mount_Sys/leader/src/mot_master_chassis.c
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mot_master_chassis.h"
#include "mot_leader_defs.h"
#include "mot_share_daemon.h"
#include "mot_share_ipc.h"
#include "mot_ctl.h"
#include "mot_commu_ipc.h"

static int initchassis(CHASSIS_MASTER_THREAD_INFO *pTModule);


static void  cyc_rotate_pos_upper_cmd(CHASSIS_MASTER_THREAD_INFO *pTModule);
static void  cyc_vel_upper_cmd(CHASSIS_MASTER_THREAD_INFO *pTModule);
static void  filter_smooth(CHASSIS_MASTER_THREAD_INFO *pTModule);

static int   destroyChassisMaster(CHASSIS_MASTER_THREAD_INFO *pTModule);
static void  destroy_slaves_inf(EK1100_slaves_inf *slaves_inf_list);
static int  createMaster(int mid,ec_master_t **pms,ec_domain_t **pdi);


/**************************************************master bus***************************************************/
void* ethercatChassisThread(void* );


static int  createMaster(int mid,ec_master_t **pms,ec_domain_t **pdi)
{
    int iRet = -1;
    if(*pms == NULL)
    {
        *pms = ecrt_request_master(mid);
        if (*pms==NULL)
        {
            printf("#########:create master%d failed\n\r",mid);
            iRet = -1;
        }else{
            printf("#########:create master%d OK\n\r",mid);
            *pdi = ecrt_master_create_domain(*pms);
            printf("#########:create master%d domain1 OK\n\r",mid);
            iRet = 0;
        }
    }
    return iRet;
}

static int initchassis(CHASSIS_MASTER_THREAD_INFO *pTModule)
{
  //创建soket.*非阻塞模式
  pTModule->mSoket = createSoket(get_mn_master_port(pTModule->mthreadMasterId), MN_LocalIpV4Str, 0, 0);

  pTModule->masterActivated=false;
  pTModule->mEtherWorker.mWorking = 1;
  strcpy(pTModule->mEtherWorker.mThreadName,"ethercatIgh");
  pTModule->mEtherWorker.pTModule = pTModule;
  pTModule->mEtherWorker.mPid = hiCreateThread("EtherIGH", ethercatChassisThread,PRI_MASTER, &pTModule->mEtherWorker);
  memset(pTModule->upper_target_v_Flag,0,sizeof(int32_t)*5);
  memset(pTModule->upper_target_p_Flag,0,sizeof(int32_t)*4);
  memset(pTModule->velocity_smooth,0,sizeof(int32_t)*MOTEC_SIZE*SMOOTH_SIZE);
  memset(pTModule->tar_vel_cmd,0,sizeof(int32_t)*MOTEC_SIZE);

  return 0;
}

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

static void  cyc_rotate_pos_upper_cmd(CHASSIS_MASTER_THREAD_INFO *pTModule)
{
    /**
     * 200HZ 运行周期
     */
    // ARMS_MASTER_THREAD_INFO * pArms = pTModule->mArmsModule;
    // for (int i = 0; i < 4; i++)
    // {
    //     if(pTModule->upper_target_p_Flag[i])
    //     {
    //         //V_cmd = Kp*(S_obj-S_now) + Kd(^S_obj-^S_now)
    //         float cmdSpeed;
    //         float kp = 3,kd = 0.1;
    //         cmdSpeed=kp*(pTModule->upper_target_pos[i]-pTModule->r_pos[i])-kd*(pTModule->r_speed[i]);

    //         //限制速度
    //         if(cmdSpeed>30)
    //             cmdSpeed = 30;
    //         if(cmdSpeed<-30)
    //             cmdSpeed = -30;

    //         //判断是否都达到目标速度
    //         if(fabs(pTModule->r_pos[i]-pTModule->upper_target_pos[i]) < 0.1)
    //             cmdSpeed = 0;
    //         chassisSetIRSpeed(pTModule,i,cmdSpeed);
    //         //到达位置
    //         if(fabs(pTModule->r_pos[i]-pTModule->upper_target_pos[i]) < 0.1)
    //         {
    //             pTModule->upper_target_p_Flag[i] = 0;
    //         }
    //     }
    // }
}

static void cyc_vel_upper_cmd(CHASSIS_MASTER_THREAD_INFO *pTModule)
{
    /**
     * 200HZ 运行周期
     */
    // for (int i = 0; i < 4; i++)
    // {
    //     if(pTModule->upper_target_v_Flag[i])
    //     {
    //         //V_cmd = V_now + a*dt
    //         float cmdSpeed[2];
    //         int cmd_target_ok = 0;
    //         for (int j = 0; j < 2; j++)
    //         {
    //             cmdSpeed[j] = pTModule->upper_last_v_cmd[i][j] + pTModule->upper_acc[i][j]*0.005;
    //             if(pTModule->upper_acc[i][j]>0 && cmdSpeed[j]>=pTModule->upper_target_velocity[i][j])
    //             {
    //                 cmdSpeed[j]=pTModule->upper_target_velocity[i][j];
    //                 pTModule->upper_acc[i][j] = 0;
    //                 cmd_target_ok++;
    //             }else if(pTModule->upper_acc[i][j]<0 && cmdSpeed[j]<=pTModule->upper_target_velocity[i][j])
    //             {
    //                 cmdSpeed[j]=pTModule->upper_target_velocity[i][j];
    //                 pTModule->upper_acc[i][j] = 0;
    //                 cmd_target_ok++;
    //             }else if(fabs(pTModule->upper_acc[i][j]) < 1e-7)
    //             {
    //                 cmd_target_ok++;
    //             }
    //         }

    //         //判断是否都达到目标速度
    //         if(
    //             fabs(cmdSpeed[0]-pTModule->upper_target_velocity[i][0]) < 1 &&
    //             fabs(cmdSpeed[1]-pTModule->upper_target_velocity[i][1]) < 1 &&
    //             cmd_target_ok >= 2
    //         )
    //         {
    //             pTModule->upper_target_v_Flag[i] = 0;
    //             cmdSpeed[0] = pTModule->upper_target_velocity[i][0];
    //             cmdSpeed[1] = pTModule->upper_target_velocity[i][1];
    //         }

    //         for (int j = 0; j < 2; j++)
    //         {
    //             pTModule->upper_last_v_cmd[i][j] = cmdSpeed[j];
    //         }
    //         chassisSetIWSpeed(pTModule,i,cmdSpeed[0]);
    //         chassisSetIRSpeed(pTModule,i,cmdSpeed[1]);
    //     }
    // }
}

static void  filter_smooth(CHASSIS_MASTER_THREAD_INFO *pTModule)
{
    //指令滤波
    // for (int  i = 0; i < 8; i++)
    // {
    //     for (int k = 0; k < SMOOTH_SIZE-1; k++)
    //     {
    //         pTModule->velocity_smooth[i][k] = pTModule->velocity_smooth[i][k+1];
    //     } 
    //     pTModule->velocity_smooth[i][SMOOTH_SIZE-1] = pTModule->tar_vel_cmd[i];

    //     int32_t smoothV = 0;
    //     for (int k = 0; k < SMOOTH_SIZE; k++)
    //     {
    //         smoothV += pTModule->velocity_smooth[i][k];
    //     } 
    //     pTModule->motors[i].target_velocity = (int32_t)(smoothV/SMOOTH_SIZE);
    // }
}

static void  destroy_slaves_inf(EK1100_slaves_inf *slaves_inf_list)
{
    if(slaves_inf_list->childSlaveSize >0)
    {   
        EK1100_slaves_inf *i = NULL;
        EK1100_slaves_inf *tmp = NULL;
        list_for_each_entry_safe(i, tmp, &slaves_inf_list->list, list) 
        {
            list_del(&i->list);
            free(i);
        }
        slaves_inf_list->childSlaveSize = 0;
    }
}

static int  destroyChassisMaster(CHASSIS_MASTER_THREAD_INFO *pTModule)
{
    //伺服运行过程中，急停电机
    if (pTModule->masterActivated)
    {
        pTModule->arm_fsa = S_HAIT;
        usleep(2000);
        pTModule->masterActivated = false;
    }
    
    //销毁master
    if(pTModule->master != NULL)
    {
        ecrt_release_master(pTModule->master);
        pTModule->master = NULL;
    }
    //销毁从站信息
    destroy_slaves_inf(&pTModule->slaves_inf);
}

static void  chassisLoopRecLocalMsg(CHASSIS_MASTER_THREAD_INFO *pTModule)
{
    struct sockaddr_in  mPeerAddr;
    socklen_t mun = sizeof(mPeerAddr);
    char mLocalData[Msg_T_S];

    if(recvfrom(pTModule->mSoket, mLocalData, Msg_T_S, 0, (struct sockaddr*)&(mPeerAddr), &mun)>0)
    {
      MsgHeader *mMsg = (MsgHeader *)mLocalData;
      //MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
      switch (mMsg->mMsgId)
      {
        // case MSG_ID_FIRE_IN_THE_HOLE:
        // {
        //   pTModule->mState = M_STATE_RUN;
        //   printf("BASE::MSG_ID_FIRE_IN_THE_HOLE\n");
        //   break;
        // }
        case MSG_ID_MODULE_EXIT_ALL:
        {
          printf("BASE::MSG_ID_MODULE_EXIT_ALL--master chassis\n");
          pTModule->mState = M_STATE_QUIT;

          pTModule->mWorking = false;

          //ethercat线程销毁
          pTModule->mEtherWorker.mWorking = false;
          //pTModule->mMagicWorker[i].mWorking = false;
          pthread_join(pTModule->mEtherWorker.mPid, NULL);

          //发送到leader 已经退出
          sendNoticeToLeader(pTModule->mSoket,MSG_ID_THREAD_REQUEST_EXIT_ALL_ACK, 0, 0);
          printf("timer endup\n");
          break;
        }
        case CMD_IGH_CREATE_MASTERS:
        {
          createMaster(pTModule->masterId,&pTModule->master,&pTModule->domain1);
          break;
        }
        case CMD_DESTROY_MASTER:
        {
          destroyChassisMaster(pTModule);
          break;
        }
        // case CMD_IGH_SCAN_SLAVES:
        // {
        //   scanChassisSlaves(pTModule);
        //   break;
        // }
        // case CMD_IGH_CONF_SERVO_PDOS:
        // {
        //   confChassisSlavesPdos(pTModule);
        //   break;
        // }
        // case CMD_IGH_START_OP:
        // {
        //   activeChassisMater(pTModule);
        //   break;
        // }
        // case CMD_MASTER_OP_KEYDOWN:
        // {
        //   activeChassisMaterKeydown(pTModule);
        //   break;
        // }
        // case CMD_IGH_FSA_SHUTDOWN:
        // {
        //     pTModule->arm_fsa = S_SHUTDOWN;
        //     break;
        // }
        // case CMD_IGH_FSA_SWITH_ON:
        // {
        //     pTModule->arm_fsa = S_SWITCH_ON;
        //     break;
        // }
        // case CMD_IGH_FSA_ENOP:
        // {
        //     pTModule->arm_fsa = S_ENABLE_OP;
        //     break;
        // }
        // case CMD_IGH_FSA_HALT:
        // {
        //     pTModule->arm_fsa = S_HAIT;
        //     break;
        // }  
        // case CMD_IGH_CLEAR_ERROR:
        // {
        //     pTModule->arm_fsa = S_CLEAR_ERROR;
        //     break;
        // }
        // case MSG_ID_ARMS_MAGIC_CTRL_MSG:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     ARMS_MAGIC_CTRL_V *smsg =  (ARMS_MAGIC_CTRL_V *)mLMsg->mData;
        //     memcpy(&pTModule->mArmsMagicCtrl,mLMsg->mData,sizeof(ARMS_MAGIC_CTRL_V));
        //     break;
        // } 
        // case CMD_CHASSIS_STOP:
        // {
        //     memset(pTModule->upper_target_v_Flag,0,sizeof(int32_t)*5);
        //     for (int i = 0; i < 4; i++)
        //     {
        //         //位置控制取消,启用速度控制
        //         pTModule->upper_target_p_Flag[i] = 0;
        //         //减速停止
        //         handleStartSlight(pTModule,i,0,0);
        //     }
        //     break; 
        // }
        // case CMD_IGH_CHASSIS_VEL:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     SUPR_R_MSG *smsg =  (SUPR_R_MSG *)mLMsg->mData;
        //     float *pVel = (float*)smsg->mData;
        //     memset(pTModule->upper_target_v_Flag,0,sizeof(int32_t)*5);
        //     for (int i = 0; i < 4; i++)
        //     {
        //         if(smsg->mModuleFlag[i])
        //         {
        //             //轴安装顺序 Y XM XS Z
        //             handleStartSlight(pTModule,i,pVel[0],pVel[1]);
        //         }
        //     }
        //     //开启顶升气缸。不使用缓坡加减速
        //     if(smsg->mModuleFlag[4])
        //     {
        //         chassisSetUpSpeed(pTModule,pVel[0]);
        //     }
        //     break;
        // }    
        // case CMD_CHASSIS_WHEEL_AGNGLE:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     SUPR_R_MSG *smsg =  (SUPR_R_MSG *)mLMsg->mData;
        //     float *pPos = (float*)smsg->mData;
        //     for (int i = 0; i < 4; i++)
        //     {
        //         if(smsg->mModuleFlag[i])
        //         {
        //             //轴安装顺序 Y XM XS Z
        //             chassisSetIRotatePos(pTModule,i,pPos[i]);
        //         }
        //     }
        //     break;
        // }   
        // case CMD_CHASSIS_MOVE:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     SUPR_R_MSG *smsg =  (SUPR_R_MSG *)mLMsg->mData;
        //     float *pPos = (float*)smsg->mData;
        //     //横向单位m/s  纵向单位 m/s 旋转单位rad/s 
        //     chassisMove(pTModule,pPos[0]/1000.0,pPos[1]/1000.0,pPos[2]*3.14159/180.0);
        //     break;
        // } 
        // case CMD_VALVE_CHASSIS24_ON:
        // case CMD_VALVE_CHASSIS24_OFF:
        // case CMD_VALVE_CHASSIS48_ON:
        // case CMD_VALVE_CHASSIS48_OFF:
        // case CMD_VALVE_CHASSIS_GAS_ON:
        // case CMD_VALVE_CHASSIS_GAS_OFF:
        // case CMD_VALVE_CHASSIS_BRK_ON:
        // case CMD_VALVE_CHASSIS_BRK_OFF:
        // {
        //   MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //   memcpy(&pTModule->mSerialWorker->upperMsg,mLMsg->mData,sizeof(SUPR_R_MSG));
        //   pTModule->mSerialWorker->newUpperMsg = true;
        //   break;
        // }
        // case CMD_PROPORTIONAL_VALVE:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     SUPR_R_MSG *pmsg = (SUPR_R_MSG *)mLMsg->mData;
        //     proportional_value *pv =  (proportional_value *)pmsg->mData;
        //     setChassisProportional(pTModule,pv->pvid,pv->value);
        //     break;
        // }
        // case CMD_PROPORTIONAL_VALVES:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     SUPR_R_MSG *pmsg = (SUPR_R_MSG *)mLMsg->mData;
        //     int pvs = *(int *)pmsg->mData;
        //     proportional_value *pv =  (proportional_value *)(pmsg->mData+4);
        //     setChassisProportionals(pTModule,pv,pvs);
        //     break;
        // }
        // case CMD_AUTO_FOLLOW:
        // case CMD_AUTO_FOLLOW_CHASSIS:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     chassisMagicStateChange(pTModule,R_FOLLOW);
        //     break;
        // }
        // case CMD_ALL_STOP:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     chassisMagicStateChange(pTModule,R_STOP);
        //     break;
        // }
        // case CMD_CHASSIS_MAGIC_ENABLE:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     SUPR_R_MSG *pmsg = (SUPR_R_MSG *)mLMsg->mData;
        //     uint8_t *penab = (uint8_t*)pmsg->mData;
        //     pTModule->enable_flag = penab[0];
        //     break;
        // }
        // case CMD_AUTO_FOLLOW_VERTICAL:
        // {
        //     MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
        //     chassisMagicStateChange(pTModule,R_FOLLOW_VERTICAL);
        //     break;
        // }
        default:
        {
          break;
        }
      } //end switch

    }
}



 void* master_chassis_threadEntry(void* pModule)
{
  CHASSIS_MASTER_THREAD_INFO *pTModule = (CHASSIS_MASTER_THREAD_INFO *) pModule;

  if(NULL == pTModule)
  {
    printf("null timer\n");
    return 0;
  }

  initchassis(pTModule);

  unsigned int nDelay = 1000;        /* usec */
  //timer 定时器
  struct timespec  next, interval;
  interval.tv_sec  = nDelay / USEC_PER_SEC;
  interval.tv_nsec = (nDelay % USEC_PER_SEC) * 1000;
  clock_gettime(CLOCK_MONOTONIC, &next);

  printf("%s running!threadEntry\n",pTModule->mThreadName);
  sendNoticeToLeader(pTModule->mSoket,MSG_ID_THREAD_START_ACK, 0, 0);
  
  memset(pTModule->upper_target_v_Flag,0,sizeof(float)*5);

  int ret = 0;
  uint32_t runCnt = 0;
  pTModule->mRunState = R_NULL;
  while(pTModule->mWorking)
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

    runCnt++;
    //200HZ rec local msg
    if((runCnt) % 5==0)
    {
        chassisLoopRecLocalMsg(pTModule);

        //循环2s的增加速度到目标值
        cyc_vel_upper_cmd(pTModule);

        //位置控制
        cyc_rotate_pos_upper_cmd(pTModule);

        //指令滤波
        filter_smooth(pTModule);
    }

    if((runCnt%50) == 0)
    {
        //chassis_magic_run(pTModule);
    }

    //20HZ发送底盘舵轮控制数据
    if((runCnt) % 50==0)
    {
        //主站激活之后才发送消息到上位机
        if(pTModule->masterActivated)
        {
            //packChassis_S_MSG(pTModule);

            

        }
    }

    //2HZ发送底盘采集的传感器数据
    if((runCnt) % 500==0)
    {
        //packChassis_sensor_S_MSG(pTModule);
    }

    if((runCnt) % 99==0)
    {
        //主站激活之后才发送消息到上位机
        if(pTModule->masterActivated)
        {
            //packM_S_master_run_inf(pTModule);
        }
    }

  } 

  sleep(1);
  //销毁master
  if(pTModule->master != NULL)
  {
      ecrt_release_master(pTModule->master);
      pTModule->master = NULL;
  }
  if(pTModule->mSoket>0)
  {
    close(pTModule->mSoket);
    pTModule->mSoket = -1;
  }

  return NULL;
}

static void check_chassis_master_state(CHASSIS_MASTER_THREAD_INFO *pTModule)
{
    // ec_master_state_t ms;
    // ecrt_master_state(pTModule->master, &ms);
    // if (ms.slaves_responding != pTModule->master_state.slaves_responding)
    // {
    //     printf("%u slave(s).\n", ms.slaves_responding);
    //     pTModule->st_slaves = ms.slaves_responding;
    // }
    // if (ms.al_states != pTModule->master_state.al_states)
    // {
    //     printf("AL states: 0x%02X. \n", ms.al_states);
    //     if(ms.al_states == 0x01)
    //         pTModule->al_states = AL_INIT;
    //     if(ms.al_states == 0x02)
    //         pTModule->al_states = AL_PREOP;
    //     if(ms.al_states == 0x04)
    //         pTModule->al_states = AL_SAFEOP;
    //     if(ms.al_states == 0x08)
    //         pTModule->al_states = AL_OP;
    // }
    // if (ms.link_up != pTModule->master_state.link_up)
    // {
    //     printf("Link is %s.\n", ms.link_up ? "up" : "down");
    //     pTModule->master_link_up = ms.link_up;
    // }
    // pTModule->master_state = ms;
}

//获取当前系统时间
#define     TIMESPEC2NS(T)       ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
static uint64_t system_time_ns(void)
{
    struct timespec  rt_time;
    clock_gettime(CLOCK_REALTIME, &rt_time);
    uint64_t time = TIMESPEC2NS(rt_time);
    return time;
}

/******************************************************************************
* 功能：子线程：master使用ethercat交换pdo线程入口函数，此模块的生命周期就在此函数中。
* @param pTModule : args
* @return Descriptions
******************************************************************************/
static void check_chassis_domain1_state(CHASSIS_MASTER_THREAD_INFO *pTModule)
{
    static uint32_t cnt = 0;
    ec_domain_state_t ds;
    ecrt_domain_state(pTModule->domain1, &ds);
    if (ds.working_counter != pTModule->domain1_state.working_counter)
    {
        printf("master:%d Domain1: WC %u. cnt:%d\n", pTModule->mthreadMasterId,ds.working_counter,cnt++);
    }
    if (ds.wc_state != pTModule->domain1_state.wc_state)
    {
        printf("Domain1: State %u.\n", ds.wc_state);
    }
    pTModule->domain1_state = ds;
}

void* ethercatChassisThread(void* pModule)
{
    ETHERCAT_THREAD_INFO *pEModule = (ETHERCAT_THREAD_INFO *) pModule;
    CHASSIS_MASTER_THREAD_INFO *pTModule = (CHASSIS_MASTER_THREAD_INFO *)pEModule->pTModule;
    //uint16_t mIdentifier = pTModule->mIdentifier;
    unsigned int nDelay = 1000;        /* usec */
    //timer 定时器
    struct timespec  next, interval;
    interval.tv_sec  = nDelay / USEC_PER_SEC;
    interval.tv_nsec = (nDelay % USEC_PER_SEC) * 1000;
    clock_gettime(CLOCK_MONOTONIC, &next);
    int ret = 0;
    //uint32_t  runCnt = 0;
    while (pEModule->mWorking)
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

        if (!pTModule->masterActivated)
          continue;


        struct timespec  igh_s_time,igh_e_time;
        clock_gettime(CLOCK_MONOTONIC, &igh_s_time);

        ecrt_master_receive(pTModule->master);
        ecrt_domain_process(pTModule->domain1);
        /*Check process data state(optional)*/
        check_chassis_domain1_state(pTModule);
        //Check for master state
        check_chassis_master_state(pTModule);

        /******************************************实时传输PDO******************************************/
        //读取交换的数据
        // for (int i = 0; i < pTModule->motor_size; i++)
        // {
        //     //nimotion 伺服电机，读取速度单位是RPM  位置单位是：用户单位
        //     pTModule->ni_stword[i] = EC_READ_U16(pTModule->domain1_pd + pTModule->nimotion_pdos[i].status_word );
            
        //     float speedU = (float) EC_READ_S32(pTModule->domain1_pd + pTModule->nimotion_pdos[i].velocity_value);
        //     float posU = (float) EC_READ_S32(pTModule->domain1_pd + pTModule->nimotion_pdos[i].Position_value);
        //     //转换成电机圈
        //     if(i==0 || i==2 || i==4 || i==6)
        //     {
        //         //行走电机
        //         pTModule->w_pos[i/2] = posU*(W_POS_TO_M*CHASSIS_DIR[i]);
        //         pTModule->w_speed[i/2] = speedU * (W_VEL_RPM_TO_MM*CHASSIS_DIR[i]);
        //     }
        //     if(i==1 || i==3 || i==5 || i==7)
        //     {
        //         //旋转电机.位置转换.mIdentifier是框架id ： 1，2
        //         pTModule->r_pos[(i-1)/2] = posU*(R_POS_TO_MM*CHASSIS_DIR[i])- pTModule->r_pos_zore[i/2];
        //         pTModule->r_speed[(i-1)/2] =speedU * (R_VEL_RPM_TO_DEG*CHASSIS_DIR[i]);
        //     }

        // }

        // /******************************************有限状态机*******************************************/
        // chassisFsa(pTModule);

        // /******************************************实时传输PDO******************************************/
        // //写入交换somanet的数据，1ms 刷新写入一次
        // for (int i = 0; i < pTModule->motor_size; i++)
        // {
        //     //将从站输出的值转换成PDO类型的
        //     //sensors_write_i_to_f(pTModule,i);
        //     EC_WRITE_U16(pTModule->domain1_pd + pTModule->nimotion_pdos[i].ctrl_word, pTModule->motors[i].ctrl_word);
        //     EC_WRITE_U16(pTModule->domain1_pd + pTModule->nimotion_pdos[i].operation_mode, pTModule->motors[i].operation_mode);
        //     EC_WRITE_S32(pTModule->domain1_pd + pTModule->nimotion_pdos[i].target_velocity, pTModule->motors[i].target_velocity);
        // }        

        //  /**< Application-layer states of all slaves.
        //                           The states are coded in the lower 4 bits.
        //                           If a bit is set, it means that at least one
        //                           slave in the bus is in the corresponding
        //                           state:
        //                           - Bit 0: \a INIT
        //                           - Bit 1: \a PREOP
        //                           - Bit 2: \a SAFEOP
        //                           - Bit 3: \a OP */
        // if(pTModule->master_state.al_states == 0x08)
        // {
        //     ;
        // }else
        // {
        //   ;
        // }

        // write application time to master
        ecrt_master_application_time(pTModule->master, system_time_ns());
        ecrt_master_sync_reference_clock(pTModule->master);
        ecrt_master_sync_slave_clocks(pTModule->master);
        
        /*Send process data*/
        ecrt_domain_queue(pTModule->domain1);
        ecrt_master_send(pTModule->master);

        clock_gettime(CLOCK_MONOTONIC, &igh_e_time);
    }

    sleep(1);
    printf("ethercatThread exit\n\r");

    return NULL;
}
 