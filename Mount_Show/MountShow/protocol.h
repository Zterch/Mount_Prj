/*
 * protocol.h
 * 上位机与 supr 进程通信协议封装
 * 协议定义来源于 Mount_Sys/share/mot_share_messages.h
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>
#include <QVector>
#include <cstdint>

namespace MountProtocol {

// 消息头大小
static constexpr int MSG_HEADER_SIZE = 28;
static constexpr int MSG_LONG_DATA_SIZE = 2048;
static constexpr int MAX_MODULE_COUNT = 8;
static constexpr int32_t MSG_ID_MASTER_TELEMETRY = 50;
static constexpr uint32_t TELEMETRY_MAGIC = 0x4D544C59u;
static constexpr int TELEMETRY_MAX_MOTORS = 6;
static constexpr int TELEMETRY_SERIAL_BYTES = 22;
static constexpr int TELEMETRY_SENSOR_COUNT = 7;

// 命令ID
enum CmdId : int32_t {
    CMD_HEARTBEAT            = 101,
    CMD_LINK                 = 102,
    CMD_UNLINK               = 103,

    CMD_IGH_CREATE_MASTERS   = 105,
    CMD_IGH_SCAN_SLAVES      = 106,

    CMD_IGH_START_OP         = 110,
    CMD_HAND_VEL_MM          = 111,
    CMD_HAND_RELA_POS_MM     = 112,
    CMD_HAND_ABS_POS_M       = 113,

    CMD_IGH_FSA_SHUTDOWN     = 114,
    CMD_IGH_FSA_SWITH_ON     = 115,
    CMD_IGH_FSA_ENOP         = 116,
    CMD_IGH_FSA_HALT         = 117,
    CMD_IGH_CLEAR_ERROR      = 118,

    CMD_DESTROY_MASTER       = 119,
    CMD_MASTER_OP_KEYDOWN    = 120,
    CMD_EL2809_OUTPUT        = 121
};

struct El2809OutputCmd {
    uint8_t armIndex;
    uint8_t channel;
    uint8_t enabled;
    uint8_t reserved;
};

#pragma pack(push, 1)
struct SuprResponseMsg {
    int32_t mCmd;
    int32_t mFrameId;
    uint8_t mModuleFlag[MAX_MODULE_COUNT];
    uint16_t mDataLength;
    uint16_t mCRC;
};
#pragma pack(pop)

// 模块ID
enum ModuleId : int32_t {
    MN_ID_SUPR   = 1,
    MN_ID_LEADER = 2,
    MN_ID_MASTER_ARMS    = 3,
    MN_ID_MASTER_CHASSIS = 4
};

// 臂数量
static constexpr int ARM_COUNT = 2;
// 每臂电机数量（3臂电机 + 3托板电机）
static constexpr int MOTOR_PER_ARM = 6;

// 上位机发送消息结构（supr 接收）
#pragma pack(push, 1)
struct SuprRMsg {
    int32_t   mCmd;
    int32_t   mFrameId;
    uint8_t   mModuleFlag[MAX_MODULE_COUNT];
    uint16_t  mDataLength;
    uint16_t  mCRC;
};

struct TelemetryMotor {
    uint16_t statusWord;
    int32_t position;
    int32_t velocity;
    int16_t torque;
    uint16_t errorCode;
};

struct MasterTelemetry {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint8_t armCount;
    uint8_t motorCount[ARM_COUNT];
    uint8_t reserved;
    uint32_t masterAlState;
    uint16_t domainWc;
    uint16_t reserved2;
    TelemetryMotor motor[ARM_COUNT][TELEMETRY_MAX_MOTORS];
    uint8_t el2809[ARM_COUNT][16];
    uint16_t el6022Status[ARM_COUNT][2];
    uint8_t el6022Data[ARM_COUNT][2][TELEMETRY_SERIAL_BYTES];
    int32_t sensorValue[ARM_COUNT][TELEMETRY_SENSOR_COUNT];
    uint8_t sensorValid[ARM_COUNT][TELEMETRY_SENSOR_COUNT];
};
#pragma pack(pop)

// 协议构建工具类
class ProtocolBuilder {
public:
    ProtocolBuilder();

    // 基础连接命令
    static QByteArray buildLink(const QString &localIp, quint16 localPort);
    static QByteArray buildUnlink();
    static QByteArray buildHeartbeat();

    // 主站管理
    static QByteArray buildCreateMasters();
    static QByteArray buildScanSlaves();
    static QByteArray buildDestroyMaster();
    static QByteArray buildStartOp();

    // 状态机命令
    static QByteArray buildFsaShutdown();
    static QByteArray buildFsaSwitchOn();
    static QByteArray buildFsaEnableOp();
    static QByteArray buildFsaHalt();
    static QByteArray buildClearError();

    // 运动控制命令
    static QByteArray buildHandVelMm(const QVector<float> &arm1Vel, const QVector<float> &arm2Vel);
    static QByteArray buildHandRelaPosMm(const QVector<float> &arm1Pos, const QVector<float> &arm2Pos);
    static QByteArray buildHandAbsPosM(const QVector<float> &arm1Pos, const QVector<float> &arm2Pos);
    static QByteArray buildMasterOpKeydown(int32_t keyCode);
    static QByteArray buildEl2809Output(int armIndex, int channel, bool enabled);

private:
    static QByteArray buildBaseCmd(int32_t cmdId, const QByteArray &data = QByteArray());
    static QByteArray buildModuleFlagForAllArms();
    static QByteArray buildModuleFlagForArm(int armIndex);
    static uint16_t calcCrc(const QByteArray &data);
};

} // namespace MountProtocol

#endif // PROTOCOL_H
