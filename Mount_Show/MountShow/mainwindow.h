/*
 * mainwindow.h
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "armwidget.h"
#include "logwidget.h"
#include "udpmanager.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace MountShow {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // 连接控制
    void onConnectClicked();
    void onDisconnectClicked();

    // 系统操作
    void onCreateMastersClicked();
    void onScanSlavesClicked();
    void onStartOpClicked();
    void onDestroyMasterClicked();

    // 全局 FSA
    void onGlobalShutdown();
    void onGlobalSwitchOn();
    void onGlobalEnableOp();
    void onGlobalHalt();
    void onGlobalClearError();

    // 臂控制
    void onArm1VelocityChanged(const QVector<float> &vel);
    void onArm2VelocityChanged(const QVector<float> &vel);
    void onArm1RelaPosChanged(const QVector<float> &pos);
    void onArm2RelaPosChanged(const QVector<float> &pos);
    void onArm1AbsPosChanged(const QVector<float> &pos);
    void onArm2AbsPosChanged(const QVector<float> &pos);

    // 通信事件
    void onLogMessage(const QString &msg, int level);
    void onConnectedChanged(bool connected);
    void onPacketReceived(const QByteArray &data, const QString &fromAddr, quint16 fromPort);
    void onHeartbeatTimeout();

private:
    void setupUi();
    void setupConnections();

    Ui::MainWindow *ui;

    UdpManager *m_udpManager;
    ArmWidget *m_armWidget1;
    ArmWidget *m_armWidget2;
    LogWidget *m_logWidget;
};

} // namespace MountShow

#endif // MAINWINDOW_H
