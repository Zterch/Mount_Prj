/*
 * armwidget.cpp
 */

#include "armwidget.h"
#include "ui_armwidget.h"

#include <QMessageBox>

namespace MountShow {

ArmWidget::ArmWidget(int armIndex, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ArmWidget)
    , m_armIndex(armIndex)
{
    ui->setupUi(this);
    setupUi();
}

ArmWidget::~ArmWidget()
{
    delete ui;
}

void ArmWidget::setupUi()
{
    setWindowTitle(QString("机械臂 %1").arg(m_armIndex + 1));

    // 设置电机标签
    QStringList labels = { "臂电机1", "臂电机2", "臂电机3", "托板电机1", "托板电机2", "托板电机3" };
    for (int i = 0; i < 6; ++i) {
        QLabel *label = findChild<QLabel *>(QString("labelMotor%1").arg(i));
        if (label) {
            label->setText(labels[i]);
        }
    }

    connect(ui->btnSendVel, &QPushButton::clicked, this, &ArmWidget::onBtnSendVelocity);
    connect(ui->btnSendRelaPos, &QPushButton::clicked, this, &ArmWidget::onBtnSendRelaPos);
    connect(ui->btnSendAbsPos, &QPushButton::clicked, this, &ArmWidget::onBtnSendAbsPos);
    connect(ui->btnAllZero, &QPushButton::clicked, this, &ArmWidget::onBtnAllZero);
    connect(ui->btnEnableAll, &QPushButton::clicked, this, &ArmWidget::onBtnEnableAll);
    connect(ui->btnDisableAll, &QPushButton::clicked, this, &ArmWidget::onBtnDisableAll);
    connect(ui->btnEmergencyStop, &QPushButton::clicked, this, &ArmWidget::onBtnEmergencyStop);

    connect(ui->btnFsaShutdown, &QPushButton::clicked, this, &ArmWidget::fsaShutdownClicked);
    connect(ui->btnFsaSwitchOn, &QPushButton::clicked, this, &ArmWidget::fsaSwitchOnClicked);
    connect(ui->btnFsaEnableOp, &QPushButton::clicked, this, &ArmWidget::fsaEnableOpClicked);
    connect(ui->btnFsaHalt, &QPushButton::clicked, this, &ArmWidget::fsaHaltClicked);
    connect(ui->btnClearError, &QPushButton::clicked, this, &ArmWidget::clearErrorClicked);
}

QVector<bool> ArmWidget::motorEnabled() const
{
    QVector<bool> enabled(6, false);
    QCheckBox *boxes[6] = {
        ui->checkEnable0, ui->checkEnable1, ui->checkEnable2,
        ui->checkEnable3, ui->checkEnable4, ui->checkEnable5
    };
    for (int i = 0; i < 6; ++i) {
        if (boxes[i]) {
            enabled[i] = boxes[i]->isChecked();
        }
    }
    return enabled;
}

QVector<float> ArmWidget::currentVelocities() const
{
    QVector<float> vel(6, 0.0f);
    QDoubleSpinBox *boxes[6] = {
        ui->spinVel0, ui->spinVel1, ui->spinVel2,
        ui->spinVel3, ui->spinVel4, ui->spinVel5
    };
    QVector<bool> enabled = motorEnabled();
    for (int i = 0; i < 6; ++i) {
        if (boxes[i] && enabled[i]) {
            vel[i] = static_cast<float>(boxes[i]->value());
        }
    }
    return vel;
}

QVector<float> ArmWidget::currentPositions() const
{
    QVector<float> pos(6, 0.0f);
    QDoubleSpinBox *boxes[6] = {
        ui->spinPos0, ui->spinPos1, ui->spinPos2,
        ui->spinPos3, ui->spinPos4, ui->spinPos5
    };
    QVector<bool> enabled = motorEnabled();
    for (int i = 0; i < 6; ++i) {
        if (boxes[i] && enabled[i]) {
            pos[i] = static_cast<float>(boxes[i]->value());
        }
    }
    return pos;
}

void ArmWidget::updateMotorStatus(int motorIndex, float actualPos, float actualVel, uint16_t statusWord)
{
    if (motorIndex < 0 || motorIndex >= 6) {
        return;
    }
    QLabel *posLabel = findChild<QLabel *>(QString("labelActualPos%1").arg(motorIndex));
    QLabel *velLabel = findChild<QLabel *>(QString("labelActualVel%1").arg(motorIndex));
    QLabel *statusLabel = findChild<QLabel *>(QString("labelStatus%1").arg(motorIndex));

    if (posLabel) {
        posLabel->setText(QString::number(actualPos, 'f', 2));
    }
    if (velLabel) {
        velLabel->setText(QString::number(actualVel, 'f', 2));
    }
    if (statusLabel) {
        statusLabel->setText(QString("0x%1").arg(statusWord, 4, 16, QChar('0')));
    }
}

void ArmWidget::emergencyStop()
{
    onBtnEmergencyStop();
}

void ArmWidget::onBtnSendVelocity()
{
    emit velocityChanged(currentVelocities());
}

void ArmWidget::onBtnSendRelaPos()
{
    emit relativePositionChanged(currentPositions());
}

void ArmWidget::onBtnSendAbsPos()
{
    emit absolutePositionChanged(currentPositions());
}

void ArmWidget::onBtnAllZero()
{
    QDoubleSpinBox *velBoxes[6] = {
        ui->spinVel0, ui->spinVel1, ui->spinVel2,
        ui->spinVel3, ui->spinVel4, ui->spinVel5
    };
    QDoubleSpinBox *posBoxes[6] = {
        ui->spinPos0, ui->spinPos1, ui->spinPos2,
        ui->spinPos3, ui->spinPos4, ui->spinPos5
    };
    for (int i = 0; i < 6; ++i) {
        if (velBoxes[i]) velBoxes[i]->setValue(0.0);
        if (posBoxes[i]) posBoxes[i]->setValue(0.0);
    }
}

void ArmWidget::onBtnEnableAll()
{
    QCheckBox *boxes[6] = {
        ui->checkEnable0, ui->checkEnable1, ui->checkEnable2,
        ui->checkEnable3, ui->checkEnable4, ui->checkEnable5
    };
    for (int i = 0; i < 6; ++i) {
        if (boxes[i]) boxes[i]->setChecked(true);
    }
}

void ArmWidget::onBtnDisableAll()
{
    QCheckBox *boxes[6] = {
        ui->checkEnable0, ui->checkEnable1, ui->checkEnable2,
        ui->checkEnable3, ui->checkEnable4, ui->checkEnable5
    };
    for (int i = 0; i < 6; ++i) {
        if (boxes[i]) boxes[i]->setChecked(false);
    }
}

void ArmWidget::onBtnEmergencyStop()
{
    onBtnAllZero();
    QVector<float> zero(6, 0.0f);
    emit velocityChanged(zero);
}

} // namespace MountShow
