/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 08:42:07
 * @LastEditors: Zterch
 * @LastEditTime: 2026-06-16 16:50:10
 * @FilePath: /Mount_Sys/leader/src/mot_master.c
 * @Description: 
 * 
 * Copyright (c) 2026 by jy.zterch@foxmail.com, All Rights Reserved. 
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mot_master.h"
#include "mot_leader_defs.h"
#include "mot_share_ipc.h"
#include "mot_ctl.h"
#include "mot_commu_ipc.h"
#include "mot_share_daemon.h"

static int32_t initMaster(ARMS_MASTER_THREAD_INFO *pTModule);


static int moduleEndUp(ARMS_MASTER_THREAD_INFO *pTModule);
static int     handle_module_exit(ARMS_MASTER_THREAD_INFO *pTModule);

static void  destroyMaster(ARMS_MASTER_THREAD_INFO *pTModule);

static void  destroy_slaves_inf(EK1100_slaves_inf *slaves_inf_list);

static int  createMaster(int mid,ec_master_t **pms,ec_domain_t **pdi);


static inline void tsnorm(struct timespec *ts);

static void LoopRecLocalMsg(ARMS_MASTER_THREAD_INFO *pTModule);

static void  cyc_vel_upper_cmd(ARMS_MASTER_THREAD_INFO *pTModule);

static void  cyc_pos_upper_cmd(ARMS_MASTER_THREAD_INFO *pTModule);

/*****************************ethercat总线相关函数********************************/
#define     TIMESPEC2NS(T)       ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
static void check_domain1_state(ARMS_MASTER_THREAD_INFO *pTModule);
static void check_master_state(ARMS_MASTER_THREAD_INFO *pTModule);
static uint64_t system_time_ns(void);
static int32_t armsFsa(ARMS_MASTER_THREAD_INFO *pTModule);

void* ethercatBusThread(void* );
static void reflash_tpdo_Ebus(ARMS_MASTER_THREAD_INFO *pTModule);
static void reflash_rpdo_Ebus(ARMS_MASTER_THREAD_INFO *pTModule);

static void  destroyMaster(ARMS_MASTER_THREAD_INFO *pTModule)
{
    //伺服运行过程中，急停电机
    if (pTModule->masterActivated)
    {
        for (int i = 0; i < pTModule->armsSize; i++)
        {
            pTModule->arm_fsa[i] = S_HAIT;
        }
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
    for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    {  
        destroy_slaves_inf(&pTModule->slaves_inf[i]);
    }
    //销毁申请内存
    #if 0
    for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    {
        if(pTModule->mMagicWorker[i].auto_follow_sim_d_size >0 )
        {
            //设置仿真数据大小，并申请内存
            free(pTModule->mMagicWorker[i].auto_follow_sim_dx);
            free(pTModule->mMagicWorker[i].auto_follow_sim_dy);
            pTModule->mMagicWorker[i].auto_follow_sim_d_size = 0;
        }
    }
    #endif 
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


/******************************************************************************
* 功能：初始化master变量
* @return Descriptions
******************************************************************************/
static int32_t initMaster(ARMS_MASTER_THREAD_INFO *pTModule) {

  int32_t iRet = 0;
  //leader run in cpux
  hiSetCpuAffinity(pTModule->mCpuAffinity);

  //创建soket.*非阻塞模式
  pTModule->mSoket = createSoket(get_mn_master_port(pTModule->mthreadMasterId), MN_LocalIpV4Str, 0, 0);

  //创建slaves_inf头
  for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
  {    
    pTModule->slaves_inf[i].childSlaveSize = 0;
    INIT_LIST_HEAD(&pTModule->slaves_inf[i].list);
  }

  //测试创建插值表数据库。
  #if 0
  //delete_table(DB_FILE,db_interpn_tables[3]);
  for (int  i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
  {
    //delete_table(DB_FILE,db_interpn_tables[i]);
    create_interpn_table(DB_FILE,db_interpn_tables[i]);
  }
  #endif

  

  #if 0
  for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
  {  
    interpn_value v = interpn_magic(&pTModule->mQuads[i],-20,-20);  
    printf("interpn_v:(%f %f) \n", v.x,v.y);
  }
  #endif
  
  /*一号臂的电机的减速比、轴方向*/
//   for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
//   {
//         pTModule->AXIS_DIR[i][0] = 1;
//         pTModule->AXIS_DIR[i][1] = -1;
//         pTModule->AXIS_DIR[i][2] = 1;
//         pTModule->AXIS_DIR[i][3] = -1;
//   }

//   for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
//   {
//      pTModule->sikoXYZ_ZERO[i][0] = 0.0;
//      pTModule->sikoXYZ_ZERO[i][1] = 0.0;
//      pTModule->sikoXYZ_ZERO[i][2] = 0.0;
//      //上电时刻默认给中阀 小阀门-5V全部放气
//      for (int j = 2; j < 4; j++)
//      {
//         pTModule->proportion_cmd[i][j] = -5;   
//      }
//      //外部编码器绝对值起始位置
//      pTModule->xStartMExtPos[i] = 0;
//      pTModule->xStartSExtPos[i] = 0;
//   }
    
  //ethercat线程
  pTModule->mEtherWorker.mWorking = 1;
  strcpy(pTModule->mEtherWorker.mThreadName,"ethercatIgh");
  pTModule->mEtherWorker.pTModule = pTModule;
  pTModule->mEtherWorker.mPid = hiCreateThread("EtherIGH", ethercatBusThread,PRI_MASTER, &pTModule->mEtherWorker);


//   //创建算法magic线程。
//   for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
//   {
//     pTModule->mMagicWorker[i].mWorking = 1;
//     sprintf(pTModule->mMagicWorker[i].mThreadName,"arm:%d",i);
//     pTModule->mMagicWorker[i].mthreadMasterId = pTModule->masterId;
//     pTModule->mMagicWorker[i].mArmsId = i;
//     pTModule->mMagicWorker[i].pMaster = pTModule;
//     pTModule->mMagicWorker[i].mPid = hiCreateThread("armn", magic_threadEntry,PRI_MASTER, &(pTModule->mMagicWorker[i]));
//   }

  //初始化主站信息
  pTModule->master=NULL;
  memset(&pTModule->master_state,0,sizeof(ec_master_state_t));
  pTModule->domain1=NULL;
  memset(&pTModule->domain1_state,0,sizeof(ec_domain_state_t));

//   //初始化速度、位置控制信息
//   memset(pTModule->upper_target_p_Flag,0,sizeof(uint8_t)*MASTER_ARMS_MAX_SIZE);
//   memset(pTModule->upper_target_v_Flag,0,sizeof(uint8_t)*MASTER_ARMS_MAX_SIZE);
//   memset(pTModule->upper_target_p_Kp_cnt,0,sizeof(int32_t)*MASTER_ARMS_MAX_SIZE);

  return iRet;
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

/******************************************************************************
* 功能：此函数销毁初始化模块，释放线程指针里的内容
* @param pTModule : pTModule是线程信息结构体指针，里边存储的线程运行期间用到的数据和交换通信数据
* @return Descriptions
******************************************************************************/
static int moduleEndUp(ARMS_MASTER_THREAD_INFO *pTModule)
{
    destroyMaster(pTModule);
    if(pTModule->mSoket>0)
    {
        close(pTModule->mSoket);
        pTModule->mSoket = -1;
    }
    return 0;
}

static int  handle_module_exit(ARMS_MASTER_THREAD_INFO *pTModule)
{
    printf("BASE::MSG_ID_MODULE_EXIT_ALL -- master\n");
    pTModule->mState = M_STATE_QUIT;

    pTModule->mWorking = false;
    //线程销毁
    pTModule->mEtherWorker.mWorking = false;
    // pTModule->mVisionWorker.mWorking = false;

    // for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    // {
    //     pTModule->mMagicWorker[i].mWorking = false;
    // }
    pthread_join(pTModule->mEtherWorker.mPid, NULL);
    // pthread_join(pTModule->mVisionWorker.mPid, NULL);

    // for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    // {
    //     pthread_join(pTModule->mMagicWorker[i].mPid, NULL);
    // }
    //发送到leader 已经退出
    sendNoticeToLeader(pTModule->mSoket,MSG_ID_THREAD_REQUEST_EXIT_ALL_ACK, 0, 0);
    printf("timer endup\n");
    return 0;
}


/*** 
 * @description: 
 * @Author: lyq
 * @Date: 2024-06-04 01:31:37
 * @return {*}
 */
static void LoopRecLocalMsg(ARMS_MASTER_THREAD_INFO *pTModule)
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
        
        case MSG_ID_MODULE_EXIT_ALL:
        {
            handle_module_exit(pTModule);
            break;
        }
        case CMD_IGH_CREATE_MASTERS:
        {
            createMaster(pTModule->masterId,&pTModule->master,&pTModule->domain1);
            break;
        }
        case CMD_DESTROY_MASTER:
        {
            if(pTModule->master != NULL)
            {
                    destroyMaster(pTModule);
                       
            }
            break;
        }
        default:
        {
            break;
        }
      } //end switch

    }

}

static void  cyc_vel_upper_cmd(ARMS_MASTER_THREAD_INFO *pTModule)
{
    /**
     * 200HZ 运行周期
     */
    
    // for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    // {
    //     if(pTModule->upper_target_v_Flag[i])
    //     {
    //         //V_cmd = V_now + a*dt
    //         float cmdSpeed[4];
    //         int cmd_target_ok = 0;
    //         for (int j = 0; j < 4; j++)
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
    //             }else if(pTModule->upper_acc[i][j] == 0.0)
    //             {
    //                 cmd_target_ok++;
    //             }
    //         }

    //         //判断是否都达到目标速度
    //         if(
    //             fabs(cmdSpeed[0]-pTModule->upper_target_velocity[i][0]) < 0.01 &&
    //             fabs(cmdSpeed[1]-pTModule->upper_target_velocity[i][1]) < 0.01 &&
    //             fabs(cmdSpeed[2]-pTModule->upper_target_velocity[i][2]) < 0.01 && 
    //             fabs(cmdSpeed[3]-pTModule->upper_target_velocity[i][3]) < 0.01 &&
    //             cmd_target_ok >= 4
    //         )
    //         {
    //             pTModule->upper_target_v_Flag[i] = 0;
    //             cmdSpeed[0] = pTModule->upper_target_velocity[i][0];
    //             cmdSpeed[1] = pTModule->upper_target_velocity[i][1];
    //             cmdSpeed[2] = pTModule->upper_target_velocity[i][2];
    //             cmdSpeed[3] = pTModule->upper_target_velocity[i][3];
    //         }

    //         for (int j = 0; j < 4; j++)
    //         {
    //             pTModule->upper_last_v_cmd[i][j] = cmdSpeed[j];
    //         }
    //         armsSetSpeed(pTModule,i,cmdSpeed[0],cmdSpeed[1],cmdSpeed[2],cmdSpeed[3]);

            
            
    //     }
        
        
    // }
    
}

static void  cyc_pos_upper_cmd(ARMS_MASTER_THREAD_INFO *pTModule)
{
    /**
     * 200HZ 运行周期
     */
    // for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    // {
    //     if(pTModule->upper_target_p_Flag[i])
    //     {
    //         //V_cmd = Kp*(S_obj-S_now) + Kd(^S_obj-^S_now)
    //         float cmdSpeed[4];
    //         float kp = ((float)pTModule->upper_target_p_Kp_cnt[i])/2.0;
    //         float kd = ((float)pTModule->upper_target_p_Kp_cnt[i])/20.0;
    //         if(kp>500)
    //         {
    //             kp = 500;
    //         }
    //         if(kd>50)
    //         {
    //             kd = 50;
    //         }
    //         pTModule->upper_target_p_Kp_cnt[i]++;

    //         float error[4];
    //         error[0] = pTModule->upper_target_pos[i][0]-pTModule->xMIntPos[i];
    //         error[1] = pTModule->upper_target_pos[i][1]-pTModule->xSIntPos[i];
    //         error[2] = pTModule->upper_target_pos[i][2]-pTModule->yPos[i];
    //         error[3] = pTModule->upper_target_pos[i][3]-pTModule->zPos[i];

    //         cmdSpeed[0]=kp*(error[0])-kd*(pTModule->xMIntSpeed[i]/1000.0);
    //         cmdSpeed[1]=kp*(error[1])-kd*(pTModule->xSIntSpeed[i]/1000.0);
    //         cmdSpeed[2]=kp*(error[2])    -kd*(pTModule->ySpeed[i]/1000.0);
    //         cmdSpeed[3]=kp*(error[3])    -kd*(pTModule->zSpeed[i]/1000.0);
           
    //         //printf("%f %f %f %f \n",error[0],cmdSpeed[0],error[1],cmdSpeed[1]);
    //         /*
    //         for (int j = 0; j < 4; j++)
    //         {
    //             float max_acc = 20;
    //             //起始阶段限制加速度.
    //             float new_cmd_acc = (cmdSpeed[j] - pTModule->target_p_last_cmdvel[i][j])/0.005;
    //             if(new_cmd_acc > max_acc)
    //             {
    //                 cmdSpeed[j] = pTModule->target_p_last_cmdvel[i][j] + max_acc*0.005;
    //             }
    //             if(new_cmd_acc < -max_acc)
    //             {
    //                 cmdSpeed[j] = pTModule->target_p_last_cmdvel[i][j] - max_acc*0.005;
    //             }
    //             pTModule->target_p_last_cmdvel[i][j] = cmdSpeed[j];
    //         }
    //         */
    //         #if 1
    //         //速度命令平滑处理
    //         for (int  j = 0; j < 4; j++)
    //         {
    //             for (int k = 0; k < upper_pos_smooth_cnt-1; k++)
    //             {
    //                 pTModule->upper_pos_vel_smooth[i][j][k] = pTModule->upper_pos_vel_smooth[i][j][k+1];
    //             } 
    //             pTModule->upper_pos_vel_smooth[i][j][upper_pos_smooth_cnt-1] = cmdSpeed[j];

    //             float smoothV = 0;
    //             for (int k = 0; k < upper_pos_smooth_cnt; k++)
    //             {
    //                 smoothV += pTModule->upper_pos_vel_smooth[i][j][k];
    //             } 
    //             cmdSpeed[j] = smoothV/upper_pos_smooth_cnt;
    //         }
    //         #endif
    //         //判断是否都达到目标速度
    //         if(fabs(error[0]) < 0.0002)
    //             cmdSpeed[0] = 0;
    //         if(fabs(error[1]) < 0.0002)
    //             cmdSpeed[1] = 0;
    //         if(fabs(error[2]) < 0.0002)
    //             cmdSpeed[2] = 0;
    //         if(fabs(error[3]) < 0.0002)
    //             cmdSpeed[3] = 0;

    //         //限制速度
    //         for (int j = 0; j < 4; j++)
    //         {
    //             if(cmdSpeed[j]>50)
    //                 cmdSpeed[j] = 50;
    //             if(cmdSpeed[j]<-50)
    //                 cmdSpeed[j] = -50;
    //         }
    //         armsSetSpeed(pTModule,i,cmdSpeed[0],cmdSpeed[1],cmdSpeed[2],cmdSpeed[3]);
    //         //printf("%f %f %f %f\n",cmdSpeed[0],cmdSpeed[1], cmdSpeed[2],cmdSpeed[3]);
    //         //到达位置
    //         if(
    //             fabs(pTModule->xMIntPos[i]-pTModule->upper_target_pos[i][0]) < 0.0002 &&
    //             fabs(pTModule->xSIntPos[i]-pTModule->upper_target_pos[i][1]) < 0.0002 &&
    //             fabs(pTModule->yPos[i]-pTModule->upper_target_pos[i][2]) < 0.0002 &&
    //             fabs(pTModule->zPos[i]-pTModule->upper_target_pos[i][3]) < 0.0002
    //         )
    //         {
    //             pTModule->upper_target_p_Flag[i] = 0;
    //             pTModule->upper_target_p_Kp_cnt[i] = 0;
    //             sendNoticeToUpper(pTModule,pTModule->mIdentifier,i,ACK_NOTICE_HAND_POS_ARRIVE);
    //         }
    //     }
    // }
}

/*****************************************ethercat总线相关**************************************************/
//获取当前系统时间
static uint64_t system_time_ns(void)
{
    struct timespec  rt_time;
    clock_gettime(CLOCK_REALTIME, &rt_time);
    uint64_t time = TIMESPEC2NS(rt_time);
    return time;
}

static int32_t armsFsa(ARMS_MASTER_THREAD_INFO *pTModule)
{
    int32_t iRet = 0;
  
    //有限状态机切换
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        //4个伺服电机、EK1100 Ek1110 ...
        if(pTModule->slaves_inf[i].childSlaveSize>4)
        {
            //写入4个电机的数据
            switch (pTModule->arm_fsa[i])
            {
                case S_SHUTDOWN:
                {
                    //"开启"禁用状态，此时需要发送控制字
                    for (int j = 0; j < 4; j++)
                    {
                        pTModule->motors[i][j].ctrl_word = 0x0006;
                        pTModule->motors[i][j].operation_mode = pTModule->somanetn_mode;
                        pTModule->motors[i][j].target_velocity = 0;
                    }
                    break;
                }
                case S_SWITCH_ON:
                {
                    for (int j = 0; j < 4; j++)
                    {
                        pTModule->motors[i][j].ctrl_word = 0x0007;
                        pTModule->motors[i][j].operation_mode = pTModule->somanetn_mode;
                        pTModule->motors[i][j].target_velocity = 0;   
                    }                   
                    break;
                }
                case S_ENABLE_OP:
                {
                    //判断状态字是否op成功
                    if
                    ( (pTModule->motors[i][0].status_word&0x67)==0x27 &&(pTModule->motors[i][1].status_word&0x67)==0x27 && 
                      (pTModule->motors[i][2].status_word&0x67)==0x27 && (pTModule->motors[i][3].status_word&0x67)==0x27
                    )
                    {
                        pTModule->arm_fsa[i] = S_OP_OK;
                    }
                    else
                    {
                        for (int j = 0; j < 4; j++)
                        {
                            pTModule->motors[i][j].ctrl_word = 0x000F;
                            pTModule->motors[i][j].operation_mode = pTModule->somanetn_mode;
                        }
                    }
                    break;
                }
                case S_HAIT:
                {
                    
                    for (int j = 0; j < 4; j++)
                    {
                        pTModule->motors[i][j].ctrl_word = 0x02; 
                        pTModule->motors[i][j].target_velocity = 0;
                    }

                    // if(chaeckArmIStop(pTModule,i))
                    // {
                    //     pTModule->arm_fsa[i] = S_NULL;
                    // }
                    break;
                }
                case S_NULL:{
                    //TUDO init some param
                    break;
                }
                case S_OP_OK:{
                    
                    //TUDO init some param
                    break;
                }
                default:{
                    break;
                }
            }

            //电机再op状态下出现error，进行急停
            // if(chaeckArmIError(pTModule,i)!=0 && (pTModule->arm_fsa[i]==S_ENABLE_OP || pTModule->arm_fsa[i]==S_NULL))
            // {
           
            //     pTModule->arm_fsa[i] = S_HAIT;
            
            // } 
            
        }
    }

  return iRet;
}

static void check_master_state(ARMS_MASTER_THREAD_INFO *pTModule)
{
    ec_master_state_t ms;
    ecrt_master_state(pTModule->master, &ms);
    if (ms.slaves_responding != pTModule->master_state.slaves_responding)
    {
        printf("%u slave(s).\n", ms.slaves_responding);
        pTModule->st_slaves = ms.slaves_responding;
    }
    if (ms.al_states != pTModule->master_state.al_states)
    {
        printf("AL states: 0x%02X. \n", ms.al_states);
        if(ms.al_states == 0x01)
            pTModule->al_states = AL_INIT;
        if(ms.al_states == 0x02)
            pTModule->al_states = AL_PREOP;
        if(ms.al_states == 0x04)
            pTModule->al_states = AL_SAFEOP;
        if(ms.al_states == 0x08)
            pTModule->al_states = AL_OP;
    }
    if (ms.link_up != pTModule->master_state.link_up)
    {
        printf("Link is %s.\n", ms.link_up ? "up" : "down");
        pTModule->master_link_up = ms.link_up;
    }
    pTModule->master_state = ms;
}

static void check_domain1_state(ARMS_MASTER_THREAD_INFO *pTModule)
{
    static uint32_t cnt = 0;
    ec_domain_state_t ds;
    ecrt_domain_state(pTModule->domain1, &ds);
    if (ds.working_counter != pTModule->domain1_state.working_counter)
    {
        printf("master:%d Domain1: WC %u. cnt:%d\n",pTModule->mthreadMasterId, ds.working_counter,cnt++);
        pTModule->domain_wc = ds.working_counter;
    }
    if (ds.wc_state != pTModule->domain1_state.wc_state)
    {
        printf("Domain1: State %u.\n", ds.wc_state);
    }
    pTModule->domain1_state = ds;
}


/******************************************************************************
* 功能：线程入口函数，此模块的生命周期就在此函数中。
* @param pTModule : pTModule是线程信息指针，里边包含发送/接收消息，socket等信息
* @return Descriptions
******************************************************************************/
void* master_arms_threadEntry(void* pModule)
{
  ARMS_MASTER_THREAD_INFO *pTModule = (ARMS_MASTER_THREAD_INFO *) pModule;

  if(NULL == pTModule)
  {
    printf("null timer\n");
    return 0;
  }
  initMaster(pTModule);

  unsigned int nDelay = 1000;        /* usec */
  //timer 定时器
  struct timespec  next, interval;
  interval.tv_sec  = nDelay / USEC_PER_SEC;
  interval.tv_nsec = (nDelay % USEC_PER_SEC) * 1000;
  clock_gettime(CLOCK_MONOTONIC, &next);

  printf("\n%s running!threadEntry\n",pTModule->mThreadName);
  sendNoticeToLeader(pTModule->mSoket,MSG_ID_THREAD_START_ACK, 0, 0);

  int ret = 0;
  uint32_t runCnt = 0;
  //int maxR =0;
  pTModule->masterActivated=false;
  uint32_t pack_ms = 10;

  //系统出错数据包的记录的北京时间
  //time_t rawtime;
  //struct tm *timeinfo;
  
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
        LoopRecLocalMsg(pTModule);
        //循环2s的增加速度到目标值
        cyc_vel_upper_cmd(pTModule);
        cyc_pos_upper_cmd(pTModule);

        //检查是否触碰软线
        //check_arms_near(pTModule);
        //检查随动算法主副轴是否误差过大
        //checkFollowMSPosError(pTModule);
    }
    //检查机械臂上限位开关，并作处理
    //check_limit_switch(pTModule);

    //50HZ发送到上位机,机械臂信息
    if((runCnt) % pack_ms==0)
    {
        //static uint32_t ccc = 0;
        //主站激活之后才发送消息到上位机
        if(pTModule->masterActivated)
        {
            //packM_S_MSG(pTModule);
            //packM_S_Sample_MSG(pTModule);     


        }
    }
    
    //发送吊杆主站控制信息到底盘
    if((runCnt) % 20==0)
    {
        //发送吊杆主站控制信息到底盘
        //packM_S_chassisMSG(pTModule);
    }

    //10HZ发送到上位机,机械臂信息
    if((runCnt) % 100==0)
    {
        

        //主站激活之后才发送消息到上位机
        if(pTModule->masterActivated)
        {
            //packM_S_master_run_inf(pTModule);
        }
        pack_ms = pTModule->PACK_MSG_MS;
        if(pack_ms <10 )
            pack_ms = 10;
    }

  }

    sleep(1);
    
    moduleEndUp(pTModule);

    return NULL;
}


/******************************************************************************
* 功能：子线程：master使用ethercat交换pdo线程入口函数，此模块的生命周期就在此函数中。
* @param pTModule : args
* @return Descriptions
******************************************************************************/
static void reflash_tpdo_Ebus(ARMS_MASTER_THREAD_INFO *pTModule)
{
    //读取交换somanet的数据
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        //4个伺服电机、EK1100 Ek1110 ...
        if(pTModule->slaves_inf[i].childSlaveSize>4)
        {
            for (int j = 0; j < 4; j++)
            {
                //CSV、CSP模式下都需要刷新
                pTModule->motors[i][j].status_word    = EC_READ_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].status_word);
                pTModule->motors[i][j].velocity_value = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].velocity_value);
                pTModule->motors[i][j].Ext_Pos_value = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].Ext_Pos_value);
                pTModule->motors[i][j].Int_Pos_value = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].Int_Pos_value);
                
                //CVS模式下，算法要用到力矩偏置
                pTModule->motors[i][j].torque_act_value = EC_READ_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].torque_act_value);
                pTModule->motors[i][j].torque_demand = EC_READ_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].torque_demand);
            }
            //伺服里的PDO数据参数转换成使用单位
            // somanets_i_to_f(pTModule,i);            
            //读取EC-3045传感器数据,绝压值
            //pTModule->EC_3045_abs_stress[i] = EC_READ_U16(pTModule->domain1_pd + pTModule->EC_3045[i][6]);
            //读取EC-3045传感器数据,限位开关
            //pTModule->EC_3045_switch[i] = EC_READ_U16(pTModule->domain1_pd + pTModule->EC_3045[i][5]);
        }
    }
}

static void reflash_rpdo_Ebus(ARMS_MASTER_THREAD_INFO *pTModule)
{
    //写入交换somanet的数据，1ms 刷新写入一次
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        //4个伺服电机、EK1100 Ek1110 ...
        if(pTModule->slaves_inf[i].childSlaveSize>4)
        {             
            //CVS模式下，算法要用到力矩偏置
            if(pTModule->somanetn_mode == MODE_CSV)
            {
                //目标速度存储
                // pTModule->cmd_velocity[i][0] = ((float)pTModule->motors[i][1].target_velocity)*X_RPM_TO_V_MM*pTModule->AXIS_DIR[i][1];
                // pTModule->cmd_velocity[i][1] = ((float)pTModule->motors[i][2].target_velocity)*X_RPM_TO_V_MM*pTModule->AXIS_DIR[i][2];
                // pTModule->cmd_velocity[i][2] = ((float)pTModule->motors[i][0].target_velocity)*Y_RPM_TO_V_MM*pTModule->AXIS_DIR[i][0];
                // pTModule->cmd_velocity[i][3] = ((float)pTModule->motors[i][3].target_velocity)*Z_RPM_TO_V_MM*pTModule->AXIS_DIR[i][3];
                for (int j = 0; j < 4; j++)
                {
                    EC_WRITE_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].ctrl_word, pTModule->motors[i][j].ctrl_word);
                    EC_WRITE_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].operation_mode, pTModule->motors[i][j].operation_mode);
                    EC_WRITE_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].target_velocity, pTModule->motors[i][j].target_velocity);
                    EC_WRITE_S16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].torque_offset, pTModule->motors[i][j].torque_offset);
                }
            }
            //EC-3045输出
            //将从站输出的值转换成PDO类型的
            // prop_to_ec3045(pTModule,i); 
        //     EC_WRITE_U16(pTModule->domain1_pd + pTModule->EC_3045[i][0], pTModule->EC_3045_vari[i][0]);

        //     //模拟量输出
        //     EC_WRITE_U16(pTModule->domain1_pd + pTModule->EC_3045[i][1], pTModule->EC_3045_vari[i][1]);
        //     EC_WRITE_U16(pTModule->domain1_pd + pTModule->EC_3045[i][2], pTModule->EC_3045_vari[i][2]);
        //     EC_WRITE_U16(pTModule->domain1_pd + pTModule->EC_3045[i][3], pTModule->EC_3045_vari[i][3]);
        //     EC_WRITE_U16(pTModule->domain1_pd + pTModule->EC_3045[i][4], pTModule->EC_3045_vari[i][4]);
        }
    }
}

void* ethercatBusThread(void* pModule)
{
    ETHERCAT_THREAD_INFO *pEModule = (ETHERCAT_THREAD_INFO *) pModule;
    ARMS_MASTER_THREAD_INFO *pTModule = (ARMS_MASTER_THREAD_INFO *)pEModule->pTModule;

    unsigned int nDelay = 1000;        /* usec */
    //timer 定时器
    struct timespec  next, interval;
    interval.tv_sec  = nDelay / USEC_PER_SEC;
    interval.tv_nsec = (nDelay % USEC_PER_SEC) * 1000;
    clock_gettime(CLOCK_MONOTONIC, &next);
    int ret = 0;
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
        check_domain1_state(pTModule);
        //Check for master state
        check_master_state(pTModule);

        /******************************************实时传输PDO******************************************/
        //读取交换somanet的数据
        reflash_tpdo_Ebus(pTModule);
       
        /******************************************有限状态机******************************************/
        armsFsa(pTModule);

        /******************************************实时传输PDO******************************************/
        reflash_rpdo_Ebus(pTModule);
                 
         /**< Application-layer states of all slaves.
                                  The states are coded in the lower 4 bits.
                                  If a bit is set, it means that at least one
                                  slave in the bus is in the corresponding
                                  state:
                                  - Bit 0: \a INIT
                                  - Bit 1: \a PREOP
                                  - Bit 2: \a SAFEOP
                                  - Bit 3: \a OP */
        if(pTModule->master_state.al_states == 0x08)
        {
            
        }else
        {
          ;
        }

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