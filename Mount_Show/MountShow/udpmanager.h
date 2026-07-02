/*
 * udpmanager.h
 * UDP 通信管理器
 * 负责上位机与 supr 进程的 UDP 收发、心跳维护
 */

#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include "protocol.h"

#include <QMutex>
#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QThread>

namespace MountShow {

// 工作线程：持续监听 UDP 回复
class UdpReceiverThread : public QThread {
    Q_OBJECT
public:
    explicit UdpReceiverThread(QUdpSocket *socket, QObject *parent = nullptr);

    void stop();

signals:
    void packetReceived(const QByteArray &data, const QString &fromAddr, quint16 fromPort);

public slots:
    void setSocket(QUdpSocket *socket);

protected:
    void run() override;

private:
    QUdpSocket *m_socket;
    bool m_running;
    QMutex m_mutex;
};

// UDP 通信管理器
class UdpManager : public QObject {
    Q_OBJECT
public:
    explicit UdpManager(QObject *parent = nullptr);
    ~UdpManager() override;

    // 连接目标 supr
    bool connectToSupr(const QString &suprIp, quint16 suprPort, quint16 localPort = 0);

    // 断开连接
    void disconnectFromSupr();

    // 是否已连接
    bool isConnected() const;

    // 发送命令
    bool sendCommand(const QByteArray &packet);

    // 设置心跳间隔（ms），默认 500ms
    void setHeartbeatInterval(int ms);

    // 协议构建
    void sendLink();
    void sendHeartbeat();
    void sendCreateMasters();
    void sendScanSlaves();
    void sendStartOp();
    void sendFsaShutdown();
    void sendFsaSwitchOn();
    void sendFsaEnableOp();
    void sendFsaHalt();
    void sendClearError();
    void sendDestroyMaster();
    void sendHandVelMm(const QVector<float> &arm1Vel, const QVector<float> &arm2Vel);
    void sendHandRelaPosMm(const QVector<float> &arm1Pos, const QVector<float> &arm2Pos);
    void sendHandAbsPosM(const QVector<float> &arm1Pos, const QVector<float> &arm2Pos);

signals:
    void connectedChanged(bool connected);
    void packetReceived(const QByteArray &data, const QString &fromAddr, quint16 fromPort);
    void logMessage(const QString &msg, int level);
    void heartbeatTimeout();

private slots:
    void onHeartbeat();
    void onPacketReceived(const QByteArray &data, const QString &fromAddr, quint16 fromPort);

private:
    QUdpSocket *m_socket;
    UdpReceiverThread *m_receiver;
    QTimer *m_heartbeatTimer;

    QString m_suprIp;
    quint16 m_suprPort;
    quint16 m_localPort;

    bool m_connected;
    int m_heartbeatIntervalMs;
    int m_timeoutCount;
    static constexpr int MAX_TIMEOUT_COUNT = 3;

    QMutex m_sendMutex;
};

} // namespace MountShow

#endif // UDPMANAGER_H
