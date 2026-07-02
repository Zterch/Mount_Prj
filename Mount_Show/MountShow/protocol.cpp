/*
 * protocol.cpp
 */

#include "protocol.h"

#include <QHostAddress>
#include <cstring>

namespace MountProtocol {

ProtocolBuilder::ProtocolBuilder()
{
}

QByteArray ProtocolBuilder::buildBaseCmd(int32_t cmdId, const QByteArray &data)
{
    SuprRMsg msg;
    memset(&msg, 0, sizeof(SuprRMsg));
    msg.mCmd = cmdId;
    msg.mFrameId = 0;

    // 默认使能所有臂
    for (int i = 0; i < MAX_MODULE_COUNT; ++i) {
        msg.mModuleFlag[i] = (i < ARM_COUNT) ? 1 : 0;
    }

    msg.mDataLength = static_cast<uint16_t>(data.size());

    QByteArray packet;
    packet.resize(sizeof(SuprRMsg) + data.size());
    memcpy(packet.data(), &msg, sizeof(SuprRMsg));
    if (!data.isEmpty()) {
        memcpy(packet.data() + sizeof(SuprRMsg), data.constData(), data.size());
    }

    // CRC 计算整个包
    uint16_t crc = calcCrc(packet);
    reinterpret_cast<SuprRMsg*>(packet.data())->mCRC = crc;

    return packet;
}

QByteArray ProtocolBuilder::buildModuleFlagForAllArms()
{
    QByteArray flag(8, 0);
    flag[0] = 1;
    flag[1] = 1;
    return flag;
}

QByteArray ProtocolBuilder::buildModuleFlagForArm(int armIndex)
{
    QByteArray flag(8, 0);
    if (armIndex >= 0 && armIndex < ARM_COUNT) {
        flag[armIndex] = 1;
    }
    return flag;
}

uint16_t ProtocolBuilder::calcCrc(const QByteArray &data)
{
    // 累加和校验，与下位机保持一致
    uint16_t crc = 0;
    const char *ptr = data.constData();
    int len = data.size();
    for (int i = 0; i < len; ++i) {
        crc += static_cast<uint8_t>(ptr[i]);
    }
    return crc;
}

QByteArray ProtocolBuilder::buildLink(const QString &localIp, quint16 localPort)
{
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&localPort), sizeof(quint16));
    data.append(localIp.toUtf8());
    // 补零到固定长度，确保解析正确
    while (data.size() < 18) {
        data.append('\0');
    }
    return buildBaseCmd(CMD_LINK, data);
}

QByteArray ProtocolBuilder::buildUnlink()
{
    return buildBaseCmd(CMD_UNLINK);
}

QByteArray ProtocolBuilder::buildHeartbeat()
{
    return buildBaseCmd(CMD_HEARTBEAT);
}

QByteArray ProtocolBuilder::buildCreateMasters()
{
    return buildBaseCmd(CMD_IGH_CREATE_MASTERS);
}

QByteArray ProtocolBuilder::buildScanSlaves()
{
    return buildBaseCmd(CMD_IGH_SCAN_SLAVES);
}

QByteArray ProtocolBuilder::buildDestroyMaster()
{
    return buildBaseCmd(CMD_DESTROY_MASTER);
}

QByteArray ProtocolBuilder::buildStartOp()
{
    return buildBaseCmd(CMD_IGH_START_OP);
}

QByteArray ProtocolBuilder::buildFsaShutdown()
{
    return buildBaseCmd(CMD_IGH_FSA_SHUTDOWN);
}

QByteArray ProtocolBuilder::buildFsaSwitchOn()
{
    return buildBaseCmd(CMD_IGH_FSA_SWITH_ON);
}

QByteArray ProtocolBuilder::buildFsaEnableOp()
{
    return buildBaseCmd(CMD_IGH_FSA_ENOP);
}

QByteArray ProtocolBuilder::buildFsaHalt()
{
    return buildBaseCmd(CMD_IGH_FSA_HALT);
}

QByteArray ProtocolBuilder::buildClearError()
{
    return buildBaseCmd(CMD_IGH_CLEAR_ERROR);
}

QByteArray ProtocolBuilder::buildHandVelMm(const QVector<float> &arm1Vel, const QVector<float> &arm2Vel)
{
    QByteArray data;

    auto appendVel = [&data](const QVector<float> &vel) {
        for (int i = 0; i < MOTOR_PER_ARM; ++i) {
            float v = (i < vel.size()) ? vel[i] : 0.0f;
            data.append(reinterpret_cast<const char*>(&v), sizeof(float));
        }
    };

    appendVel(arm1Vel);
    appendVel(arm2Vel);

    return buildBaseCmd(CMD_HAND_VEL_MM, data);
}

QByteArray ProtocolBuilder::buildHandRelaPosMm(const QVector<float> &arm1Pos, const QVector<float> &arm2Pos)
{
    QByteArray data;

    auto appendPos = [&data](const QVector<float> &pos) {
        for (int i = 0; i < MOTOR_PER_ARM; ++i) {
            float p = (i < pos.size()) ? pos[i] : 0.0f;
            data.append(reinterpret_cast<const char*>(&p), sizeof(float));
        }
    };

    appendPos(arm1Pos);
    appendPos(arm2Pos);

    return buildBaseCmd(CMD_HAND_RELA_POS_MM, data);
}

QByteArray ProtocolBuilder::buildHandAbsPosM(const QVector<float> &arm1Pos, const QVector<float> &arm2Pos)
{
    QByteArray data;

    auto appendPos = [&data](const QVector<float> &pos) {
        for (int i = 0; i < MOTOR_PER_ARM; ++i) {
            float p = (i < pos.size()) ? pos[i] : 0.0f;
            data.append(reinterpret_cast<const char*>(&p), sizeof(float));
        }
    };

    appendPos(arm1Pos);
    appendPos(arm2Pos);

    return buildBaseCmd(CMD_HAND_ABS_POS_M, data);
}

QByteArray ProtocolBuilder::buildMasterOpKeydown(int32_t keyCode)
{
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&keyCode), sizeof(int32_t));
    return buildBaseCmd(CMD_MASTER_OP_KEYDOWN, data);
}

} // namespace MountProtocol
