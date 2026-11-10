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
#include <stddef.h>
#include <math.h>
#include <inttypes.h>
#include <unistd.h>

#include "mot_master.h"
#include "mot_leader_defs.h"
#include "mot_share_ipc.h"
#include "mot_ctl.h"
#include "mot_commu_ipc.h"
#include "mot_share_daemon.h"





/* Match sysEthPrj_Gene: 20-bit encoder, 101:1, 10 deg/s speed and 30 deg/s^2 ramp. */
#define CSV_COUNTS_PER_DEG (1048576.0 * 101.0 / 360.0)
#define CSV_MAX_DEG_PER_SEC 10.0f
#define CSV_ACC_DEG_PER_SEC2 30.0f

static void resetVelocityRamp(ARMS_MASTER_THREAD_INFO *module, int arm)
{
    module->upper_target_v_Flag[arm] = 0;
    for (int motor = 0; motor < ARM_MOTOR_COUNT; ++motor) {
        module->upper_target_velocity[arm][motor] = 0;
        module->upper_velocity_v0[arm][motor] = 0;
        module->upper_last_v_cmd[arm][motor] = 0;
        module->upper_acc[arm][motor] = 0;
        module->motors[arm][motor].target_velocity = 0;
    }
}

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
static void sendTelemetry(ARMS_MASTER_THREAD_INFO *pTModule);
static void pollEL6022(ARMS_MASTER_THREAD_INFO *pTModule);
static uint16_t modbusCrc16(const uint8_t *data, size_t length);

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
static int confSomanetCSVPdos(ec_domain_t *domain1,ec_slave_info_t *inf,ec_slave_config_t *sc,motec_pdos *ppdos);
static void confEL2809Pdos(ARMS_MASTER_THREAD_INFO *pTModule,ec_slave_info_t *inf,ec_slave_config_t *sc,int EK1100Index);
static void confEL6022Pdos(ARMS_MASTER_THREAD_INFO *pTModule,ec_slave_info_t *inf,ec_slave_config_t *sc,int EK1100Index);


static void  destroyMaster(ARMS_MASTER_THREAD_INFO *pTModule)
{
    for (int arm = 0; arm < pTModule->armsSize; ++arm)
        resetVelocityRamp(pTModule, arm);
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

  memset(pTModule->arm_motor_count, 0, sizeof(pTModule->arm_motor_count));
    memset(pTModule->EL2809, 0xff, sizeof(pTModule->EL2809));
    memset(pTModule->EL6022, 0xff, sizeof(pTModule->EL6022));

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

  for (int arm = 0; arm < MASTER_ARMS_MAX_SIZE; ++arm) {
      // 通道 0 (Port 0)：3个角度编码器，地址 1, 2, 3，读 0x0014 起 2个寄存器，周期 30ms
      pTModule->el6022_sensor_cfg[arm][0][0][0] = 1; pTModule->el6022_sensor_cfg[arm][0][0][1] = 0x0014; pTModule->el6022_sensor_cfg[arm][0][0][2] = 2; pTModule->el6022_sensor_cfg[arm][0][0][3] = 30;
      pTModule->el6022_sensor_cfg[arm][0][1][0] = 2; pTModule->el6022_sensor_cfg[arm][0][1][1] = 0x0014; pTModule->el6022_sensor_cfg[arm][0][1][2] = 2; pTModule->el6022_sensor_cfg[arm][0][1][3] = 30;
      pTModule->el6022_sensor_cfg[arm][0][2][0] = 3; pTModule->el6022_sensor_cfg[arm][0][2][1] = 0x0014; pTModule->el6022_sensor_cfg[arm][0][2][2] = 2; pTModule->el6022_sensor_cfg[arm][0][2][3] = 30;

      // 通道 1 (Port 1)：3个压力 + 1个拉线，地址 1, 2, 3, 4，读 0x0014 起 2个寄存器，周期 100ms
      pTModule->el6022_sensor_cfg[arm][1][0][0] = 1; pTModule->el6022_sensor_cfg[arm][1][0][1] = 0x0014; pTModule->el6022_sensor_cfg[arm][1][0][2] = 2; pTModule->el6022_sensor_cfg[arm][1][0][3] = 100;
      pTModule->el6022_sensor_cfg[arm][1][1][0] = 2; pTModule->el6022_sensor_cfg[arm][1][1][1] = 0x0014; pTModule->el6022_sensor_cfg[arm][1][1][2] = 2; pTModule->el6022_sensor_cfg[arm][1][1][3] = 100;
      pTModule->el6022_sensor_cfg[arm][1][2][0] = 3; pTModule->el6022_sensor_cfg[arm][1][2][1] = 0x0014; pTModule->el6022_sensor_cfg[arm][1][2][2] = 2; pTModule->el6022_sensor_cfg[arm][1][2][3] = 100;
      pTModule->el6022_sensor_cfg[arm][1][3][0] = 4; pTModule->el6022_sensor_cfg[arm][1][3][1] = 0x0014; pTModule->el6022_sensor_cfg[arm][1][3][2] = 2; pTModule->el6022_sensor_cfg[arm][1][3][3] = 100;
      
      // 初始化状态机
      for (int port = 0; port < 2; ++port) {
          pTModule->el6022_state[arm][port] = EL6022_ST_INIT;
          pTModule->el6022_inited[arm][port] = 0;
          pTModule->el6022_sensor_index[arm][port] = 0;
          pTModule->el6022_next_poll_ms[arm][port] = 0;
      }
  }


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

    int EK1100size = 0;

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

    for (int i = 0; i < actual_slave_size; i++) {
     
        ec_slave_info_t slave_in;

        // 从主站获取该position的实际从站信息
        if(ecrt_master_get_slave(pTModule->master, i, &slave_in) < 0)
        {
            printf("获取从站 %d 信息失败\n", i);
            break;
        }

        // 分配从站信息节点
        EK1100_slaves_inf *childSlave = (EK1100_slaves_inf*)malloc(sizeof(EK1100_slaves_inf));
        memset((char*)childSlave, 0, sizeof(EK1100_slaves_inf));
        memcpy(&childSlave->slave_in, &slave_in, sizeof(ec_slave_info_t));

        if (strstr(slave_in.name, "EK1100") != NULL)
        {
            EK1100size++;
        }else{

        }

        childSlave->masteri = pTModule->masterId;

        // 添加到对应臂的链表
        list_add_tail(&childSlave->list, &pTModule->slaves_inf[EK1100size-1].list);
        pTModule->slaves_inf[EK1100size-1].childSlaveSize++;

        printf("&&&&&&& %s %d %d \n", childSlave->slave_in.name,childSlave->slave_in.alias,childSlave->slave_in.position );
    }
    //设置从站个数、arms的个数
    pTModule->slaves_size = actual_slave_size;
    pTModule->armsSize = EK1100size;
    printf("扫描完成: 总共%d个从站, %d套臂\n", 
             pTModule->slaves_size, pTModule->armsSize);
    return 0;
}


static int confSomanetCSVPdos(ec_domain_t *domain1,ec_slave_info_t *inf,ec_slave_config_t *sc,motec_pdos *ppdos)
{
    /*Config PDOs*/
    uint8_t rpdo_entries_size = 4;
    uint8_t tpdo_entries_size = 6;
    ec_pdo_entry_info_t rpdo_entries[] = {
        /*RxPdo 0x1600*/
        {0x6040, 0x00, 16}, /* C */
        {0x607a, 0x00, 32}, /* T */
        {0x60ff, 0x00, 32}, /* T */
        {0x6071, 0x00, 16}, /* T */
    };
    ec_pdo_entry_info_t tpdo_entries[] = {
        /*TxPdo 0x1A00*/
        {0x6041, 0x00, 16}, /* S */
        {0x6064, 0x00, 32}, /* P */
        {0x606c, 0x00, 32}, /* V */
        {0x6077, 0x00, 16}, /* T */
        {0x60f4, 0x00, 32}, /* F */
        {0x603f, 0x00, 16}, /* E */
    };

    ec_pdo_info_t ec_rpdos[] = {
        //RxPdo
        {0x1601, rpdo_entries_size, rpdo_entries + 0 }, /*recive pdo maping*/
    };
    ec_pdo_info_t ec_tpdos[] = {
        //TxPdo
        {0x1A01, tpdo_entries_size, tpdo_entries + 0 } /*send pdo maping*/
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
    // ecrt_slave_config_sdo16( sc, 0x6040, 0, 0x80 );
    // ecrt_slave_config_sdo16( sc, 0x2010, 0x0a, 0x07D0);
    // ecrt_slave_config_sdo16( sc, 0x2010, 0x0b, 0x07D0);
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
    if (ecrt_slave_config_sdo8(sc, 0x6060, 0, 9) < 0)
        return -1;

    if (ecrt_slave_config_pdos(sc, EC_END, servo_syncs))
    {
        fprintf(stderr, "Failed to configure servo PDOs: slave=%u\n", inf->position);
        return -1;
    }
    else
    {
        printf("*Success to configuring servo PDOs*\n");
    }

    ppdos->ctrl_word = ecrt_slave_config_reg_pdo_entry(sc,0x6040, 0, domain1, NULL);
    // ppdos->operation_mode = ecrt_slave_config_reg_pdo_entry(sc,0x6060, 0, domain1, NULL);
    ppdos->target_position = ecrt_slave_config_reg_pdo_entry(sc,0x607A, 0, domain1, NULL);
    ppdos->target_velocity = ecrt_slave_config_reg_pdo_entry(sc,0x60FF, 0, domain1, NULL);
    ppdos->target_torque  =  ecrt_slave_config_reg_pdo_entry(sc,0x6071, 0, domain1, NULL);

    ppdos->status_word = ecrt_slave_config_reg_pdo_entry(sc,0x6041, 0, domain1, NULL);
    ppdos->Position_value = ecrt_slave_config_reg_pdo_entry(sc,0x6064, 0, domain1, NULL);
    ppdos->velocity_value = ecrt_slave_config_reg_pdo_entry(sc,0x606c, 0, domain1, NULL);
    ppdos->torque_act_value = ecrt_slave_config_reg_pdo_entry(sc,0x6077, 0, domain1, NULL);
    ppdos->following_error = ecrt_slave_config_reg_pdo_entry(sc,0x60f4, 0, domain1, NULL);
    ppdos->error_code = ecrt_slave_config_reg_pdo_entry(sc,0x603f, 0, domain1, NULL);
    // ppdos->torque_demand = ecrt_slave_config_reg_pdo_entry(sc,0x6074, 0, domain1, NULL);
    
    //ppdos->setting_time_now   =  ecrt_slave_config_reg_pdo_entry(sc,0x2010, 0x0a, domain1, NULL);
    //ppdos->damping_ratio_now   =  ecrt_slave_config_reg_pdo_entry(sc,0x2010, 0x0b, domain1, NULL);
    const int offsets[] = {ppdos->ctrl_word, ppdos->target_position,
        ppdos->target_velocity, ppdos->target_torque, ppdos->status_word,
        ppdos->Position_value, ppdos->velocity_value, ppdos->torque_act_value,
        ppdos->following_error, ppdos->error_code};
    for (size_t entry = 0; entry < sizeof(offsets) / sizeof(offsets[0]); ++entry) {
        if (offsets[entry] < 0) {
            fprintf(stderr, "Servo slave=%u PDO registration entry=%zu failed: %d\n",
                    inf->position, entry, offsets[entry]);
            return -1;
        }
    }
    printf("Servo slave=%u offsets cw=%d tp=%d tv=%d tq=%d sw=%d pos=%d vel=%d torque=%d following=%d error=%d\n",
           inf->position, offsets[0], offsets[1], offsets[2], offsets[3], offsets[4],
           offsets[5], offsets[6], offsets[7], offsets[8], offsets[9]);
    /* Read mode through the mailbox; keep the ESI's default TxPDO layout. */
    ppdos->mode_display = -1;
    ppdos->mode_request_state = EC_REQUEST_UNUSED;
    ppdos->mode_display_request = ecrt_slave_config_create_sdo_request(sc, 0x6061, 0, 1);
    if (ppdos->mode_display_request)
        ecrt_sdo_request_timeout(ppdos->mode_display_request, 500);
    else
        fprintf(stderr, "Servo slave=%u: 0x6061 diagnostic request unavailable\n", inf->position);
    return 0;
}



static void  upperFSASomanets(ARMS_MASTER_THREAD_INFO *pTModule,S_ETHR_STATE fsastate)
{
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        resetVelocityRamp(pTModule, i);
        pTModule->arm_fsa[i] = fsastate;
    }
}

static void clearErrorSomanets(ARMS_MASTER_THREAD_INFO *pTModule, SUPR_R_MSG *mSMsg)
{
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        if (mSMsg->mModuleFlag[i])
        {
            resetVelocityRamp(pTModule, i);
            pTModule->arm_fsa[i] = S_CLEAR_ERROR;
            for (int j = 0; j < pTModule->arm_motor_count[i]; j++)
            {
                pTModule->motors[i][j].target_velocity = 0;
                pTModule->motors[i][j].ctrl_word = 0x0080;
            }
        }
    }
}

static int32_t handleStartSlight(ARMS_MASTER_THREAD_INFO *pTModule,int armsid,float v0,float v1,float v2,float v3,float v4,float v5,float acc_mm)
{
    if (armsid < 0 || armsid >= pTModule->armsSize)
        return -1;
    pTModule->upper_target_v_Flag[armsid] = 0;
    const float requested[ARM_MOTOR_COUNT] = {v0, v1, v2, v3, v4, v5};
    for (int i = 0; i < pTModule->arm_motor_count[armsid]; ++i) {
        pTModule->upper_target_velocity[armsid][i] = requested[i];
        pTModule->upper_velocity_v0[armsid][i] = pTModule->upper_last_v_cmd[armsid][i];
    }
    
    // 200hz 调用周期，加减速1s
    for (int i = 0; i < pTModule->arm_motor_count[armsid]; i++)
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
    if (mSMsg->mDataLength < sizeof(float) * ARM_MOTOR_COUNT * MASTER_ARMS_MAX_SIZE)
        return;
    const float *pVel = (const float *)mSMsg->mData;
    for (int i = 0; i < MASTER_ARMS_MAX_SIZE * ARM_MOTOR_COUNT; ++i) {
        if (!isfinite(pVel[i]) || fabsf(pVel[i]) > CSV_MAX_DEG_PER_SEC) {
            fprintf(stderr, "Rejected velocity command: index=%d requested=%g deg/s; allowed finite [-%g, +%g] deg/s; targets unchanged\n",
                    i, pVel[i], CSV_MAX_DEG_PER_SEC, CSV_MAX_DEG_PER_SEC);
            return;
        }
    }
    memset(pTModule->upper_target_p_Flag, 0, sizeof(uint8_t) * MASTER_ARMS_MAX_SIZE);
    for (int i = 0; i < pTModule->armsSize; ++i) {
        if (mSMsg->mModuleFlag[i]) {
            pTModule->upper_target_p_Flag[i] = 0;
            const float *armVel = pVel + i * ARM_MOTOR_COUNT;
            handleStartSlight(pTModule, i, armVel[0], armVel[1], armVel[2],
                              armVel[3], armVel[4], armVel[5], CSV_ACC_DEG_PER_SEC2);
        }
    }
}

static void upperRelaPosSomanets(ARMS_MASTER_THREAD_INFO *pTModule, SUPR_R_MSG *mSMsg)
{
    const size_t expected = sizeof(float) * ARM_MOTOR_COUNT * MASTER_ARMS_MAX_SIZE;
    if (mSMsg->mDataLength != expected)
        return;
    const float *positions = (const float *)mSMsg->mData;
    for (int arm = 0; arm < pTModule->armsSize; ++arm) {
        if (!mSMsg->mModuleFlag[arm])
            continue;
        const float *arm_positions = positions + arm * ARM_MOTOR_COUNT;
        for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor) {
            if (!isfinite(arm_positions[motor]) || fabsf(arm_positions[motor]) > 5.0f) {
                fprintf(stderr, "Rejected relative position: arm=%d motor=%d value=%g deg; allowed +/-5 deg\n",
                        arm, motor, arm_positions[motor]);
                return;
            }
        }
    }
    for (int arm = 0; arm < pTModule->armsSize; ++arm) {
        if (!mSMsg->mModuleFlag[arm])
            continue;
        const float *arm_positions = positions + arm * ARM_MOTOR_COUNT;
        for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor) {
            float actual_deg = pTModule->motors[arm][motor].Position_value / (float)CSV_COUNTS_PER_DEG;
            pTModule->upper_target_pos[arm][motor] = actual_deg + arm_positions[motor];
            pTModule->target_p_last_cmdvel[arm][motor] = 0.0f;
        }
        pTModule->upper_target_v_Flag[arm] = 0;
        pTModule->upper_target_p_Flag[arm] = 1;
    }
}

static void upperAbsPosSomanets(ARMS_MASTER_THREAD_INFO *pTModule, SUPR_R_MSG *mSMsg)
{
    const size_t expected = sizeof(float) * ARM_MOTOR_COUNT * MASTER_ARMS_MAX_SIZE;
    if (mSMsg->mDataLength != expected)
        return;
    const float *positions = (const float *)mSMsg->mData;
    for (int arm = 0; arm < pTModule->armsSize; ++arm) {
        if (!mSMsg->mModuleFlag[arm])
            continue;
        const float *arm_positions = positions + arm * ARM_MOTOR_COUNT;
        for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor) {
            float actual_deg = pTModule->motors[arm][motor].Position_value / (float)CSV_COUNTS_PER_DEG;
            if (!isfinite(arm_positions[motor]) || fabsf(arm_positions[motor] - actual_deg) > 5.0f) {
                fprintf(stderr, "Rejected absolute position: arm=%d motor=%d target=%g deg actual=%g deg; move limit +/-5 deg\n",
                        arm, motor, arm_positions[motor], actual_deg);
                return;
            }
        }
    }
    for (int arm = 0; arm < pTModule->armsSize; ++arm) {
        if (!mSMsg->mModuleFlag[arm])
            continue;
        const float *arm_positions = positions + arm * ARM_MOTOR_COUNT;
        for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor) {
            pTModule->upper_target_pos[arm][motor] = arm_positions[motor];
            pTModule->target_p_last_cmdvel[arm][motor] = 0.0f;
        }
        pTModule->upper_target_p_Flag[arm] = 1;
    }
}

static void confEL2809Pdos(ARMS_MASTER_THREAD_INFO *pTModule,ec_slave_info_t *inf,ec_slave_config_t *sc,int EK1100Index)
{
    for (int ch = 0; ch < 16; ++ch) {
        pTModule->EL2809[EK1100Index][ch] = -1;
        pTModule->EL2809_bit[EK1100Index][ch] = 0;
        pTModule->EL2809_output[EK1100Index][ch] = 0;
    }

    ec_pdo_entry_info_t slave_el2809_pdo_entries[] = {
        {0x7000, 0x01, 1}, /* Output */
        {0x7010, 0x01, 1}, /* Output */
        {0x7020, 0x01, 1}, /* Output */
        {0x7030, 0x01, 1}, /* Output */
        {0x7040, 0x01, 1}, /* Output */
        {0x7050, 0x01, 1}, /* Output */
        {0x7060, 0x01, 1}, /* Output */
        {0x7070, 0x01, 1}, /* Output */
        {0x7080, 0x01, 1}, /* Output */
        {0x7090, 0x01, 1}, /* Output */
        {0x70a0, 0x01, 1}, /* Output */
        {0x70b0, 0x01, 1}, /* Output */
        {0x70c0, 0x01, 1}, /* Output */
        {0x70d0, 0x01, 1}, /* Output */
        {0x70e0, 0x01, 1}, /* Output */
        {0x70f0, 0x01, 1}, /* Output */
    };

    ec_pdo_info_t slave_el2809_pdos[] = {
        {0x1600, 1, slave_el2809_pdo_entries + 0}, /* Channel 1 */
        {0x1601, 1, slave_el2809_pdo_entries + 1}, /* Channel 2 */
        {0x1602, 1, slave_el2809_pdo_entries + 2}, /* Channel 3 */
        {0x1603, 1, slave_el2809_pdo_entries + 3}, /* Channel 4 */
        {0x1604, 1, slave_el2809_pdo_entries + 4}, /* Channel 5 */
        {0x1605, 1, slave_el2809_pdo_entries + 5}, /* Channel 6 */
        {0x1606, 1, slave_el2809_pdo_entries + 6}, /* Channel 7 */
        {0x1607, 1, slave_el2809_pdo_entries + 7}, /* Channel 8 */
        {0x1608, 1, slave_el2809_pdo_entries + 8}, /* Channel 9 */
        {0x1609, 1, slave_el2809_pdo_entries + 9}, /* Channel 10 */
        {0x160a, 1, slave_el2809_pdo_entries + 10}, /* Channel 11 */
        {0x160b, 1, slave_el2809_pdo_entries + 11}, /* Channel 12 */
        {0x160c, 1, slave_el2809_pdo_entries + 12}, /* Channel 13 */
        {0x160d, 1, slave_el2809_pdo_entries + 13}, /* Channel 14 */
        {0x160e, 1, slave_el2809_pdo_entries + 14}, /* Channel 15 */
        {0x160f, 1, slave_el2809_pdo_entries + 15}, /* Channel 16 */
    };

    ec_sync_info_t slave_el2809_syncs[] = {
        {0, EC_DIR_OUTPUT, 8, slave_el2809_pdos + 0, EC_WD_ENABLE},
        {1, EC_DIR_OUTPUT, 8, slave_el2809_pdos + 8, EC_WD_ENABLE},
        {0xff}
    };

    if (ecrt_slave_config_pdos(sc, EC_END, slave_el2809_syncs))
    {
        printf("Failed to configure EL2809 PDOs!\n");
        return;
    }
    printf("*Success to configuring EL2809 PDOs*\n");

    // 按 Sync Manager 和 PDO 位置注册 16 个通道，避免后 8 路被映射到第一组。
    // for (int ch = 0; ch < 16; ch++) {
    //     unsigned int bit_pos = 0;
    //     unsigned int sync_index = ch < 8 ? 0 : 1;
    //     unsigned int pdo_position = (unsigned int)(ch % 8);
    //     int byte_offset = ecrt_slave_config_reg_pdo_entry_pos(
    //         sc, sync_index, pdo_position, 0, pTModule->domain1, &bit_pos);
    //     pTModule->EL2809[EK1100Index][ch] = byte_offset;
    //     pTModule->EL2809_bit[EK1100Index][ch] = (uint8_t)bit_pos;
    //     if (byte_offset >= 0 && bit_pos > 7) {
    //         fprintf(stderr, "EL2809 arm=%d channel=%d invalid bit offset %u\\n",
    //                 EK1100Index, ch + 1, bit_pos);
    //     }
    //     printf("EL2809 arm=%d channel=%d sync=%u pdo=%u offset=%d bit=%u\n",
    //            EK1100Index, ch + 1, sync_index, pdo_position,
    //            byte_offset, bit_pos);
    // }
        // 注册16个通道的偏移量
    for (int ch = 0; ch < 16; ch++) {
        unsigned int bit_pos = 0;
        int byte_offset = ecrt_slave_config_reg_pdo_entry(
            sc,
            0x7000 + (ch * 0x10),  // 0x7000, 0x7010, ..., 0x70f0
            0x01,
            pTModule->domain1,
            &bit_pos);
            
        pTModule->EL2809[EK1100Index][ch] = byte_offset;
        pTModule->EL2809_bit[EK1100Index][ch] = (uint8_t)bit_pos;
        
        // 打印验证，确保 offset 不是负数
        printf("EL2809 arm=%d channel=%d offset=%d bit=%u\n", EK1100Index, ch + 1, byte_offset, bit_pos);
    }

}

static void confEL6022Pdos(ARMS_MASTER_THREAD_INFO *pTModule,ec_slave_info_t *inf,ec_slave_config_t *sc,int EK1100Index)
{
    ec_pdo_entry_info_t slave_el6022_pdo_entries[] = {
        {0x7001, 0x01, 16}, /* Ctrl */
        {0x7000, 0x11, 8}, /* Data Out 0 */
        {0x7000, 0x12, 8}, /* Data Out 1 */
        {0x7000, 0x13, 8}, /* Data Out 2 */
        {0x7000, 0x14, 8}, /* Data Out 3 */
        {0x7000, 0x15, 8}, /* Data Out 4 */
        {0x7000, 0x16, 8}, /* Data Out 5 */
        {0x7000, 0x17, 8}, /* Data Out 6 */
        {0x7000, 0x18, 8}, /* Data Out 7 */
        {0x7000, 0x19, 8}, /* Data Out 8 */
        {0x7000, 0x1a, 8}, /* Data Out 9 */
        {0x7000, 0x1b, 8}, /* Data Out 10 */
        {0x7000, 0x1c, 8}, /* Data Out 11 */
        {0x7000, 0x1d, 8}, /* Data Out 12 */
        {0x7000, 0x1e, 8}, /* Data Out 13 */
        {0x7000, 0x1f, 8}, /* Data Out 14 */
        {0x7000, 0x20, 8}, /* Data Out 15 */
        {0x7000, 0x21, 8}, /* Data Out 16 */
        {0x7000, 0x22, 8}, /* Data Out 17 */
        {0x7000, 0x23, 8}, /* Data Out 18 */
        {0x7000, 0x24, 8}, /* Data Out 19 */
        {0x7000, 0x25, 8}, /* Data Out 20 */
        {0x7000, 0x26, 8}, /* Data Out 21 */
        {0x7011, 0x01, 16}, /* Ctrl */
        {0x7010, 0x11, 8}, /* Data Out 0 */
        {0x7010, 0x12, 8}, /* Data Out 1 */
        {0x7010, 0x13, 8}, /* Data Out 2 */
        {0x7010, 0x14, 8}, /* Data Out 3 */
        {0x7010, 0x15, 8}, /* Data Out 4 */
        {0x7010, 0x16, 8}, /* Data Out 5 */
        {0x7010, 0x17, 8}, /* Data Out 6 */
        {0x7010, 0x18, 8}, /* Data Out 7 */
        {0x7010, 0x19, 8}, /* Data Out 8 */
        {0x7010, 0x1a, 8}, /* Data Out 9 */
        {0x7010, 0x1b, 8}, /* Data Out 10 */
        {0x7010, 0x1c, 8}, /* Data Out 11 */
        {0x7010, 0x1d, 8}, /* Data Out 12 */
        {0x7010, 0x1e, 8}, /* Data Out 13 */
        {0x7010, 0x1f, 8}, /* Data Out 14 */
        {0x7010, 0x20, 8}, /* Data Out 15 */
        {0x7010, 0x21, 8}, /* Data Out 16 */
        {0x7010, 0x22, 8}, /* Data Out 17 */
        {0x7010, 0x23, 8}, /* Data Out 18 */
        {0x7010, 0x24, 8}, /* Data Out 19 */
        {0x7010, 0x25, 8}, /* Data Out 20 */
        {0x7010, 0x26, 8}, /* Data Out 21 */
        {0x6001, 0x01, 16}, /* Status */
        {0x6000, 0x11, 8}, /* Data In 0 */
        {0x6000, 0x12, 8}, /* Data In 1 */
        {0x6000, 0x13, 8}, /* Data In 2 */
        {0x6000, 0x14, 8}, /* Data In 3 */
        {0x6000, 0x15, 8}, /* Data In 4 */
        {0x6000, 0x16, 8}, /* Data In 5 */
        {0x6000, 0x17, 8}, /* Data In 6 */
        {0x6000, 0x18, 8}, /* Data In 7 */
        {0x6000, 0x19, 8}, /* Data In 8 */
        {0x6000, 0x1a, 8}, /* Data In 9 */
        {0x6000, 0x1b, 8}, /* Data In 10 */
        {0x6000, 0x1c, 8}, /* Data In 11 */
        {0x6000, 0x1d, 8}, /* Data In 12 */
        {0x6000, 0x1e, 8}, /* Data In 13 */
        {0x6000, 0x1f, 8}, /* Data In 14 */
        {0x6000, 0x20, 8}, /* Data In 15 */
        {0x6000, 0x21, 8}, /* Data In 16 */
        {0x6000, 0x22, 8}, /* Data In 17 */
        {0x6000, 0x23, 8}, /* Data In 18 */
        {0x6000, 0x24, 8}, /* Data In 19 */
        {0x6000, 0x25, 8}, /* Data In 20 */
        {0x6000, 0x26, 8}, /* Data In 21 */
        {0x6011, 0x01, 16}, /* Status */
        {0x6010, 0x11, 8}, /* Data In 0 */
        {0x6010, 0x12, 8}, /* Data In 1 */
        {0x6010, 0x13, 8}, /* Data In 2 */
        {0x6010, 0x14, 8}, /* Data In 3 */
        {0x6010, 0x15, 8}, /* Data In 4 */
        {0x6010, 0x16, 8}, /* Data In 5 */
        {0x6010, 0x17, 8}, /* Data In 6 */
        {0x6010, 0x18, 8}, /* Data In 7 */
        {0x6010, 0x19, 8}, /* Data In 8 */
        {0x6010, 0x1a, 8}, /* Data In 9 */
        {0x6010, 0x1b, 8}, /* Data In 10 */
        {0x6010, 0x1c, 8}, /* Data In 11 */
        {0x6010, 0x1d, 8}, /* Data In 12 */
        {0x6010, 0x1e, 8}, /* Data In 13 */
        {0x6010, 0x1f, 8}, /* Data In 14 */
        {0x6010, 0x20, 8}, /* Data In 15 */
        {0x6010, 0x21, 8}, /* Data In 16 */
        {0x6010, 0x22, 8}, /* Data In 17 */
        {0x6010, 0x23, 8}, /* Data In 18 */
        {0x6010, 0x24, 8}, /* Data In 19 */
        {0x6010, 0x25, 8}, /* Data In 20 */
        {0x6010, 0x26, 8}, /* Data In 21 */
    };

    ec_pdo_info_t slave_el6022_pdos[] = {
        {0x1604, 23, slave_el6022_pdo_entries + 0}, /* COM RxPDO-Map Outputs Ch.1 */
        {0x1605, 23, slave_el6022_pdo_entries + 23}, /* COM RxPDO-Map Outputs Ch.2 */
        {0x1a04, 23, slave_el6022_pdo_entries + 46}, /* COM TxPDO-Map Inputs Ch.1 */
        {0x1a05, 23, slave_el6022_pdo_entries + 69}, /* COM TxPDO-Map Inputs Ch.2 */
    };

    ec_sync_info_t slave_el6022_syncs[] = {
        {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
        {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
        {2, EC_DIR_OUTPUT, 2, slave_el6022_pdos + 0, EC_WD_DISABLE},
        {3, EC_DIR_INPUT, 2, slave_el6022_pdos + 2, EC_WD_DISABLE},
        {0xff}
    };

    if (ecrt_slave_config_pdos(sc, EC_END, slave_el6022_syncs))
    {
        printf("Failed to configure EL6022 PDOs!\n");
        return;
    }
    else
    {
        printf("*Success to configuring EL6022 PDOs*\n");
    }

    // 注册偏移量
    for (int port = 0; port < 2; ++port) {
        uint16_t txCtrl  = port ? 0x7011 : 0x7001;
        uint16_t txIndex = port ? 0x7010 : 0x7000; // 注意这里改成了 txIndex
        uint16_t rxCtrl  = port ? 0x6011 : 0x6001;
        uint16_t rxIndex = port ? 0x6010 : 0x6000; // 注意这里改成了 rxIndex
        int offTxCtrl = ecrt_slave_config_reg_pdo_entry(sc, txCtrl, 0x01, pTModule->domain1, NULL);
        int offRxCtrl = ecrt_slave_config_reg_pdo_entry(sc, rxCtrl, 0x01, pTModule->domain1, NULL);
        int offTxData = ecrt_slave_config_reg_pdo_entry(sc, txIndex, 0x11, pTModule->domain1, NULL); // 这里用 txIndex
        int offRxData = ecrt_slave_config_reg_pdo_entry(sc, rxIndex, 0x11, pTModule->domain1, NULL); // 这里用 rxIndex

        // 只存 4 个偏移，去掉没用的 Length
        pTModule->EL6022[EK1100Index][port][0] = offTxCtrl;
        pTModule->EL6022[EK1100Index][port][1] = offTxData;
        pTModule->EL6022[EK1100Index][port][2] = offRxCtrl;
        pTModule->EL6022[EK1100Index][port][3] = offRxData;

        printf("EL6022 arm=%d port=%d offsets (TxCtrl=%d, TxData=%d, RxCtrl=%d, RxData=%d)\n",
               EK1100Index, port, offTxCtrl, offTxData, offRxCtrl, offRxData);
    }
    
}

static int confSlavesPdos(ARMS_MASTER_THREAD_INFO *pTModule)
{
    printf(">>> Enter confSlavesPdos, armsSize=%d\n", pTModule->armsSize);

    if(pTModule->armsSize < 1 || pTModule->armsSize > MASTER_ARMS_MAX_SIZE)
    {
        printf("confSlavesPdos:机械臂不存在\n\r");
        return -1;
    }

    //遍历从站，找到所有伺服电机，并通过sdo配置所使用的pdo
    int motecIndex=0;
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        motecIndex = 0; // 每个臂开始时清零
        EK1100_slaves_inf *inf_t = NULL;
        
        list_for_each_entry(inf_t, &pTModule->slaves_inf[i].list, list)
        {
            //创建从站配置，使用绝对position（alias=0）
            ec_slave_config_t *ec_sla_con = ecrt_master_slave_config(
                                                pTModule->master, 
                                                0,
                                                inf_t->slave_in.position, 
                                                inf_t->slave_in.vendor_id,
                                                inf_t->slave_in.product_code);

            if(ec_sla_con == NULL)
            {
                printf("ecrt_master_slave_config ********* NULL\n");
                return -1;
            }


            inf_t->slave_conf = ec_sla_con;
            //如果是EK1100节点的话就赋值后continue
            if(strstr(inf_t->slave_in.name,"EK1100") != NULL)
            {
                motecIndex=0;
            }

            if(strstr(inf_t->slave_in.name,"EL2809") != NULL)
            {
                printf("EL2809 EL2809\n");
                confEL2809Pdos(pTModule, &inf_t->slave_in, ec_sla_con, i);
            }

            if(strstr(inf_t->slave_in.name,"EL6022") != NULL)
            {
                printf("EL6022 EL6022\n");
                confEL6022Pdos(pTModule, &inf_t->slave_in, ec_sla_con, i);
            }

            //机械臂的somanet结点，
            if(strstr(inf_t->slave_in.name,"KDE") != NULL)
            {
                if(pTModule->somanetn_mode == MODE_CSV)
                {
                    if (motecIndex >= ARM_MOTOR_COUNT ||
                        confSomanetCSVPdos(pTModule->domain1, &inf_t->slave_in, ec_sla_con,
                                          &pTModule->somanet_pdo[i][motecIndex]) < 0)
                        return -1;
                }
                motecIndex++;
            }

            //如果是EK1110节点的话就赋值后continue
            if(strstr(inf_t->slave_in.name,"EK1110") != NULL)
            {
                printf("EK1110 EK1110\n");
            }
            
        }

        pTModule->arm_motor_count[i] = motecIndex; 
        printf(">>> 臂%d 实际配置了 %d 个电机\n", i, pTModule->arm_motor_count[i]);

    }
    
    printf("###########################\n");
    return 0;
}

static int activeMater(ARMS_MASTER_THREAD_INFO *pTModule)
{
    int iRet = -1;
    if(pTModule->slaves_size < 1)
      return iRet;
    
    //设置DC时钟
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        // 查找对应slave_conf
        EK1100_slaves_inf *inf_t = NULL;
        list_for_each_entry(inf_t, &pTModule->slaves_inf[i].list, list)
        {
            //电机使用dc
            if(strstr(inf_t->slave_in.name,"KDE") != NULL)
            {    
                //ecrt_slave_config_dc(inf_t->slave_conf, 0x0700, 1000000, 500000, 0, 0); 
                // 尝试将 shift time 改为 0
                ecrt_slave_config_dc(inf_t->slave_conf, 0x0300, 1000000, 500000, 0, 0); 
                printf("%s DC 配置成功！\n",inf_t->slave_in.name);
            }
        }
    }

    int ret = ecrt_master_select_reference_clock(pTModule->master, NULL);
    if (ret < 0) {
        printf("Failed to select reference clock\n");
        return iRet;
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

    ssize_t recvSize = recvfrom(pTModule->mSoket, mLocalData, Msg_T_S, 0,
                                (struct sockaddr *)&mPeerAddr, &mun);
    if (recvSize >= (ssize_t)sizeof(MsgHeader))
    {
      MsgHeader *mMsg = (MsgHeader *)mLocalData;
      SUPR_R_MSG *upperMsg = NULL;
      size_t upperAvailable = 0;
      if (recvSize >= (ssize_t)(offsetof(MsgLongMsg, mData) + offsetof(SUPR_R_MSG, mData))) {
          MsgLongMsg *mLMsg = (MsgLongMsg *)mLocalData;
          upperMsg = (SUPR_R_MSG *)mLMsg->mData;
          upperAvailable = (size_t)recvSize - offsetof(MsgLongMsg, mData);
          if (upperMsg->mDataLength > upperAvailable - offsetof(SUPR_R_MSG, mData))
              upperMsg = NULL;
      }
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
        case CMD_EL2809_OUTPUT:
        {
            SUPR_R_MSG *commandMsg = upperMsg;
            if (commandMsg && commandMsg->mDataLength == sizeof(EL2809_OUTPUT_CMD))
            {
                EL2809_OUTPUT_CMD *command = (EL2809_OUTPUT_CMD *)commandMsg->mData;
                if (command->arm_index < pTModule->armsSize && command->channel < 16)
                {
                    pTModule->EL2809_output[command->arm_index][command->channel] = command->enabled != 0;
                    //printf("收到IO命令: Arm=%d, Ch=%d, En=%d\n", command->arm_index, command->channel, command->enabled);
                }
            }
            break;
        }
        case CMD_IGH_CLEAR_ERROR:
        {
            if (upperMsg)
                clearErrorSomanets(pTModule, upperMsg);
            break;
        }
        case CMD_HAND_VEL_MM:
        {
            SUPR_R_MSG *commandMsg = upperMsg;
            if (commandMsg && commandMsg->mDataLength == sizeof(float) * ARM_MOTOR_COUNT * MASTER_ARMS_MAX_SIZE) {
                upperAngularVelSomanets(pTModule, commandMsg);
                printf("Received velocity command: arm flags=%u,%u length=%u\n",
                       commandMsg->mModuleFlag[0], commandMsg->mModuleFlag[1], commandMsg->mDataLength);
            } else {
                fprintf(stderr, "Invalid velocity command length=%d\n",
                        commandMsg ? (int)commandMsg->mDataLength : -1);
            }
            break;
        }
        case CMD_HAND_RELA_POS_MM:
        {
            if (upperMsg)
                upperRelaPosSomanets(pTModule, upperMsg);
            break;
        }
        case CMD_HAND_ABS_POS_M:
        {
            if (upperMsg)
                upperAbsPosSomanets(pTModule, upperMsg);
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
    for (int arm = 0; arm < pTModule->armsSize; ++arm) {
        if (!pTModule->masterActivated || pTModule->arm_fsa[arm] != S_OP_OK) {
            resetVelocityRamp(pTModule, arm);
            continue;
        }
        if (!pTModule->upper_target_v_Flag[arm])
            continue;
        for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor) {
            const float target = pTModule->upper_target_velocity[arm][motor];
            float current = pTModule->upper_last_v_cmd[arm][motor];
            const float step = pTModule->upper_acc[arm][motor] * 0.005f;
            if (step > 0.0f && current + step >= target)
                current = target;
            else if (step < 0.0f && current + step <= target)
                current = target;
            else
                current += step;
            pTModule->upper_last_v_cmd[arm][motor] = current;
            pTModule->motors[arm][motor].target_velocity = (int32_t)(current * CSV_COUNTS_PER_DEG);
        }
        bool reached = true;
        for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor) {
            if (pTModule->upper_last_v_cmd[arm][motor] != pTModule->upper_target_velocity[arm][motor]) {
                reached = false;
                break;
            }
        }
        if (reached)
            pTModule->upper_target_v_Flag[arm] = 0;
    }

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

static void cyc_pos_upper_cmd(ARMS_MASTER_THREAD_INFO *pTModule)
{
    const float dt = 0.005f;
    const float max_speed_deg_s = 2.0f;
    const float max_acc_deg_s2 = 30.0f;
    const float counts_per_deg = (float)CSV_COUNTS_PER_DEG;

    for (int arm = 0; arm < pTModule->armsSize; ++arm) {
        if (!pTModule->masterActivated || pTModule->arm_fsa[arm] != S_OP_OK) {
            pTModule->upper_target_p_Flag[arm] = 0;
            for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor)
                pTModule->motors[arm][motor].target_velocity = 0;
            continue;
        }
        if (!pTModule->upper_target_p_Flag[arm])
            continue;

        bool arrived = true;
        for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor) {
            float actual_deg = (float)pTModule->motors[arm][motor].Position_value / counts_per_deg;
            float error_deg = pTModule->upper_target_pos[arm][motor] - actual_deg;
            float current_deg_s = pTModule->target_p_last_cmdvel[arm][motor];
            float desired_deg_s = fmaxf(-max_speed_deg_s,
                                        fminf(max_speed_deg_s, error_deg * 2.0f));
            float step = max_acc_deg_s2 * dt;
            current_deg_s += fmaxf(-step, fminf(step, desired_deg_s - current_deg_s));

            if (fabsf(error_deg) <= 0.02f && fabsf(current_deg_s) <= step) {
                current_deg_s = 0.0f;
            } else {
                arrived = false;
            }
            pTModule->target_p_last_cmdvel[arm][motor] = current_deg_s;
            pTModule->motors[arm][motor].target_velocity =
                (int32_t)lrintf(current_deg_s * counts_per_deg);
        }
        if (arrived) {
            pTModule->upper_target_p_Flag[arm] = 0;
            printf("Position target reached: arm=%d\n", arm);
        }
    }
}

#if 0 /* obsolete duplicated position/FSA block */
/*****************************************ethercat总线相关**************************************************/
//获取当前系统时间
static uint64_t system_time_ns(void)
{
    struct timespec  rt_time;
    clock_gettime(CLOCK_MONOTONIC, &rt_time);
    uint64_t time = TIMESPEC2NS(rt_time);
    return time;
}

static int32_t armsFsa(ARMS_MASTER_THREAD_INFO *pTModule)
{
    int32_t iRet = 0;
  
    //有限状态机切换
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        // 如果该臂没有电机，跳过
        if(pTModule->arm_motor_count[i] == 0) continue;
        //4个伺服电机、EK1100 Ek1110 ...
        // if(pTModule->slaves_inf[i].childSlaveSize>4)
        // {
            //写入4个电机的数据
            switch (pTModule->arm_fsa[i])
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

#endif

/*****************************************ethercat总线相关**************************************************/
//获取当前系统时间
static uint64_t system_time_ns(void)
{
    struct timespec  rt_time;
    clock_gettime(CLOCK_MONOTONIC, &rt_time);
    uint64_t time = TIMESPEC2NS(rt_time);
    return time;
}

static int32_t armsFsa(ARMS_MASTER_THREAD_INFO *pTModule)
{
    int32_t iRet = 0;
  
    //有限状态机切换
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        // 如果该臂没有电机，跳过
        if(pTModule->arm_motor_count[i] == 0) continue;
        //4个伺服电机、EK1100 Ek1110 ...
        // if(pTModule->slaves_inf[i].childSlaveSize>4)
        // {
            //写入4个电机的数据
            switch (pTModule->arm_fsa[i])
            {
                case S_SHUTDOWN:
                {
                    //"开启"禁用状态，此时需要发送控制字
                    for (int j = 0; j < pTModule->arm_motor_count[i]; j++)
                    {
                        pTModule->motors[i][j].ctrl_word = 0x0006;
                        //pTModule->motors[i][j].operation_mode = pTModule->somanetn_mode;
                        pTModule->motors[i][j].target_velocity = 0;
                    }
                    break;
                }
                case S_SWITCH_ON:
                {
                    for (int j = 0; j < pTModule->arm_motor_count[i]; j++)
                    {
                        pTModule->motors[i][j].ctrl_word = 0x0007;
                        //pTModule->motors[i][j].operation_mode = pTModule->somanetn_mode;
                        pTModule->motors[i][j].target_velocity = 0;   
                    }                   
                    break;
                }
                case S_CLEAR_ERROR:
                {
                    bool cleared = true;
                    for (int j = 0; j < pTModule->arm_motor_count[i]; j++) {
                        if (pTModule->motors[i][j].status_word & 0x0008) {
                            pTModule->motors[i][j].ctrl_word = 0x0080;
                            cleared = false;
                        } else {
                            pTModule->motors[i][j].ctrl_word = 0x0006;
                        }
                        pTModule->motors[i][j].target_velocity = 0;
                    }
                    if (cleared)
                        pTModule->arm_fsa[i] = S_SHUTDOWN;
                    break;
                }
                case S_ENABLE_OP:
                {
                    //判断状态字是否op成功
                    bool all_enabled = true;
                    for (int j = 0; j < pTModule->arm_motor_count[i]; j++)
                    {
                        if ((pTModule->motors[i][j].status_word & 0x67) != 0x27)
                        {
                            all_enabled = false;
                            break;
                        }
                    }
                    if (all_enabled)
                    {
                        pTModule->arm_fsa[i] = S_OP_OK;
                    }
                    else
                    {
                        for (int j = 0; j < pTModule->arm_motor_count[i]; j++)
                        {
                            pTModule->motors[i][j].ctrl_word = 0x000F;
                            //pTModule->motors[i][j].operation_mode = pTModule->somanetn_mode;
                        }
                    }
                    break;
                }
                case S_HAIT:
                {
                    
                    for (int j = 0; j < pTModule->arm_motor_count[i]; j++)
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
            // }

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
            sendTelemetry(pTModule);
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
static uint16_t modbusCrc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
}

static void pollEL6022(ARMS_MASTER_THREAD_INFO *pTModule)
{
    static uint32_t cycle = 0;
    cycle++;

    for (int arm = 0; arm < pTModule->armsSize; ++arm) {
        for (int port = 0; port < 2; ++port) {
            int offTxCtrl = pTModule->EL6022[arm][port][0];
            int offTxData = pTModule->EL6022[arm][port][1];
            int offRxCtrl = pTModule->EL6022[arm][port][2];
            int offRxData = pTModule->EL6022[arm][port][3];

            if (offTxCtrl < 0 || offRxCtrl < 0 || offTxData < 0 || offRxData < 0) continue;

            uint16_t ctrl = EC_READ_U16(pTModule->domain1_pd + offTxCtrl);
            uint16_t status = EC_READ_U16(pTModule->domain1_pd + offRxCtrl);
            uint8_t sensor_count = (port == 0) ? 3 : 4;

            switch (pTModule->el6022_state[arm][port]) {
                case EL6022_ST_INIT: {
                    ctrl |= 0x0004;
                    EC_WRITE_U16(pTModule->domain1_pd + offTxCtrl, ctrl);
                    if (status & 0x0004) {
                        ctrl &= ~0x0004;
                        EC_WRITE_U16(pTModule->domain1_pd + offTxCtrl, ctrl);
                        pTModule->el6022_state[arm][port] = EL6022_ST_IDLE;
                        printf(">>> EL6022 arm=%d port=%d Init OK, UART Ready!\n", arm, port);
                    }
                    break;
                }

                case EL6022_ST_IDLE: {
                    if (cycle >= pTModule->el6022_next_poll_ms[arm][port]) {
                        uint8_t idx = pTModule->el6022_sensor_index[arm][port];
                        uint16_t addr = pTModule->el6022_sensor_cfg[arm][port][idx][0];
                        uint16_t reg  = pTModule->el6022_sensor_cfg[arm][port][idx][1];
                        uint16_t cnt  = pTModule->el6022_sensor_cfg[arm][port][idx][2];

                        uint8_t *tx = pTModule->el6022_tx[arm][port];
                        tx[0] = addr; tx[1] = 0x03;
                        tx[2] = (reg >> 8) & 0xFF; tx[3] = reg & 0xFF;
                        tx[4] = (cnt >> 8) & 0xFF; tx[5] = cnt & 0xFF;
                        uint16_t crc = modbusCrc16(tx, 6);
                        tx[6] = (uint8_t)crc; tx[7] = (uint8_t)(crc >> 8);

                        for(int i = 0; i < 8; i++) EC_WRITE_U8(pTModule->domain1_pd + offTxData + i, tx[i]);
                        
                        ctrl &= 0x00FF; 
                        ctrl |= (8 << 8);
                        ctrl ^= 0x0001; // 翻转 Transmit Request
                        EC_WRITE_U16(pTModule->domain1_pd + offTxCtrl, ctrl);

                        // 记录发送前瞬间的状态字，用于接收时比对翻转
                        pTModule->el6022_last_status[arm][port] = status; 

                        pTModule->el6022_state[arm][port] = EL6022_ST_WAIT_TX_ACK;
                        pTModule->el6022_timeout_ms[arm][port] = cycle + 30;
                    }
                    break;
                }

                case EL6022_ST_WAIT_TX_ACK: {
                    // 判断 Transmit Accepted (Bit 0) 是否发生了翻转
                    uint16_t txRequest = ctrl & 0x0001;
                    if (((status & 0x0001) != 0) == (txRequest != 0)) {
                        pTModule->el6022_last_status[arm][port] = status;
                        pTModule->el6022_state[arm][port] = EL6022_ST_WAIT_RX;
                        pTModule->el6022_timeout_ms[arm][port] = cycle + 50;
                    } else if (cycle > pTModule->el6022_timeout_ms[arm][port]) {
                        printf("EL6022 arm=%d port=%d TX Timeout! \n", arm, port);
                        pTModule->el6022_sensor_index[arm][port] = (pTModule->el6022_sensor_index[arm][port] + 1) % sensor_count;
                        pTModule->el6022_next_poll_ms[arm][port] = cycle + 100;
                        pTModule->el6022_state[arm][port] = EL6022_ST_IDLE;
                    }
                    break;
                }

                case EL6022_ST_WAIT_RX: {
                    // 判断 Receive Request (Bit 1) 是否发生了翻转
                    if ((status & 0x0002) != (pTModule->el6022_last_status[arm][port] & 0x0002)) {
                        uint8_t in_len = (status >> 8) & 0xFF;
                        uint8_t idx = pTModule->el6022_sensor_index[arm][port];
                        
                        if (in_len >= 5) {
                            uint8_t rx[22];
                            for(int i = 0; i < in_len && i < 22; i++) 
                                rx[i] = EC_READ_U8(pTModule->domain1_pd + offRxData + i);
                            
                            if (rx[0] == pTModule->el6022_sensor_cfg[arm][port][idx][0] && rx[1] == 0x03) {
                                uint16_t crc = modbusCrc16(rx, in_len - 2);
                                if (rx[in_len - 2] == (uint8_t)crc && rx[in_len - 1] == (uint8_t)(crc >> 8)) {
                                    // 【修改】小端浮点数解析
                                    float angle_val = 0.0f;
                                    uint8_t f_bytes[4] = { rx[3], rx[4], rx[5], rx[6] };
                                    memcpy(&angle_val, f_bytes, 4);
                                    
                                    // 乘 1000 存为 int32，便于你原来的系统兼容
                                    int32_t val = (int32_t)(angle_val * 1000); 
                                    
                                    int sensor_idx = (port == 0) ? idx : (3 + idx);
                                    pTModule->sensor_value[arm][sensor_idx] = val;
                                    pTModule->sensor_valid[arm][sensor_idx] = 1;
                                    
                                    //printf("SUCCESS: arm=%d port=%d Sensor Addr %d = %.3f deg\n", arm, port, rx[0], angle_val);
                                }
                            }
                        }

                        // 翻转 Receive Accepted (Bit 1)
                        ctrl ^= 0x0002;
                        EC_WRITE_U16(pTModule->domain1_pd + offTxCtrl, ctrl);

                        uint16_t period = pTModule->el6022_sensor_cfg[arm][port][idx][3];
                        pTModule->el6022_sensor_index[arm][port] = (idx + 1) % sensor_count;
                        pTModule->el6022_next_poll_ms[arm][port] = cycle + period;
                        pTModule->el6022_state[arm][port] = EL6022_ST_IDLE;
                    } else if (cycle > pTModule->el6022_timeout_ms[arm][port]) {
                        // 限制打印频率，避免刷屏（每个端口独立计数）
                        static uint32_t last_print[MASTER_ARMS_MAX_SIZE][2] = {0};
                        if (cycle - last_print[arm][port] > 500) {
                            //printf("EL6022 arm=%d port=%d No response from Sensor Addr %d\n", arm, port, pTModule->el6022_sensor_cfg[arm][port][pTModule->el6022_sensor_index[arm][port]][0]);
                            last_print[arm][port] = cycle; 
                        }
                        ctrl ^= 0x0002; // 强制翻转清空
                        EC_WRITE_U16(pTModule->domain1_pd + offTxCtrl, ctrl);
                        
                        uint8_t idx = pTModule->el6022_sensor_index[arm][port];
                        pTModule->el6022_sensor_index[arm][port] = (idx + 1) % sensor_count;
                        pTModule->el6022_next_poll_ms[arm][port] = cycle + 100;
                        pTModule->el6022_state[arm][port] = EL6022_ST_IDLE;
                    }
                    break;
                }
            }
        }
    }
}
 
 static void sendTelemetry(ARMS_MASTER_THREAD_INFO *pTModule)
 {
    if (!pTModule->masterActivated || pTModule->masterId != 0)
        return;
    MASTER_TELEMETRY telemetry;
    memset(&telemetry, 0, sizeof(telemetry));
    telemetry.magic = MASTER_TELEMETRY_MAGIC;
    telemetry.version = MASTER_TELEMETRY_VERSION;
    telemetry.size = sizeof(telemetry);
    telemetry.arm_count = pTModule->armsSize;
    telemetry.master_al_state = pTModule->al_states;
    telemetry.domain_wc = pTModule->domain_wc;
    for (int arm = 0; arm < pTModule->armsSize && arm < MASTER_TELEMETRY_MAX_ARMS; ++arm) {
        telemetry.motor_count[arm] = pTModule->arm_motor_count[arm];
        for (int motor = 0; motor < pTModule->arm_motor_count[arm] && motor < MASTER_TELEMETRY_MAX_MOTORS; ++motor) {
            telemetry.motor[arm][motor].status_word = pTModule->motors[arm][motor].status_word;
            telemetry.motor[arm][motor].position = pTModule->motors[arm][motor].Position_value;
            telemetry.motor[arm][motor].velocity = pTModule->motors[arm][motor].velocity_value;
            telemetry.motor[arm][motor].torque = pTModule->motors[arm][motor].torque_act_value;
            telemetry.motor[arm][motor].error_code = pTModule->motors[arm][motor].error_code;
        }
        for (int ch = 0; ch < 16; ++ch) {
            if (pTModule->EL2809[arm][ch] >= 0)
                telemetry.el2809[arm][ch] = pTModule->EL2809_output[arm][ch];
        }
        for (int sensor = 0; sensor < MASTER_TELEMETRY_SENSOR_COUNT; ++sensor) {
            telemetry.sensor_value[arm][sensor] = pTModule->sensor_value[arm][sensor];
            telemetry.sensor_valid[arm][sensor] = pTModule->sensor_valid[arm][sensor];
        }
        for (int port = 0; port < 2; ++port) {
            int statusOffset = pTModule->EL6022[arm][port][2];
            int dataOffset = pTModule->EL6022[arm][port][3];
            if (statusOffset >= 0 && dataOffset >= 0) {
                telemetry.el6022_status[arm][port] = EC_READ_U16(pTModule->domain1_pd + statusOffset);
                for (int byte = 0; byte < MASTER_TELEMETRY_SERIAL_BYTES; ++byte)
                    telemetry.el6022_data[arm][port][byte] = EC_READ_U8(pTModule->domain1_pd + dataOffset + byte);
            }
        }
    }
    MsgSendLongMsg(pTModule->mSoket, get_port("LISTENER_LOCAL_PORT"), MN_ID_LEADER, MN_ID_SUPR,
                   MSG_ID_MASTER_TELEMETRY, 0, 0, (const char *)&telemetry, sizeof(telemetry));
}

static void reflash_tpdo_Ebus(ARMS_MASTER_THREAD_INFO *pTModule)
{
    // 读取交换somanet的数据
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        if (pTModule->arm_motor_count[i] == 0) continue;

        for (int j = 0; j < pTModule->arm_motor_count[i]; j++)
        {
                
            // CSV、CSP模式下都需要刷新
            pTModule->motors[i][j].status_word    = EC_READ_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].status_word);
            pTModule->motors[i][j].Position_value = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].Position_value);
            pTModule->motors[i][j].velocity_value = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].velocity_value);
            pTModule->motors[i][j].torque_act_value = EC_READ_S16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].torque_act_value);
            
            // CVS模式下，算法要用到力矩偏置
            pTModule->motors[i][j].following_error = EC_READ_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].following_error);
            pTModule->motors[i][j].error_code = EC_READ_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].error_code);
        }
    }
}

static void reflash_rpdo_Ebus(ARMS_MASTER_THREAD_INFO *pTModule)
{
    // 写入交换somanet的数据，1ms刷新写入一次
    for (int i = 0; i < pTModule->armsSize; i++)
    {
        if (pTModule->arm_motor_count[i] == 0) continue;

        for (int j = 0; j < pTModule->arm_motor_count[i]; j++)
        {
            EC_WRITE_U16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].ctrl_word, pTModule->motors[i][j].ctrl_word);
            EC_WRITE_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].target_position, pTModule->motors[i][j].target_position);
            EC_WRITE_S32(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].target_velocity, pTModule->motors[i][j].target_velocity);
            EC_WRITE_S16(pTModule->domain1_pd + pTModule->somanet_pdo[i][j].target_torque, pTModule->motors[i][j].target_torque);
        }
    }
    pollEL6022(pTModule);
     // ================== EL2809 控制逻辑 ==================
    for (int arm = 0; arm < pTModule->armsSize; ++arm) {
        for (int ch = 0; ch < 16; ++ch) {
            int byte_offset = pTModule->EL2809[arm][ch];
            unsigned int bit = pTModule->EL2809_bit[arm][ch];
            if (byte_offset >= 0 && bit < 8)
                EC_WRITE_BIT(pTModule->domain1_pd + byte_offset, bit,
                             pTModule->EL2809_output[arm][ch]);
        }
    }

    static unsigned int diagnostic_cycle = 0;
    static int32_t previous_position[MASTER_ARMS_MAX_SIZE][ARM_SLAVES_PER_ARM];
    static struct timespec previous_position_time[MASTER_ARMS_MAX_SIZE][ARM_SLAVES_PER_ARM];
    static bool previous_position_valid[MASTER_ARMS_MAX_SIZE][ARM_SLAVES_PER_ARM];
    if (++diagnostic_cycle % 1000 == 0) {
        struct timespec diagnostic_time;
        clock_gettime(CLOCK_MONOTONIC, &diagnostic_time);
        for (int arm = 0; arm < pTModule->armsSize; ++arm) {
            for (int motor = 0; motor < pTModule->arm_motor_count[arm]; ++motor) {
                motec_pdos *pdo = &pTModule->somanet_pdo[arm][motor];
                motec_motors *state = &pTModule->motors[arm][motor];
                if (pdo->mode_display_request) {
                    pdo->mode_request_state = ecrt_sdo_request_state(pdo->mode_display_request);
                    if (pdo->mode_request_state == EC_REQUEST_SUCCESS &&
                        ecrt_sdo_request_data_size(pdo->mode_display_request) == 1)
                        pdo->mode_display = EC_READ_S8(ecrt_sdo_request_data(pdo->mode_display_request));
                    else if (pdo->mode_request_state == EC_REQUEST_ERROR)
                        pdo->mode_display = -1;
                    if (pdo->mode_request_state != EC_REQUEST_BUSY)
                        ecrt_sdo_request_read(pdo->mode_display_request);
                }
                int64_t position_delta = 0;
                double position_delta_seconds = 0.0;
                bool position_delta_valid = previous_position_valid[arm][motor];
                if (position_delta_valid) {
                    position_delta = (int64_t)state->Position_value - previous_position[arm][motor];
                    position_delta_seconds = (diagnostic_time.tv_sec - previous_position_time[arm][motor].tv_sec) +
                        (diagnostic_time.tv_nsec - previous_position_time[arm][motor].tv_nsec) / 1e9;
                    position_delta_valid = position_delta_seconds > 0.0;
                }
                previous_position[arm][motor] = state->Position_value;
                previous_position_time[arm][motor] = diagnostic_time;
                previous_position_valid[arm][motor] = true;
                double position_counts_per_second = position_delta_valid ?
                    (double)position_delta / position_delta_seconds : 0.0;
                // printf("Servo master=%d arm=%d motor=%d requested_deg_s=%g ramp_deg_s=%g target_counts_s=%d written60ff_counts_s=%d cw=0x%04x sw=0x%04x error=0x%04x pos6064=%d pos_delta=%" PRId64 " counts dt=%.6f s pos_diff=%.3f counts/s (%s) vel606c_raw=%d fsa=%d mode6061=%d sdo_state=%d offsets(tv,pos,vel)=%d,%d,%d WC=%u/%d\n",
                //        pTModule->masterId, arm, motor,
                //        pTModule->upper_target_velocity[arm][motor],
                //        pTModule->upper_last_v_cmd[arm][motor], state->target_velocity,
                //        EC_READ_S32(pTModule->domain1_pd + pdo->target_velocity),
                //        state->ctrl_word, state->status_word, state->error_code,
                //        state->Position_value, position_delta, position_delta_seconds,
                //        position_counts_per_second,
                //        position_delta_valid ? "valid" : "invalid-first-sample",
                //        state->velocity_value, pTModule->arm_fsa[arm],
                //        pdo->mode_display, pdo->mode_request_state,
                //        pdo->target_velocity, pdo->Position_value, pdo->velocity_value,
                //        pTModule->domain1_state.working_counter, pTModule->domain1_state.wc_state);
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