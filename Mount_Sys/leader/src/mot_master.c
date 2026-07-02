/*
 * @Author: Zterch jy.zterch@foxmail.com
 * @Date: 2026-05-28 08:42:07
 * @LastEditors: Zterch
 * @LastEditTime: 2026-07-02 10:02:35
 * @FilePath: /Mount_Prj/Mount_Sys/leader/src/mot_master.c
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

static int  scanSlaves(ARMS_MASTER_THREAD_INFO *pTModule);
static void   upperFSASomanets(ARMS_MASTER_THREAD_INFO *pTModule,S_ETHR_STATE fsastate);
static void   clearErrorSomanets(ARMS_MASTER_THREAD_INFO *pTModule,SUPR_R_MSG *mSMsg);
static void  upperAngularVelSomanets(ARMS_MASTER_THREAD_INFO *pTModule,SUPR_R_MSG *mSMsg);
static int32_t handleStartSlight(ARMS_MASTER_THREAD_INFO *pTModule,int armsid,float v0,float v1,float v2,float v3,float v4,float v5,float acc_mm);
static void  upperRelaPosSomanets(ARMS_MASTER_THREAD_INFO *pTModule,SUPR_R_MSG *mSMsg);
static void  upperAbsPosSomanets(ARMS_MASTER_THREAD_INFO *pTModule,SUPR_R_MSG *mSMsg);
static void    activeMaterKeydown(ARMS_MASTER_THREAD_INFO *pTModule);
static int  confSlavesPdos(ARMS_MASTER_THREAD_INFO *pTModule);
static int  activeMater(ARMS_MASTER_THREAD_INFO *pTModule);
static void  confSomanetCSVPdos(ec_domain_t *domain1,ec_slave_info_t *inf,ec_slave_config_t *sc,motec_pdos * ppdos);
static void confRS485GatewayPdos(ec_domain_t *domain1, ec_slave_info_t *inf, ec_slave_config_t *sc);

/******************************************************************************
 * 新架构静态从站配置表（替代EK1100动态检测）
 * 连接顺序：PC -> 臂1(电机1->电机2->电机3->RS485网关->托板电机1->托板电机2->托板电机3)
 *           -> 臂2(电机1->电机2->电机3->RS485网关->托板电机1->托板电机2->托板电机3)
 * 
 * 注意：请根据实际设备的 vendor_id 和 product_code 修改以下值！
 *       可用命令查看：sudo ethercat slaves -v
 ******************************************************************************/
static const SLAVE_CONFIG_T g_slave_config[TOTAL_ARM_SLAVES] = {
    /* ========== 机械臂1 (arm_id=0) ========== */
    {0,  0, 0x000007CF, 0x50101611, SLAVE_TYPE_ARM_MOTOR,   0, "MOTEC_DC_SERVO"},
    {0,  1, 0x000007CF, 0x50101611, SLAVE_TYPE_ARM_MOTOR,   0, "MOTEC_DC_SERVO"},
    {0,  2, 0x000007CF, 0x50101611, SLAVE_TYPE_ARM_MOTOR,   0, "MOTEC_DC_SERVO"},
    {0,  3, 0x00000B95, 0x00001400, SLAVE_TYPE_RS485_GATEWAY, 0, "AMSAMOTION_EC_MB"},
    {0,  4, 0x000007CF, 0x50101611, SLAVE_TYPE_PALLET_MOTOR, 0, "MOTEC_DC_SERVO"},
    {0,  5, 0x000007CF, 0x50101611, SLAVE_TYPE_PALLET_MOTOR, 0, "MOTEC_DC_SERVO"},
    {0,  6, 0x000007CF, 0x50101611, SLAVE_TYPE_PALLET_MOTOR, 0, "MOTEC_DC_SERVO"},
    
    /* ========== 机械臂2 (arm_id=1) ========== */
    {0,  7, 0x000007CF, 0x50101611, SLAVE_TYPE_ARM_MOTOR,   1, "MOTEC_DC_SERVO"},
    {0,  8, 0x000007CF, 0x50101611, SLAVE_TYPE_ARM_MOTOR,   1, "MOTEC_DC_SERVO"},
    {0,  9, 0x000007CF, 0x50101611, SLAVE_TYPE_ARM_MOTOR,   1, "MOTEC_DC_SERVO"},
    {0, 10, 0x00000B95, 0x00001400, SLAVE_TYPE_RS485_GATEWAY, 1, "AMSAMOTION_EC_MB"},
    {0, 11, 0x000007CF, 0x50101611, SLAVE_TYPE_PALLET_MOTOR, 1, "MOTEC_DC_SERVO"},
    {0, 12, 0x000007CF, 0x50101611, SLAVE_TYPE_PALLET_MOTOR, 1, "MOTEC_DC_SERVO"},
    {0, 13, 0x000007CF, 0x50101611, SLAVE_TYPE_PALLET_MOTOR, 1, "MOTEC_DC_SERVO"},
};


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


static int  scanSlaves(ARMS_MASTER_THREAD_INFO *pTModule)
{
  int iRet = -1;
  if(!pTModule->master)
    return iRet;

  //创建slaves_inf头
  for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
  {    
    destroy_slaves_inf(&pTModule->slaves_inf[i]);
    INIT_LIST_HEAD(&pTModule->slaves_inf[i].list);
  }


  //使用命令行，查询从站个数
  FILE *fp = NULL;
  char comm[64]="";
  sprintf(comm,"sudo ethercat -m %d slave | wc -l",pTModule->masterId);
  fp = popen(comm,"r");

  int actual_slave_size = 0;
  if (fp != NULL)
  {   
      char buffer[100];
      while (fgets(buffer, 100, fp) != NULL)
          actual_slave_size = atoi(buffer);

      fclose(fp);          
  }
  if(actual_slave_size != TOTAL_ARM_SLAVES) {
    printf("警告: 实际从站数(%d)与配置表(%d)不匹配！请检查接线或配置。\n", 
               actual_slave_size, TOTAL_ARM_SLAVES);
  }

    for (int i = 0; i < TOTAL_ARM_SLAVES; i++) {
    const SLAVE_CONFIG_T *cfg = &g_slave_config[i];
    ec_slave_info_t slave_in;
    
    // 从主站获取该position的实际从站信息
    if(ecrt_master_get_slave(pTModule->master, cfg->position, &slave_in) < 0)
    {
        printf("获取从站 %d 信息失败\n", cfg->position);
        break;
    }

    // 分配从站信息节点
    EK1100_slaves_inf *childSlave = (EK1100_slaves_inf*)malloc(sizeof(EK1100_slaves_inf));
    memset((char*)childSlave, 0, sizeof(EK1100_slaves_inf));
    memcpy(&childSlave->slave_in, &slave_in, sizeof(ec_slave_info_t));
    
    childSlave->masteri = pTModule->masterId;
    
    // 添加到对应臂的链表
    list_add_tail(&childSlave->list, &pTModule->slaves_inf[cfg->arm_id].list);
    pTModule->slaves_inf[cfg->arm_id].childSlaveSize++;

    printf("&&&&&&& [%s] 臂%d 位置%d: %s (vendor:0x%08X product:0x%08X)\n", 
            cfg->type == SLAVE_TYPE_RS485_GATEWAY ? "网关" : 
            cfg->type == SLAVE_TYPE_PALLET_MOTOR ? "托板" : "臂电机",
            cfg->arm_id, cfg->position, 
            childSlave->slave_in.name,
            childSlave->slave_in.vendor_id,
            childSlave->slave_in.product_code);
  }
  //设置从站个数、arms的个数
  pTModule->slaves_size = TOTAL_ARM_SLAVES;
  pTModule->armsSize = MASTER_ARMS_MAX_SIZE;
  printf("扫描完成: 总共%d个从站, %d套臂, 每臂%d个从站\n", 
           pTModule->slaves_size, pTModule->armsSize, ARM_SLAVES_PER_ARM);
  return 0;
}


static void  confSomanetCSVPdos(ec_domain_t *domain1,ec_slave_info_t *inf,ec_slave_config_t *sc,motec_pdos * ppdos)
{
    /*Config PDOs*/
    uint8_t rpdo_entries_size = 5;
    uint8_t tpdo_entries_size = 7;
    ec_pdo_entry_info_t rpdo_entries[] = {
        /*RxPdo 0x1600*/
        {0x6040, 0x00, 16},
        {0x6060, 0x00, 8},
        {0x607A, 0x00, 32},
        {0x60FF, 0x00, 32},
        {0x60B2, 0x00, 16},
    };
    ec_pdo_entry_info_t tpdo_entries[] = {
        /*TxPdo 0x1A00*/
        {0x6041, 0x00, 16},
        {0x6061, 0x00, 8},
        {0x6064, 0x00, 32},
        {0x606C, 0x00, 32},
        {0x2111, 0x02, 32},
        {0x6077, 0x00, 16},
        {0x6074, 0x00, 16}
    };

    ec_pdo_info_t ec_rpdos[] = {
        //RxPdo
        {0x1600, rpdo_entries_size, rpdo_entries + 0 }, /*recive pdo maping*/
    };
    ec_pdo_info_t ec_tpdos[] = {
        //TxPdo
        {0x1A00, tpdo_entries_size, tpdo_entries + 0 } /*send pdo maping*/
    };

    /*
     通过ethercat命令行可以看到和pdo相关的几个数据结构，主站中也是通过链表这么存储的。如果从站支持邮箱协议，0,1默认给邮箱通信使用。
     SM(Sync manager index):0 邮箱数据
     SM(Sync manager index):1 邮箱数据
     SM(Sync manager index):2 pdos 0x1600 ...
                                          多个pdo条目
     SM(Sync manager index):3 pdos 0x1a00 ...
                                          多个pdo条目
     */
    ec_sync_info_t servo_syncs[] = {
        { 0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE },
        { 1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE },
        { 2, EC_DIR_OUTPUT, 1, ec_rpdos + 0, EC_WD_DISABLE },
        { 3, EC_DIR_INPUT, 1, ec_tpdos + 0, EC_WD_DISABLE },
        { 0xFF}
    };
    

    //清除伺服错误
    ecrt_slave_config_sdo16( sc, 0x6040, 0, 0x80 );
    ecrt_slave_config_sdo16( sc, 0x2010, 0x0a, 0x07D0);
    ecrt_slave_config_sdo16( sc, 0x2010, 0x0b, 0x07D0);
    #if 0
    //配置RPDO
    /* Clear RxPdo */
    ecrt_slave_config_sdo8( sc, 0x1C12, 0, 0 ); /* clear sm pdo 0x1c12 */
    ecrt_slave_config_sdo8( sc, 0x1600, 0, 0 ); /* clear RxPdo 0x1600 */
    ecrt_slave_config_sdo8( sc, 0x1601, 0, 0 ); /* clear RxPdo 0x1601 */
    ecrt_slave_config_sdo8( sc, 0x1602, 0, 0 ); /* clear RxPdo 0x1602 */
    ecrt_slave_config_sdo8( sc, 0x1603, 0, 0 ); /* clear RxPdo 0x1603 */

    ecrt_slave_config_sdo32( sc, 0x1600, 1, 0x60400010 ); /* 0x6040:0/16bits, control word */
    ecrt_slave_config_sdo32( sc, 0x1600, 2, 0x60600008 ); /* 0x6060:0/8bits  ,Modes of operation */              
    ecrt_slave_config_sdo32( sc, 0x1600, 3, 0x607A0020 ); /* 0x607A:0/32bits ,Target position  */
    ecrt_slave_config_sdo32( sc, 0x1600, 4, 0x60FF0020 ); /* 0x60FF:0/32bits ,target velocity  */
    ecrt_slave_config_sdo32( sc, 0x1600, 5, 0x60B20010 ); /* 0x60B2:0/16bits ,Torque offset  */
    //ecrt_slave_config_sdo32( sc, 0x1600, 6, 0x20100a10 ); /* 0x2010:0a/16bits ,Setting time  */
    //ecrt_slave_config_sdo32( sc, 0x1600, 7, 0x20100b10 ); /* 0x2010:0b/16bits ,Damping radio  */

    ecrt_slave_config_sdo8(  sc, 0x1600, 0, rpdo_entries_size); /* set number of PDO entries for 0x1600 */
    ecrt_slave_config_sdo16( sc, 0x1C12, 1, 0x1600 ); /* list all RxPdo in 0x1C12:1-4 */
    ecrt_slave_config_sdo8(  sc, 0x1C12, 0, 1 ); /* set number of RxPDO */

    //配置TPDO
    ecrt_slave_config_sdo8( sc, 0x1C13, 0, 0 ); /* clear sm pdo 0x1c13 */
    ecrt_slave_config_sdo8( sc, 0x1A00, 0, 0 ); /* clear TxPdo 0x1A00 */
    ecrt_slave_config_sdo8( sc, 0x1A01, 0, 0 ); /* clear TxPdo 0x1A01 */
    ecrt_slave_config_sdo8( sc, 0x1A02, 0, 0 ); /* clear TxPdo 0x1A02 */
    ecrt_slave_config_sdo8( sc, 0x1A03, 0, 0 ); /* clear TxPdo 0x1A03 */
    /* Define TxPdo */
    ecrt_slave_config_sdo32( sc, 0x1A00, 1, 0x60410010 ); /* 0x6041:0/16bits, status word */
    ecrt_slave_config_sdo32( sc, 0x1A00, 2, 0x60610010 ); /* 0x6061:0/8bits,  mode of operation display*/
    ecrt_slave_config_sdo32( sc, 0x1A00, 3, 0x60640020 );  /* 0x6064:0/32bits, act position */
    ecrt_slave_config_sdo32( sc, 0x1A00, 4, 0x606C0020 );  /* 0x606c:0/32bits, act velocity */
    
    //双编码器使用
    ecrt_slave_config_sdo32( sc, 0x1A00, 5, 0x21110220 );  /* 0x606c:0/32bits, encoder1 pos */
    ecrt_slave_config_sdo32( sc, 0x1A00, 6, 0x60770010 );  /* 0x6077:0/16bits, torque act value */
    ecrt_slave_config_sdo32( sc, 0x1A00, 7, 0x20100a10 );  /* 2010:0a/316bits, setting time */
    ecrt_slave_config_sdo32( sc, 0x1A00, 8, 0x20100b10 );  /* 0x2010:0b/16bits, damping ratio */

    ecrt_slave_config_sdo8(  sc, 0x1A00, 0, tpdo_entries_size ); /* set number of PDO entries for 0x1A00 */
    ecrt_slave_config_sdo16( sc, 0x1C13, 1, 0x1A00 ); /* list all TxPdo in 0x1C13:1-4 */
    ecrt_slave_config_sdo8(  sc, 0x1C13, 0, 1 ); /* set number of TxPDO */
   #endif
    //ecrt_slave_config_sdo8( sc, 0x6060, 0, 9);

    if (ecrt_slave_config_pdos(sc, EC_END, servo_syncs))
    {
        printf("Failed to configure servo PDOs!\n");
    }
    else
    {
        printf("*Success to configuring servo PDOs*\n");
    }

    ppdos->ctrl_word = ecrt_slave_config_reg_pdo_entry(sc,0x6040, 0, domain1, NULL);
    ppdos->operation_mode = ecrt_slave_config_reg_pdo_entry(sc,0x6060, 0, domain1, NULL);
    ppdos->Target_position = ecrt_slave_config_reg_pdo_entry(sc,0x607A, 0, domain1, NULL);
    ppdos->target_velocity = ecrt_slave_config_reg_pdo_entry(sc,0x60FF, 0, domain1, NULL);
    ppdos->torque_offset  =  ecrt_slave_config_reg_pdo_entry(sc,0x60B2, 0, domain1, NULL);

    ppdos->status_word = ecrt_slave_config_reg_pdo_entry(sc,0x6041, 0, domain1, NULL);
    ppdos->mode_display = ecrt_slave_config_reg_pdo_entry(sc,0x6061, 0, domain1, NULL);
    ppdos->Ext_Pos_value = ecrt_slave_config_reg_pdo_entry(sc,0x6064, 0, domain1, NULL);
    ppdos->velocity_value = ecrt_slave_config_reg_pdo_entry(sc,0x606c, 0, domain1, NULL);
    ppdos->Int_Pos_value = ecrt_slave_config_reg_pdo_entry(sc,0x2111, 2, domain1, NULL);
    ppdos->torque_act_value = ecrt_slave_config_reg_pdo_entry(sc,0x6077, 0, domain1, NULL);
    ppdos->torque_demand = ecrt_slave_config_reg_pdo_entry(sc,0x6074, 0, domain1, NULL);
    
    //ppdos->setting_time_now   =  ecrt_slave_config_reg_pdo_entry(sc,0x2010, 0x0a, domain1, NULL);
    //ppdos->damping_ratio_now   =  ecrt_slave_config_reg_pdo_entry(sc,0x2010, 0x0b, domain1, NULL);
}

static void confRS485GatewayPdos(ec_domain_t *domain1, ec_slave_info_t *inf, ec_slave_config_t *sc)
{
    // AMSAMOTION_EC_MB 网关不通过标准电机PDO控制
    // 如果后续需要通过PDO控制网关，在此处添加配置
    printf("配置RS485网关: %s (position=%d)\n", inf->name, inf->position);
    
    // 示例：如果网关有简单的输入输出PDO，可以在这里配置
    // 目前仅输出日志，不注册PDO条目
}

static void  upperFSASomanets(ARMS_MASTER_THREAD_INFO *pTModule,S_ETHR_STATE fsastate)
{
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        pTModule->arm_fsa[i] = fsastate;
    }
}

static void clearErrorSomanets(ARMS_MASTER_THREAD_INFO *pTModule, SUPR_R_MSG *mSMsg)
{
    for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    {
        if(mSMsg->mModuleFlag[i])
        {
            for (int j = 0; j < ARM_SLAVES_PER_ARM; j++)
            {
                // 跳过RS485网关
                if(g_slave_config[i*ARM_SLAVES_PER_ARM+j].type == SLAVE_TYPE_RS485_GATEWAY)
                    continue;
                pTModule->motors[i][j].target_velocity = 0;
                pTModule->motors[i][j].ctrl_word = 0x80;
            }
        }
    }
}

static int32_t handleStartSlight(ARMS_MASTER_THREAD_INFO *pTModule,int armsid,float v0,float v1,float v2,float v3,float v4,float v5,float acc_mm)
{
    pTModule->upper_target_v_Flag[armsid] = 0;
    // 6个电机：臂电机1, 臂电机2, 臂电机3, 托板电机1, 托板电机2, 托板电机3
    pTModule->upper_target_velocity[armsid][0] = v0; 
    pTModule->upper_target_velocity[armsid][1] = v1;
    pTModule->upper_target_velocity[armsid][2] = v2;
    pTModule->upper_target_velocity[armsid][3] = v3;
    pTModule->upper_target_velocity[armsid][4] = v4;
    pTModule->upper_target_velocity[armsid][5] = v5;

    pTModule->upper_velocity_v0[armsid][0] = pTModule->motors[armsid][0].velocity_value;
    pTModule->upper_velocity_v0[armsid][1] = pTModule->motors[armsid][1].velocity_value;
    pTModule->upper_velocity_v0[armsid][2] = pTModule->motors[armsid][2].velocity_value;
    pTModule->upper_velocity_v0[armsid][3] = pTModule->motors[armsid][3].velocity_value;
    pTModule->upper_velocity_v0[armsid][4] = pTModule->motors[armsid][4].velocity_value;
    pTModule->upper_velocity_v0[armsid][5] = pTModule->motors[armsid][5].velocity_value;
    
    // 200hz 调用周期，加减速1s
    for (int i = 0; i < ARM_MOTOR_COUNT; i++)
    {
        if(pTModule->upper_target_velocity[armsid][i] > pTModule->upper_velocity_v0[armsid][i])
            pTModule->upper_acc[armsid][i] = acc_mm;
        else if(pTModule->upper_target_velocity[armsid][i] < pTModule->upper_velocity_v0[armsid][i])
            pTModule->upper_acc[armsid][i] = -acc_mm;
        else
            pTModule->upper_acc[armsid][i] = 0;

        pTModule->upper_last_v_cmd[armsid][i] = pTModule->upper_velocity_v0[armsid][i];
    }
    pTModule->upper_target_v_Flag[armsid] = 1;
    return 0;
}

static void upperAngularVelSomanets(ARMS_MASTER_THREAD_INFO *pTModule, SUPR_R_MSG *mSMsg)
{
    memset(pTModule->upper_target_v_Flag, 0, sizeof(uint8_t)*MASTER_ARMS_MAX_SIZE);
    float *pVel = (float*)mSMsg->mData;
    for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    {
        if(mSMsg->mModuleFlag[i])
        {
            // 6个电机速度：臂电机1, 臂电机2, 臂电机3, 托板电机1, 托板电机2, 托板电机3
            handleStartSlight(pTModule, i, pVel[0], pVel[1], pVel[2], pVel[3], pVel[4], pVel[5], 200);
        }
    }
}

static void upperRelaPosSomanets(ARMS_MASTER_THREAD_INFO *pTModule, SUPR_R_MSG *mSMsg)
{
    for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    {
        if(mSMsg->mModuleFlag[i])
        {
            float *pRpos = (float*)mSMsg->mData;
            // 6个电机：臂电机1, 臂电机2, 臂电机3, 托板电机1, 托板电机2, 托板电机3
            pTModule->upper_target_pos[i][0] = pTModule->motors[i][0].Ext_Pos_value + pRpos[0];
            pTModule->upper_target_pos[i][1] = pTModule->motors[i][1].Ext_Pos_value + pRpos[1];
            pTModule->upper_target_pos[i][2] = pTModule->motors[i][2].Ext_Pos_value + pRpos[2];
            pTModule->upper_target_pos[i][3] = pTModule->motors[i][3].Ext_Pos_value + pRpos[3];
            pTModule->upper_target_pos[i][4] = pTModule->motors[i][4].Ext_Pos_value + pRpos[4];
            pTModule->upper_target_pos[i][5] = pTModule->motors[i][5].Ext_Pos_value + pRpos[5];
            
            // 位置控制的PD算法控制命令（速度），进行滤波平滑
            for (int j = 0; j < ARM_MOTOR_COUNT; j++)
            {
                for (int k = 0; k < upper_pos_smooth_cnt; k++)
                {
                    pTModule->upper_pos_vel_smooth[i][j][k] = 0;
                } 
                pTModule->target_p_last_cmdvel[i][j] = 0;
            }
            pTModule->upper_target_p_Flag[i] = 1;
            pTModule->upper_target_p_Kp_cnt[i] = 0;
        }
    }
}

static void upperAbsPosSomanets(ARMS_MASTER_THREAD_INFO *pTModule, SUPR_R_MSG *mSMsg)
{
    for (int i = 0; i < MASTER_ARMS_MAX_SIZE; i++)
    {
        if(mSMsg->mModuleFlag[i])
        {
            float *pRpos = (float*)mSMsg->mData;
            for (int j = 0; j < ARM_MOTOR_COUNT; j++)
            {
                pTModule->upper_target_pos[i][j] = pRpos[i*ARM_MOTOR_COUNT+j];
                for (int k = 0; k < upper_pos_smooth_cnt; k++)
                {
                    pTModule->upper_pos_vel_smooth[i][j][k] = 0;
                } 
                pTModule->target_p_last_cmdvel[i][j] = 0;
            }
            pTModule->upper_target_p_Flag[i] = 1;
            pTModule->upper_target_p_Kp_cnt[i] = 0;
        }
    }
}

static int confSlavesPdos(ARMS_MASTER_THREAD_INFO *pTModule)
{
    if(pTModule->armsSize < 1 || pTModule->armsSize > MASTER_ARMS_MAX_SIZE)
    {
        printf("confSlavesPdos:机械臂不存在\n\r");
        return -1;
    }

    // 根据静态配置表遍历所有从站
    for (int i = 0; i < TOTAL_ARM_SLAVES; i++)
    {
        const SLAVE_CONFIG_T *cfg = &g_slave_config[i];
        
        // 创建从站配置，使用绝对position（alias=0）
        ec_slave_config_t *ec_sla_con = ecrt_master_slave_config(
                                            pTModule->master, 
                                            cfg->alias,
                                            cfg->position, 
                                            cfg->vendor_id,
                                            cfg->product_code);

        if(ec_sla_con == NULL)
        {
            printf("ecrt_master_slave_config failed: 臂%d 位置%d %s\n",
                   cfg->arm_id, cfg->position, cfg->name);
            return -1;
        }
        
        // 查找该从站在链表中的对应节点，填充slave_conf
        EK1100_slaves_inf *inf_t = NULL;
        list_for_each_entry(inf_t, &pTModule->slaves_inf[cfg->arm_id].list, list)
        {
            if(inf_t->slave_in.position == cfg->position)
            {
                inf_t->slave_conf = ec_sla_con;
                break;
            }
        }

        // 根据从站类型配置不同的PDO
        switch(cfg->type)
        {
            case SLAVE_TYPE_ARM_MOTOR:
            case SLAVE_TYPE_PALLET_MOTOR:
            {
                // 臂电机和托板电机都使用SOMANET CSV模式PDO
                int motor_idx = i % ARM_SLAVES_PER_ARM;
                
                if(pTModule->somanetn_mode == MODE_CSV)
                {
                    confSomanetCSVPdos(pTModule->domain1, &inf_t->slave_in, ec_sla_con, 
                                     &pTModule->somanet_pdo[cfg->arm_id][motor_idx]);
                }
                printf("已配置臂%d 电机%d PDO (position=%d)\n", 
                       cfg->arm_id, motor_idx, cfg->position);
                break;
            }
            
            case SLAVE_TYPE_RS485_GATEWAY:
            {
                // RS485网关：目前不配置电机PDO
                // 如果该网关有特定PDO，需要在这里单独配置
                printf("RS485网关 臂%d 位置%d 跳过电机PDO配置\n", 
                       cfg->arm_id, cfg->position);
                break;
            }
            
            default:
            {
                printf("未知从站类型: 臂%d 位置%d\n", cfg->arm_id, cfg->position);
                break;
            }
        }
    }
    
    printf("###########################\n");
    return 0;
}

static int activeMater(ARMS_MASTER_THREAD_INFO *pTModule)
{
    int iRet = -1;
    if(pTModule->slaves_size < 1)
      return iRet;
    
    // 设置DC时钟：所有电机从站
    for (int i = 0; i < TOTAL_ARM_SLAVES; i++)
    {
        const SLAVE_CONFIG_T *cfg = &g_slave_config[i];
        
        // 只有电机从站需要DC
        if(cfg->type == SLAVE_TYPE_RS485_GATEWAY)
            continue;
        
        // 查找对应slave_conf
        EK1100_slaves_inf *inf_t = NULL;
        list_for_each_entry(inf_t, &pTModule->slaves_inf[cfg->arm_id].list, list)
        {
            if(inf_t->slave_in.position == cfg->position)
            {
                if(inf_t->slave_conf != NULL)
                {
                    ecrt_slave_config_dc(inf_t->slave_conf, 0x0700, 1000000, 500000, 0, 0);
                    printf("臂%d 电机(position=%d) 已配置DC时钟\n", cfg->arm_id, cfg->position);
                }
                break;
            }
        }
    }

    // 选择第一个电机从站作为参考时钟（避免RS485网关作为参考时钟）
    EK1100_slaves_inf *ref_slave = NULL;
    list_for_each_entry(ref_slave, &pTModule->slaves_inf[0].list, list)
    {
        if(ref_slave->slave_conf != NULL && 
           g_slave_config[ref_slave->slave_in.position % TOTAL_ARM_SLAVES].type != SLAVE_TYPE_RS485_GATEWAY)
        {
            int ret = ecrt_master_select_reference_clock(pTModule->master, ref_slave->slave_conf);
            if (ret < 0) {
                printf("Failed to select reference clock\n");
                return iRet;
            }
            printf("参考时钟: 臂0 电机从站 position=%d\n", ref_slave->slave_in.position);
            break;
        }
    }

    printf("Activating master...\n");
    if (ecrt_master_activate(pTModule->master)) {
        return iRet;
    }
    else
    {
        printf("*Master activated*\n");
    }
    if (!(pTModule->domain1_pd = ecrt_domain_data(pTModule->domain1))) {
        printf("ecrt_domain_data EXIT_FAILURE\n");
    }
    printf("starting 1000HZ pdo\n");
    // 开启1000HZ循环
    pTModule->masterActivated=true;

    return 0;
}

static void  activeMaterKeydown(ARMS_MASTER_THREAD_INFO *pTModule)
{
    if(createMaster(pTModule->masterId,&pTModule->master,&pTModule->domain1) != 0)
        return;

    if(scanSlaves(pTModule) != 0)
        return;

    if(confSlavesPdos(pTModule) != 0)
        return;

    if(activeMater(pTModule) != 0)
        return;
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
        case CMD_MASTER_OP_KEYDOWN:
        {
          //只能使用速度模式，因为驱动器pos设置的是外部编码器
            pTModule->somanetn_mode = MODE_CSV;
            activeMaterKeydown(pTModule);
            //打开拉力计串口
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
        case CMD_IGH_SCAN_SLAVES:
        {
            scanSlaves(pTModule);
            break;
        }
        case CMD_IGH_FSA_SHUTDOWN:
        {
            upperFSASomanets(pTModule,S_SHUTDOWN);
            break;
        }
        case CMD_IGH_FSA_SWITH_ON:
        {
            upperFSASomanets(pTModule,S_SWITCH_ON);
            break;
        }
        case CMD_IGH_FSA_ENOP:
        {
            upperFSASomanets(pTModule,S_ENABLE_OP);
            break;
        }
        case CMD_IGH_FSA_HALT:
        {
            upperFSASomanets(pTModule,S_HAIT);
            break;
        }
        case CMD_IGH_START_OP:
        {
            activeMater(pTModule);
            break;
        }   
        case CMD_IGH_CLEAR_ERROR:
        {
            MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
            clearErrorSomanets(pTModule,(SUPR_R_MSG *)mLMsg->mData);
            break;
        }
        case CMD_HAND_VEL_MM:
        {
            MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
            memset(pTModule->upper_target_p_Flag,0,sizeof(uint8_t)*MASTER_ARMS_MAX_SIZE);
            upperAngularVelSomanets(pTModule,(SUPR_R_MSG *)mLMsg->mData);
            
            break;
        }
        case CMD_HAND_RELA_POS_MM:
        {
            MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
            upperRelaPosSomanets(pTModule,(SUPR_R_MSG *)mLMsg->mData);
            break;
        }
        case CMD_HAND_ABS_POS_M:
        {
            MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
            upperAbsPosSomanets(pTModule,(SUPR_R_MSG *)mLMsg->mData);
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
    // 读取交换somanet的数据
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        if(pTModule->slaves_inf[i].childSlaveSize != ARM_SLAVES_PER_ARM)
            continue;

        for (int j = 0; j < ARM_SLAVES_PER_ARM; j++)
        {
            // 跳过RS485网关
            if(g_slave_config[i*ARM_SLAVES_PER_ARM+j].type == SLAVE_TYPE_RS485_GATEWAY)
                continue;
                
            // CSV、CSP模式下都需要刷新
            pTModule->motors[i][j].status_word    = EC_READ_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].status_word);
            pTModule->motors[i][j].velocity_value = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].velocity_value);
            pTModule->motors[i][j].Ext_Pos_value = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].Ext_Pos_value);
            pTModule->motors[i][j].Int_Pos_value = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].Int_Pos_value);
            
            // CVS模式下，算法要用到力矩偏置
            pTModule->motors[i][j].torque_act_value = EC_READ_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].torque_act_value);
            pTModule->motors[i][j].torque_demand = EC_READ_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].torque_demand);
        }
    }
}

static void reflash_rpdo_Ebus(ARMS_MASTER_THREAD_INFO *pTModule)
{
    // 写入交换somanet的数据，1ms刷新写入一次
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        if(pTModule->slaves_inf[i].childSlaveSize != ARM_SLAVES_PER_ARM)
            continue;

        if(pTModule->somanetn_mode == MODE_CSV)
        {
            for (int j = 0; j < ARM_SLAVES_PER_ARM; j++)
            {
                // 跳过RS485网关
                if(g_slave_config[i*ARM_SLAVES_PER_ARM+j].type == SLAVE_TYPE_RS485_GATEWAY)
                    continue;
                    
                EC_WRITE_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].ctrl_word, pTModule->motors[i][j].ctrl_word);
                EC_WRITE_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].operation_mode, pTModule->motors[i][j].operation_mode);
                EC_WRITE_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].target_velocity, pTModule->motors[i][j].target_velocity);
                EC_WRITE_S16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].torque_offset, pTModule->motors[i][j].torque_offset);
            }
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