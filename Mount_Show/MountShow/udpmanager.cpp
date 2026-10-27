/*
 * udpmanager.cpp
 */

#include "udpmanager.h"

#include <QHostAddress>
#include <QNetworkInterface>
#include <QRandomGenerator>

namespace MountShow {

UdpReceiverThread::UdpReceiverThread(QUdpSocket *socket, QObject *parent)
    : QThread(parent)
    , m_socket(socket)
    , m_running(true)
{
}

void UdpReceiverThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_running = false;
}

void UdpReceiverThread::run()
{
    while (true) {
        {
            QMutexLocker locker(&m_mutex);
            if (!m_running) {
                break;
            }
        }

        // 每次循环重新取 m_socket，若为空则安全等待
        QUdpSocket *socket = nullptr;
        {
            QMutexLocker locker(&m_mutex);
            socket = m_socket;
        }

        if (!socket) {
            msleep(1);
            continue;
        }

        if (socket->hasPendingDatagrams()) {
            QByteArray datagram;
            qint64 size = socket->pendingDatagramSize();
            if (size < 0) {
                continue;
            }
            datagram.resize(static_cast<int>(size));
            QHostAddress sender;
            quint16 senderPort;
            socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            emit packetReceived(datagram, sender.toString(), senderPort);
        } else {
            msleep(1);
        }
    }
}

void UdpReceiverThread::setSocket(QUdpSocket *socket)
{
    QMutexLocker locker(&m_mutex);
    m_socket = socket;
}

UdpManager::UdpManager(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_receiver(nullptr)
    , m_heartbeatTimer(new QTimer(this))
    , m_suprPort(0)
    , m_localPort(0)
    , m_connected(false)
    , m_heartbeatIntervalMs(500)
    , m_timeoutCount(0)
{
    connect(m_heartbeatTimer, &QTimer::timeout, this, &UdpManager::onHeartbeat);
}

UdpManager::~UdpManager()
{
    disconnectFromSupr();
}

bool UdpManager::connectToSupr(const QString &suprIp, quint16 suprPort, quint16 localPort)
{
    if (m_socket) {
        disconnectFromSupr();
    }

    m_socket = new QUdpSocket(this);
    m_suprIp = suprIp;
    m_suprPort = suprPort;

    if (localPort == 0) {
        m_localPort = 20000 + QRandomGenerator::global()->bounded(10000);
    } else {
        m_localPort = localPort;
    }

    if (!m_socket->bind(QHostAddress::AnyIPv4, m_localPort)) {
        emit logMessage(QString("绑定本地端口 %1 失败: %2").arg(m_localPort).arg(m_socket->errorString()), 2);
        delete m_socket;
        m_socket = nullptr;
        return false;
    }

    m_receiver = new UdpReceiverThread(m_socket, this);
    connect(m_receiver, &UdpReceiverThread::packetReceived, this, &UdpManager::onPacketReceived);
    m_receiver->start();

    m_connected = true;
    emit connectedChanged(true);
    emit logMessage(QString("已连接 supr %1:%2 (本地端口 %3)").arg(suprIp).arg(suprPort).arg(m_localPort), 0);

    // 发送 Link 命令
    sendLink();

    // 启动心跳
    m_timeoutCount = 0;
    m_heartbeatTimer->start(m_heartbeatIntervalMs);
    return true;
}

void UdpManager::disconnectFromSupr()
{
    m_heartbeatTimer->stop();
    m_connected = false;

    // 先停止接收线程，确保它不再访问 m_socket
    if (m_receiver) {
        m_receiver->stop();
        m_receiver->setSocket(nullptr);
        if (!m_receiver->wait(2000)) {
            m_receiver->terminate();
            m_receiver->wait();
        }
        delete m_receiver;
        m_receiver = nullptr;
    }

    if (m_socket) {
        sendDestroyMaster();
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }

    emit connectedChanged(false);
    emit logMessage("已断开 supr 连接", 0);
}

bool UdpManager::isConnected() const
{
    return m_connected && m_socket;
}

bool UdpManager::sendCommand(const QByteArray &packet)
{
    QMutexLocker locker(&m_sendMutex);
    if (!m_socket) {
        emit logMessage("发送失败：未连接", 2);
        return false;
    }

    qint64 ret = m_socket->writeDatagram(packet, QHostAddress(m_suprIp), m_suprPort);
    if (ret != packet.size()) {
        emit logMessage(QString("发送失败: %1").arg(m_socket->errorString()), 2);
        return false;
    }
    return true;
}

void UdpManager::setHeartbeatInterval(int ms)
{
    m_heartbeatIntervalMs = ms;
    if (m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->start(ms);
    }
}

void UdpManager::sendLink()
{
    QString localIp = "127.0.0.1";
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        if (iface.flags() & QNetworkInterface::IsUp && iface.name() != "lo") {
            QList<QHostAddress> addrs = iface.allAddresses();
            for (const QHostAddress &addr : addrs) {
                if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
                    localIp = addr.toString();
                    break;
                }
            }
        }
        if (localIp != "127.0.0.1") {
            break;
        }
    }

    sendCommand(MountProtocol::ProtocolBuilder::buildLink(localIp, m_localPort));
}

void UdpManager::sendHeartbeat()
{
    // 心跳包静默发送，不打印日志，避免刷屏
    QMutexLocker locker(&m_sendMutex);
    if (!m_socket) return;

    QByteArray packet = MountProtocol::ProtocolBuilder::buildHeartbeat();
    m_socket->writeDatagram(packet, QHostAddress(m_suprIp), m_suprPort);
}

void UdpManager::sendCreateMasters()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildCreateMasters());
}

void UdpManager::sendScanSlaves()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildScanSlaves());
}

void UdpManager::sendStartOp()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildStartOp());
}

void UdpManager::sendFsaShutdown()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildFsaShutdown());
}

void UdpManager::sendFsaSwitchOn()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildFsaSwitchOn());
}

void UdpManager::sendFsaEnableOp()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildFsaEnableOp());
}

void UdpManager::sendFsaHalt()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildFsaHalt());
}

void UdpManager::sendClearError()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildClearError());
}

void UdpManager::sendDestroyMaster()
{
    sendCommand(MountProtocol::ProtocolBuilder::buildDestroyMaster());
}

void UdpManager::sendHandVelMm(const QVector<float> &arm1Vel, const QVector<float> &arm2Vel)
{
    sendCommand(MountProtocol::ProtocolBuilder::buildHandVelMm(arm1Vel, arm2Vel));
}

void UdpManager::sendHandRelaPosMm(const QVector<float> &arm1Pos, const QVector<float> &arm2Pos)
{
    sendCommand(MountProtocol::ProtocolBuilder::buildHandRelaPosMm(arm1Pos, arm2Pos));
}

void UdpManager::sendHandAbsPosM(const QVector<float> &arm1Pos, const QVector<float> &arm2Pos)
{
    sendCommand(MountProtocol::ProtocolBuilder::buildHandAbsPosM(arm1Pos, arm2Pos));
}

void UdpManager::sendDigitalOutput(int armIndex, int channel, bool enabled)
{
    sendCommand(MountProtocol::ProtocolBuilder::buildEl2809Output(armIndex, channel, enabled));
}

void UdpManager::onHeartbeat()
{
    if (!m_connected) {
        return;
    }

    sendHeartbeat();

    if (++m_timeoutCount > MAX_TIMEOUT_COUNT) {
        emit heartbeatTimeout();
        m_timeoutCount = 0;
    }
}

void UdpManager::onPacketReceived(const QByteArray &data, const QString &fromAddr, quint16 fromPort)
{
    // 收到任何回复都重置超时计数
    m_timeoutCount = 0;
    emit packetReceived(data, fromAddr, fromPort);
}

} // namespace MountShow
