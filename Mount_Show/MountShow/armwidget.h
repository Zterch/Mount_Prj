/*
 * armwidget.h
 * 单臂控制组件
 * 每套臂包含：3个臂电机 + 3个托板电机
 */

#ifndef ARMWIDGET_H
#define ARMWIDGET_H

#include "protocol.h"

#include <QWidget>
#include <QVector>
#include <QLabel>
#include <QPushButton>

namespace Ui {
class ArmWidget;
}

namespace MountShow {

class ArmWidget : public QWidget {
    Q_OBJECT

public:
    explicit ArmWidget(int armIndex, QWidget *parent = nullptr);
    ~ArmWidget() override;

    // 获取当前设置的速度值（6个电机，未使能的电机为0）
    QVector<float> currentVelocities() const;

    // 获取当前设置的位置值（6个电机，未使能的电机为0）
    QVector<float> currentPositions() const;

    // 获取电机使能状态
    QVector<bool> motorEnabled() const;

    // 更新电机状态显示（预留接口）
    void updateMotorStatus(int motorIndex, float actualPos, float actualVel, uint16_t statusWord);
    void updateTelemetry(const MountProtocol::MasterTelemetry &telemetry);

    // 紧急停止：所有值归零并发送信号
    void emergencyStop();

signals:
    void velocityChanged(const QVector<float> &vel);
    void relativePositionChanged(const QVector<float> &pos);
    void absolutePositionChanged(const QVector<float> &pos);
    void fsaShutdownClicked();
    void fsaSwitchOnClicked();
    void fsaEnableOpClicked();
    void fsaHaltClicked();
    void clearErrorClicked();
    void digitalOutputChanged(int armIndex, int channel, bool enabled);

private slots:
    void onBtnSendVelocity();
    void onBtnSendRelaPos();
    void onBtnSendAbsPos();
    void onBtnAllZero();
    void onBtnEnableAll();
    void onBtnDisableAll();
    void onBtnEmergencyStop();

private:
    void setupUi();

    Ui::ArmWidget *ui;
    int m_armIndex;
    QVector<QPushButton *> m_outputButtons;
    QVector<QLabel *> m_sensorLabels;
};

} // namespace MountShow

#endif // ARMWIDGET_H
