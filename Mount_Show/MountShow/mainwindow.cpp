/*
 * mainwindow.cpp
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <cstring>

namespace MountShow {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_udpManager(new UdpManager(this))
    , m_armWidget1(new ArmWidget(0, this))
    , m_armWidget2(new ArmWidget(1, this))
    , m_logWidget(new LogWidget(this))
{
    ui->setupUi(this);
    setupUi();
    setupConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUi()
{
    setWindowTitle("Mount 机械臂上位机控制系统");
    resize(1400, 900);
    setMinimumSize(1200, 700);

    // 使用滚动区域包裹内容，防止下方被截断且支持缩小窗口
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);

    // 新建内容容器，避免与 .ui 中已有布局冲突
    QWidget *contentWidget = new QWidget(scrollArea);
    QHBoxLayout *mainLayout = new QHBoxLayout(contentWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(10);

    // 左侧系统控制面板
    ui->controlPanel->setParent(contentWidget);
    ui->controlPanel->setMinimumWidth(200);
    ui->controlPanel->setMaximumWidth(260);
    mainLayout->addWidget(ui->controlPanel, 0);

    // 中间区域：两套臂上下堆叠显示
    QVBoxLayout *armLayout = new QVBoxLayout();
    armLayout->setSpacing(10);
    armLayout->setContentsMargins(0, 0, 0, 0);

    // 臂1
    QGroupBox *armGroup1 = new QGroupBox(QString("机械臂 %1").arg(1), contentWidget);
    QVBoxLayout *layout1 = new QVBoxLayout(armGroup1);
    layout1->setContentsMargins(5, 5, 5, 5);
    layout1->addWidget(m_armWidget1);
    armLayout->addWidget(armGroup1, 1);

    // 臂2
    QGroupBox *armGroup2 = new QGroupBox(QString("机械臂 %1").arg(2), contentWidget);
    QVBoxLayout *layout2 = new QVBoxLayout(armGroup2);
    layout2->setContentsMargins(5, 5, 5, 5);
    layout2->addWidget(m_armWidget2);
    armLayout->addWidget(armGroup2, 1);

    mainLayout->addLayout(armLayout, 1);

    // 右侧日志
    m_logWidget->setParent(contentWidget);
    m_logWidget->setMinimumWidth(280);
    m_logWidget->setMaximumWidth(400);
    mainLayout->addWidget(m_logWidget, 0);

    scrollArea->setWidget(contentWidget);
    setCentralWidget(scrollArea);

    // 默认设置：supr 监听的端口为 LISTENER_TO_UPPER_PORT 33333
    ui->lineEditSuprIp->setText("127.0.0.1");
    ui->spinBoxSuprPort->setValue(33333);

    // 状态栏
    statusBar()->showMessage("就绪");
}

void MainWindow::setupConnections()
{
    // 连接控制
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);

    // 系统操作
    connect(ui->btnCreateMasters, &QPushButton::clicked, this, &MainWindow::onCreateMastersClicked);
    connect(ui->btnScanSlaves, &QPushButton::clicked, this, &MainWindow::onScanSlavesClicked);
    connect(ui->btnGlobalEnableOp, &QPushButton::clicked, this, &MainWindow::onStartOpClicked);
    connect(ui->btnDestroyMaster, &QPushButton::clicked, this, &MainWindow::onDestroyMasterClicked);

    // 全局 FSA
    connect(ui->btnGlobalShutdown, &QPushButton::clicked, this, &MainWindow::onGlobalShutdown);
    connect(ui->btnGlobalSwitchOn, &QPushButton::clicked, this, &MainWindow::onGlobalSwitchOn);
    connect(ui->btnStartOp, &QPushButton::clicked, this, &MainWindow::onGlobalEnableOp);
    connect(ui->btnGlobalHalt, &QPushButton::clicked, this, &MainWindow::onGlobalHalt);
    connect(ui->btnGlobalClearError, &QPushButton::clicked, this, &MainWindow::onGlobalClearError);

    // 臂1控制
    connect(m_armWidget1, &ArmWidget::velocityChanged, this, &MainWindow::onArm1VelocityChanged);
    connect(m_armWidget1, &ArmWidget::relativePositionChanged, this, &MainWindow::onArm1RelaPosChanged);
    connect(m_armWidget1, &ArmWidget::absolutePositionChanged, this, &MainWindow::onArm1AbsPosChanged);
    connect(m_armWidget1, &ArmWidget::digitalOutputChanged, m_udpManager, &UdpManager::sendDigitalOutput);
    connect(m_armWidget1, &ArmWidget::fsaShutdownClicked, this, &MainWindow::onGlobalShutdown);
    connect(m_armWidget1, &ArmWidget::fsaSwitchOnClicked, this, &MainWindow::onGlobalSwitchOn);
    connect(m_armWidget1, &ArmWidget::fsaEnableOpClicked, this, &MainWindow::onGlobalEnableOp);
    connect(m_armWidget1, &ArmWidget::fsaHaltClicked, this, &MainWindow::onGlobalHalt);
    connect(m_armWidget1, &ArmWidget::clearErrorClicked, this, &MainWindow::onGlobalClearError);

    // 臂2控制
    connect(m_armWidget2, &ArmWidget::velocityChanged, this, &MainWindow::onArm2VelocityChanged);
    connect(m_armWidget2, &ArmWidget::relativePositionChanged, this, &MainWindow::onArm2RelaPosChanged);
    connect(m_armWidget2, &ArmWidget::absolutePositionChanged, this, &MainWindow::onArm2AbsPosChanged);
    connect(m_armWidget2, &ArmWidget::digitalOutputChanged, m_udpManager, &UdpManager::sendDigitalOutput);
    connect(m_armWidget2, &ArmWidget::fsaShutdownClicked, this, &MainWindow::onGlobalShutdown);
    connect(m_armWidget2, &ArmWidget::fsaSwitchOnClicked, this, &MainWindow::onGlobalSwitchOn);
    connect(m_armWidget2, &ArmWidget::fsaEnableOpClicked, this, &MainWindow::onGlobalEnableOp);
    connect(m_armWidget2, &ArmWidget::fsaHaltClicked, this, &MainWindow::onGlobalHalt);
    connect(m_armWidget2, &ArmWidget::clearErrorClicked, this, &MainWindow::onGlobalClearError);

    // UDP 事件
    connect(m_udpManager, &UdpManager::logMessage, this, &MainWindow::onLogMessage);
    connect(m_udpManager, &UdpManager::connectedChanged, this, &MainWindow::onConnectedChanged);
    connect(m_udpManager, &UdpManager::packetReceived, this, &MainWindow::onPacketReceived);
    connect(m_udpManager, &UdpManager::heartbeatTimeout, this, &MainWindow::onHeartbeatTimeout);
}

void MainWindow::onConnectClicked()
{
    QString ip = ui->lineEditSuprIp->text();
    quint16 port = static_cast<quint16>(ui->spinBoxSuprPort->value());

    if (m_udpManager->connectToSupr(ip, port)) {
        ui->btnConnect->setEnabled(false);
        ui->btnDisconnect->setEnabled(true);
    } else {
        QMessageBox::critical(this, "连接失败", "无法连接 supr 进程，请检查 IP 和端口。");
    }
}

void MainWindow::onDisconnectClicked()
{
    m_udpManager->disconnectFromSupr();
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
}

void MainWindow::onCreateMastersClicked()
{
    if (!m_udpManager->isConnected()) {
        QMessageBox::warning(this, "未连接", "请先连接 supr 进程。");
        return;
    }
    m_udpManager->sendCreateMasters();
    m_logWidget->appendLog("发送创建主站命令", 0);
}

void MainWindow::onScanSlavesClicked()
{
    if (!m_udpManager->isConnected()) {
        QMessageBox::warning(this, "未连接", "请先连接 supr 进程。");
        return;
    }
    m_udpManager->sendScanSlaves();
    m_logWidget->appendLog("发送扫描从站命令", 0);
}

void MainWindow::onStartOpClicked()
{
    if (!m_udpManager->isConnected()) {
        QMessageBox::warning(this, "未连接", "请先连接 supr 进程。");
        return;
    }
    m_udpManager->sendStartOp();
    m_logWidget->appendLog("发送启动操作命令", 0);
}

void MainWindow::onDestroyMasterClicked()
{
    if (!m_udpManager->isConnected()) {
        QMessageBox::warning(this, "未连接", "请先连接 supr 进程。");
        return;
    }
    m_udpManager->sendDestroyMaster();
    m_logWidget->appendLog("发送销毁主站命令", 0);
}

void MainWindow::onGlobalShutdown()
{
    if (!m_udpManager->isConnected()) return;
    m_udpManager->sendFsaShutdown();
    m_logWidget->appendLog("发送 FSA Shutdown", 0);
}

void MainWindow::onGlobalSwitchOn()
{
    if (!m_udpManager->isConnected()) return;
    m_udpManager->sendFsaSwitchOn();
    m_logWidget->appendLog("发送 FSA Switch On", 0);
}

void MainWindow::onGlobalEnableOp()
{
    if (!m_udpManager->isConnected()) return;
    m_udpManager->sendFsaEnableOp();
    m_logWidget->appendLog("发送 FSA Enable Op", 0);
}

void MainWindow::onGlobalHalt()
{
    if (!m_udpManager->isConnected()) return;
    m_udpManager->sendFsaHalt();
    m_logWidget->appendLog("发送 FSA Halt", 1);
}

void MainWindow::onGlobalClearError()
{
    if (!m_udpManager->isConnected()) return;
    m_udpManager->sendClearError();
    m_logWidget->appendLog("发送 Clear Error", 0);
}

void MainWindow::onArm1VelocityChanged(const QVector<float> &vel)
{
    QVector<float> arm2 = m_armWidget2->currentVelocities();
    m_udpManager->sendHandVelMm(vel, arm2);
    m_logWidget->appendLog(QString("臂1发送速度命令"), 0);
}

void MainWindow::onArm2VelocityChanged(const QVector<float> &vel)
{
    QVector<float> arm1 = m_armWidget1->currentVelocities();
    m_udpManager->sendHandVelMm(arm1, vel);
    m_logWidget->appendLog(QString("臂2发送速度命令"), 0);
}

void MainWindow::onArm1RelaPosChanged(const QVector<float> &pos)
{
    QVector<float> arm2 = m_armWidget2->currentPositions();
    m_udpManager->sendHandRelaPosMm(pos, arm2);
    m_logWidget->appendLog("臂1发送相对位置命令", 0);
}

void MainWindow::onArm2RelaPosChanged(const QVector<float> &pos)
{
    QVector<float> arm1 = m_armWidget1->currentPositions();
    m_udpManager->sendHandRelaPosMm(arm1, pos);
    m_logWidget->appendLog("臂2发送相对位置命令", 0);
}

void MainWindow::onArm1AbsPosChanged(const QVector<float> &pos)
{
    QVector<float> arm2 = m_armWidget2->currentPositions();
    m_udpManager->sendHandAbsPosM(pos, arm2);
    m_logWidget->appendLog("臂1发送绝对位置命令", 0);
}

void MainWindow::onArm2AbsPosChanged(const QVector<float> &pos)
{
    QVector<float> arm1 = m_armWidget1->currentPositions();
    m_udpManager->sendHandAbsPosM(arm1, pos);
    m_logWidget->appendLog("臂2发送绝对位置命令", 0);
}

void MainWindow::onLogMessage(const QString &msg, int level)
{
    m_logWidget->appendLog(msg, level);
}

void MainWindow::onConnectedChanged(bool connected)
{
    QString status = connected ? "已连接" : "已断开";
    statusBar()->showMessage(status);
    m_logWidget->appendLog(QString("连接状态: %1").arg(status), connected ? 0 : 1);

    ui->btnCreateMasters->setEnabled(connected);
    ui->btnScanSlaves->setEnabled(connected);
    ui->btnStartOp->setEnabled(connected);
    ui->btnDestroyMaster->setEnabled(connected);
    ui->btnGlobalShutdown->setEnabled(connected);
    ui->btnGlobalSwitchOn->setEnabled(connected);
    ui->btnGlobalEnableOp->setEnabled(connected);
    ui->btnGlobalHalt->setEnabled(connected);
    ui->btnGlobalClearError->setEnabled(connected);
}

void MainWindow::onPacketReceived(const QByteArray &data, const QString &fromAddr, quint16 fromPort)
{
    Q_UNUSED(fromAddr)
    Q_UNUSED(fromPort)

    if (data.size() < static_cast<int>(sizeof(MountProtocol::SuprRMsg))) {
        return;
    }

    const MountProtocol::SuprResponseMsg *msg =
        reinterpret_cast<const MountProtocol::SuprResponseMsg *>(data.constData());

    if (msg->mCmd == MountProtocol::CMD_HEARTBEAT) {
        return;
    }

    if (msg->mCmd == MountProtocol::MSG_ID_MASTER_TELEMETRY) {
        const int payloadOffset = sizeof(MountProtocol::SuprResponseMsg);
        if (msg->mDataLength != sizeof(MountProtocol::MasterTelemetry) ||
            data.size() < payloadOffset + msg->mDataLength) {
            m_logWidget->appendLog("遥测包长度无效", 2);
            return;
        }

        MountProtocol::MasterTelemetry telemetry;
        memcpy(&telemetry, data.constData() + payloadOffset, sizeof(telemetry));
        if (telemetry.magic != MountProtocol::TELEMETRY_MAGIC ||
            telemetry.version != 1 || telemetry.size != sizeof(telemetry)) {
            m_logWidget->appendLog("遥测包版本或校验标记无效", 2);
            return;
        }

        m_armWidget1->updateTelemetry(telemetry);
        m_armWidget2->updateTelemetry(telemetry);
        statusBar()->showMessage(QString("主站 AL=0x%1 | Domain WC=%2 | 臂电机数 %3 / %4")
            .arg(telemetry.masterAlState, 2, 16, QChar('0'))
            .arg(telemetry.domainWc)
            .arg(telemetry.motorCount[0])
            .arg(telemetry.motorCount[1]));
        return;
    }

    m_logWidget->appendLog(QString("收到回复 cmd=%1 len=%2").arg(msg->mCmd).arg(data.size()), 0);
}

void MainWindow::onHeartbeatTimeout()
{
    // 不在日志区打印心跳超时，避免刷屏；仅在状态栏提示
    statusBar()->showMessage("心跳超时");
}

} // namespace MountShow
