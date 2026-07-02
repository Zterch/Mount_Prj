/********************************************************************************
** Form generated from reading UI file 'armwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ARMWIDGET_H
#define UI_ARMWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ArmWidget
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxStatus;
    QGridLayout *gridLayoutStatus;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_7;
    QLabel *labelMotor0;
    QLabel *labelActualPos0;
    QLabel *labelActualVel0;
    QLabel *labelStatus0;
    QCheckBox *checkEnable0;
    QLabel *labelMotor1;
    QLabel *labelActualPos1;
    QLabel *labelActualVel1;
    QLabel *labelStatus1;
    QCheckBox *checkEnable1;
    QLabel *labelMotor2;
    QLabel *labelActualPos2;
    QLabel *labelActualVel2;
    QLabel *labelStatus2;
    QCheckBox *checkEnable2;
    QLabel *labelMotor3;
    QLabel *labelActualPos3;
    QLabel *labelActualVel3;
    QLabel *labelStatus3;
    QCheckBox *checkEnable3;
    QLabel *labelMotor4;
    QLabel *labelActualPos4;
    QLabel *labelActualVel4;
    QLabel *labelStatus4;
    QCheckBox *checkEnable4;
    QLabel *labelMotor5;
    QLabel *labelActualPos5;
    QLabel *labelActualVel5;
    QLabel *labelStatus5;
    QCheckBox *checkEnable5;
    QGroupBox *groupBoxCommand;
    QGridLayout *gridLayoutCommand;
    QLabel *label_5;
    QDoubleSpinBox *spinVel0;
    QDoubleSpinBox *spinVel1;
    QDoubleSpinBox *spinVel2;
    QDoubleSpinBox *spinVel3;
    QDoubleSpinBox *spinVel4;
    QDoubleSpinBox *spinVel5;
    QPushButton *btnSendVel;
    QLabel *label_6;
    QDoubleSpinBox *spinPos0;
    QDoubleSpinBox *spinPos1;
    QDoubleSpinBox *spinPos2;
    QDoubleSpinBox *spinPos3;
    QDoubleSpinBox *spinPos4;
    QDoubleSpinBox *spinPos5;
    QPushButton *btnSendRelaPos;
    QPushButton *btnSendAbsPos;
    QPushButton *btnAllZero;
    QPushButton *btnEnableAll;
    QPushButton *btnDisableAll;
    QPushButton *btnEmergencyStop;
    QGroupBox *groupBoxFsa;
    QHBoxLayout *horizontalLayoutFsa;
    QPushButton *btnFsaShutdown;
    QPushButton *btnFsaSwitchOn;
    QPushButton *btnFsaEnableOp;
    QPushButton *btnFsaHalt;
    QPushButton *btnClearError;

    void setupUi(QWidget *ArmWidget)
    {
        if (ArmWidget->objectName().isEmpty())
            ArmWidget->setObjectName(QString::fromUtf8("ArmWidget"));
        ArmWidget->resize(900, 600);
        verticalLayout = new QVBoxLayout(ArmWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBoxStatus = new QGroupBox(ArmWidget);
        groupBoxStatus->setObjectName(QString::fromUtf8("groupBoxStatus"));
        gridLayoutStatus = new QGridLayout(groupBoxStatus);
        gridLayoutStatus->setObjectName(QString::fromUtf8("gridLayoutStatus"));
        label = new QLabel(groupBoxStatus);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayoutStatus->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(groupBoxStatus);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayoutStatus->addWidget(label_2, 0, 1, 1, 1);

        label_3 = new QLabel(groupBoxStatus);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayoutStatus->addWidget(label_3, 0, 2, 1, 1);

        label_4 = new QLabel(groupBoxStatus);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayoutStatus->addWidget(label_4, 0, 3, 1, 1);

        label_7 = new QLabel(groupBoxStatus);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayoutStatus->addWidget(label_7, 0, 4, 1, 1);

        labelMotor0 = new QLabel(groupBoxStatus);
        labelMotor0->setObjectName(QString::fromUtf8("labelMotor0"));

        gridLayoutStatus->addWidget(labelMotor0, 1, 0, 1, 1);

        labelActualPos0 = new QLabel(groupBoxStatus);
        labelActualPos0->setObjectName(QString::fromUtf8("labelActualPos0"));

        gridLayoutStatus->addWidget(labelActualPos0, 1, 1, 1, 1);

        labelActualVel0 = new QLabel(groupBoxStatus);
        labelActualVel0->setObjectName(QString::fromUtf8("labelActualVel0"));

        gridLayoutStatus->addWidget(labelActualVel0, 1, 2, 1, 1);

        labelStatus0 = new QLabel(groupBoxStatus);
        labelStatus0->setObjectName(QString::fromUtf8("labelStatus0"));

        gridLayoutStatus->addWidget(labelStatus0, 1, 3, 1, 1);

        checkEnable0 = new QCheckBox(groupBoxStatus);
        checkEnable0->setObjectName(QString::fromUtf8("checkEnable0"));
        checkEnable0->setChecked(true);

        gridLayoutStatus->addWidget(checkEnable0, 1, 4, 1, 1);

        labelMotor1 = new QLabel(groupBoxStatus);
        labelMotor1->setObjectName(QString::fromUtf8("labelMotor1"));

        gridLayoutStatus->addWidget(labelMotor1, 2, 0, 1, 1);

        labelActualPos1 = new QLabel(groupBoxStatus);
        labelActualPos1->setObjectName(QString::fromUtf8("labelActualPos1"));

        gridLayoutStatus->addWidget(labelActualPos1, 2, 1, 1, 1);

        labelActualVel1 = new QLabel(groupBoxStatus);
        labelActualVel1->setObjectName(QString::fromUtf8("labelActualVel1"));

        gridLayoutStatus->addWidget(labelActualVel1, 2, 2, 1, 1);

        labelStatus1 = new QLabel(groupBoxStatus);
        labelStatus1->setObjectName(QString::fromUtf8("labelStatus1"));

        gridLayoutStatus->addWidget(labelStatus1, 2, 3, 1, 1);

        checkEnable1 = new QCheckBox(groupBoxStatus);
        checkEnable1->setObjectName(QString::fromUtf8("checkEnable1"));
        checkEnable1->setChecked(true);

        gridLayoutStatus->addWidget(checkEnable1, 2, 4, 1, 1);

        labelMotor2 = new QLabel(groupBoxStatus);
        labelMotor2->setObjectName(QString::fromUtf8("labelMotor2"));

        gridLayoutStatus->addWidget(labelMotor2, 3, 0, 1, 1);

        labelActualPos2 = new QLabel(groupBoxStatus);
        labelActualPos2->setObjectName(QString::fromUtf8("labelActualPos2"));

        gridLayoutStatus->addWidget(labelActualPos2, 3, 1, 1, 1);

        labelActualVel2 = new QLabel(groupBoxStatus);
        labelActualVel2->setObjectName(QString::fromUtf8("labelActualVel2"));

        gridLayoutStatus->addWidget(labelActualVel2, 3, 2, 1, 1);

        labelStatus2 = new QLabel(groupBoxStatus);
        labelStatus2->setObjectName(QString::fromUtf8("labelStatus2"));

        gridLayoutStatus->addWidget(labelStatus2, 3, 3, 1, 1);

        checkEnable2 = new QCheckBox(groupBoxStatus);
        checkEnable2->setObjectName(QString::fromUtf8("checkEnable2"));
        checkEnable2->setChecked(true);

        gridLayoutStatus->addWidget(checkEnable2, 3, 4, 1, 1);

        labelMotor3 = new QLabel(groupBoxStatus);
        labelMotor3->setObjectName(QString::fromUtf8("labelMotor3"));

        gridLayoutStatus->addWidget(labelMotor3, 4, 0, 1, 1);

        labelActualPos3 = new QLabel(groupBoxStatus);
        labelActualPos3->setObjectName(QString::fromUtf8("labelActualPos3"));

        gridLayoutStatus->addWidget(labelActualPos3, 4, 1, 1, 1);

        labelActualVel3 = new QLabel(groupBoxStatus);
        labelActualVel3->setObjectName(QString::fromUtf8("labelActualVel3"));

        gridLayoutStatus->addWidget(labelActualVel3, 4, 2, 1, 1);

        labelStatus3 = new QLabel(groupBoxStatus);
        labelStatus3->setObjectName(QString::fromUtf8("labelStatus3"));

        gridLayoutStatus->addWidget(labelStatus3, 4, 3, 1, 1);

        checkEnable3 = new QCheckBox(groupBoxStatus);
        checkEnable3->setObjectName(QString::fromUtf8("checkEnable3"));
        checkEnable3->setChecked(true);

        gridLayoutStatus->addWidget(checkEnable3, 4, 4, 1, 1);

        labelMotor4 = new QLabel(groupBoxStatus);
        labelMotor4->setObjectName(QString::fromUtf8("labelMotor4"));

        gridLayoutStatus->addWidget(labelMotor4, 5, 0, 1, 1);

        labelActualPos4 = new QLabel(groupBoxStatus);
        labelActualPos4->setObjectName(QString::fromUtf8("labelActualPos4"));

        gridLayoutStatus->addWidget(labelActualPos4, 5, 1, 1, 1);

        labelActualVel4 = new QLabel(groupBoxStatus);
        labelActualVel4->setObjectName(QString::fromUtf8("labelActualVel4"));

        gridLayoutStatus->addWidget(labelActualVel4, 5, 2, 1, 1);

        labelStatus4 = new QLabel(groupBoxStatus);
        labelStatus4->setObjectName(QString::fromUtf8("labelStatus4"));

        gridLayoutStatus->addWidget(labelStatus4, 5, 3, 1, 1);

        checkEnable4 = new QCheckBox(groupBoxStatus);
        checkEnable4->setObjectName(QString::fromUtf8("checkEnable4"));
        checkEnable4->setChecked(true);

        gridLayoutStatus->addWidget(checkEnable4, 5, 4, 1, 1);

        labelMotor5 = new QLabel(groupBoxStatus);
        labelMotor5->setObjectName(QString::fromUtf8("labelMotor5"));

        gridLayoutStatus->addWidget(labelMotor5, 6, 0, 1, 1);

        labelActualPos5 = new QLabel(groupBoxStatus);
        labelActualPos5->setObjectName(QString::fromUtf8("labelActualPos5"));

        gridLayoutStatus->addWidget(labelActualPos5, 6, 1, 1, 1);

        labelActualVel5 = new QLabel(groupBoxStatus);
        labelActualVel5->setObjectName(QString::fromUtf8("labelActualVel5"));

        gridLayoutStatus->addWidget(labelActualVel5, 6, 2, 1, 1);

        labelStatus5 = new QLabel(groupBoxStatus);
        labelStatus5->setObjectName(QString::fromUtf8("labelStatus5"));

        gridLayoutStatus->addWidget(labelStatus5, 6, 3, 1, 1);

        checkEnable5 = new QCheckBox(groupBoxStatus);
        checkEnable5->setObjectName(QString::fromUtf8("checkEnable5"));
        checkEnable5->setChecked(true);

        gridLayoutStatus->addWidget(checkEnable5, 6, 4, 1, 1);


        verticalLayout->addWidget(groupBoxStatus);

        groupBoxCommand = new QGroupBox(ArmWidget);
        groupBoxCommand->setObjectName(QString::fromUtf8("groupBoxCommand"));
        gridLayoutCommand = new QGridLayout(groupBoxCommand);
        gridLayoutCommand->setObjectName(QString::fromUtf8("gridLayoutCommand"));
        label_5 = new QLabel(groupBoxCommand);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayoutCommand->addWidget(label_5, 0, 0, 1, 1);

        spinVel0 = new QDoubleSpinBox(groupBoxCommand);
        spinVel0->setObjectName(QString::fromUtf8("spinVel0"));
        spinVel0->setDecimals(2);
        spinVel0->setMinimum(-99999.000000000000000);
        spinVel0->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinVel0, 0, 1, 1, 1);

        spinVel1 = new QDoubleSpinBox(groupBoxCommand);
        spinVel1->setObjectName(QString::fromUtf8("spinVel1"));
        spinVel1->setDecimals(2);
        spinVel1->setMinimum(-99999.000000000000000);
        spinVel1->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinVel1, 0, 2, 1, 1);

        spinVel2 = new QDoubleSpinBox(groupBoxCommand);
        spinVel2->setObjectName(QString::fromUtf8("spinVel2"));
        spinVel2->setDecimals(2);
        spinVel2->setMinimum(-99999.000000000000000);
        spinVel2->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinVel2, 0, 3, 1, 1);

        spinVel3 = new QDoubleSpinBox(groupBoxCommand);
        spinVel3->setObjectName(QString::fromUtf8("spinVel3"));
        spinVel3->setDecimals(2);
        spinVel3->setMinimum(-99999.000000000000000);
        spinVel3->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinVel3, 0, 4, 1, 1);

        spinVel4 = new QDoubleSpinBox(groupBoxCommand);
        spinVel4->setObjectName(QString::fromUtf8("spinVel4"));
        spinVel4->setDecimals(2);
        spinVel4->setMinimum(-99999.000000000000000);
        spinVel4->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinVel4, 0, 5, 1, 1);

        spinVel5 = new QDoubleSpinBox(groupBoxCommand);
        spinVel5->setObjectName(QString::fromUtf8("spinVel5"));
        spinVel5->setDecimals(2);
        spinVel5->setMinimum(-99999.000000000000000);
        spinVel5->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinVel5, 0, 6, 1, 1);

        btnSendVel = new QPushButton(groupBoxCommand);
        btnSendVel->setObjectName(QString::fromUtf8("btnSendVel"));

        gridLayoutCommand->addWidget(btnSendVel, 0, 7, 1, 1);

        label_6 = new QLabel(groupBoxCommand);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayoutCommand->addWidget(label_6, 1, 0, 1, 1);

        spinPos0 = new QDoubleSpinBox(groupBoxCommand);
        spinPos0->setObjectName(QString::fromUtf8("spinPos0"));
        spinPos0->setDecimals(2);
        spinPos0->setMinimum(-99999.000000000000000);
        spinPos0->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinPos0, 1, 1, 1, 1);

        spinPos1 = new QDoubleSpinBox(groupBoxCommand);
        spinPos1->setObjectName(QString::fromUtf8("spinPos1"));
        spinPos1->setDecimals(2);
        spinPos1->setMinimum(-99999.000000000000000);
        spinPos1->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinPos1, 1, 2, 1, 1);

        spinPos2 = new QDoubleSpinBox(groupBoxCommand);
        spinPos2->setObjectName(QString::fromUtf8("spinPos2"));
        spinPos2->setDecimals(2);
        spinPos2->setMinimum(-99999.000000000000000);
        spinPos2->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinPos2, 1, 3, 1, 1);

        spinPos3 = new QDoubleSpinBox(groupBoxCommand);
        spinPos3->setObjectName(QString::fromUtf8("spinPos3"));
        spinPos3->setDecimals(2);
        spinPos3->setMinimum(-99999.000000000000000);
        spinPos3->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinPos3, 1, 4, 1, 1);

        spinPos4 = new QDoubleSpinBox(groupBoxCommand);
        spinPos4->setObjectName(QString::fromUtf8("spinPos4"));
        spinPos4->setDecimals(2);
        spinPos4->setMinimum(-99999.000000000000000);
        spinPos4->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinPos4, 1, 5, 1, 1);

        spinPos5 = new QDoubleSpinBox(groupBoxCommand);
        spinPos5->setObjectName(QString::fromUtf8("spinPos5"));
        spinPos5->setDecimals(2);
        spinPos5->setMinimum(-99999.000000000000000);
        spinPos5->setMaximum(99999.000000000000000);

        gridLayoutCommand->addWidget(spinPos5, 1, 6, 1, 1);

        btnSendRelaPos = new QPushButton(groupBoxCommand);
        btnSendRelaPos->setObjectName(QString::fromUtf8("btnSendRelaPos"));

        gridLayoutCommand->addWidget(btnSendRelaPos, 1, 7, 1, 1);

        btnSendAbsPos = new QPushButton(groupBoxCommand);
        btnSendAbsPos->setObjectName(QString::fromUtf8("btnSendAbsPos"));

        gridLayoutCommand->addWidget(btnSendAbsPos, 2, 7, 1, 1);

        btnAllZero = new QPushButton(groupBoxCommand);
        btnAllZero->setObjectName(QString::fromUtf8("btnAllZero"));

        gridLayoutCommand->addWidget(btnAllZero, 3, 0, 1, 1);

        btnEnableAll = new QPushButton(groupBoxCommand);
        btnEnableAll->setObjectName(QString::fromUtf8("btnEnableAll"));

        gridLayoutCommand->addWidget(btnEnableAll, 3, 1, 1, 1);

        btnDisableAll = new QPushButton(groupBoxCommand);
        btnDisableAll->setObjectName(QString::fromUtf8("btnDisableAll"));

        gridLayoutCommand->addWidget(btnDisableAll, 3, 2, 1, 1);

        btnEmergencyStop = new QPushButton(groupBoxCommand);
        btnEmergencyStop->setObjectName(QString::fromUtf8("btnEmergencyStop"));
        btnEmergencyStop->setStyleSheet(QString::fromUtf8("background-color: #ff4444; color: white;"));

        gridLayoutCommand->addWidget(btnEmergencyStop, 3, 7, 1, 1);


        verticalLayout->addWidget(groupBoxCommand);

        groupBoxFsa = new QGroupBox(ArmWidget);
        groupBoxFsa->setObjectName(QString::fromUtf8("groupBoxFsa"));
        horizontalLayoutFsa = new QHBoxLayout(groupBoxFsa);
        horizontalLayoutFsa->setObjectName(QString::fromUtf8("horizontalLayoutFsa"));
        btnFsaShutdown = new QPushButton(groupBoxFsa);
        btnFsaShutdown->setObjectName(QString::fromUtf8("btnFsaShutdown"));

        horizontalLayoutFsa->addWidget(btnFsaShutdown);

        btnFsaSwitchOn = new QPushButton(groupBoxFsa);
        btnFsaSwitchOn->setObjectName(QString::fromUtf8("btnFsaSwitchOn"));

        horizontalLayoutFsa->addWidget(btnFsaSwitchOn);

        btnFsaEnableOp = new QPushButton(groupBoxFsa);
        btnFsaEnableOp->setObjectName(QString::fromUtf8("btnFsaEnableOp"));

        horizontalLayoutFsa->addWidget(btnFsaEnableOp);

        btnFsaHalt = new QPushButton(groupBoxFsa);
        btnFsaHalt->setObjectName(QString::fromUtf8("btnFsaHalt"));

        horizontalLayoutFsa->addWidget(btnFsaHalt);

        btnClearError = new QPushButton(groupBoxFsa);
        btnClearError->setObjectName(QString::fromUtf8("btnClearError"));

        horizontalLayoutFsa->addWidget(btnClearError);


        verticalLayout->addWidget(groupBoxFsa);


        retranslateUi(ArmWidget);

        QMetaObject::connectSlotsByName(ArmWidget);
    } // setupUi

    void retranslateUi(QWidget *ArmWidget)
    {
        ArmWidget->setWindowTitle(QApplication::translate("ArmWidget", "\346\234\272\346\242\260\350\207\202\346\216\247\345\210\266", nullptr));
        groupBoxStatus->setTitle(QApplication::translate("ArmWidget", "\347\224\265\346\234\272\347\212\266\346\200\201", nullptr));
        label->setText(QApplication::translate("ArmWidget", "\347\224\265\346\234\272", nullptr));
        label_2->setText(QApplication::translate("ArmWidget", "\345\256\236\351\231\205\344\275\215\347\275\256", nullptr));
        label_3->setText(QApplication::translate("ArmWidget", "\345\256\236\351\231\205\351\200\237\345\272\246", nullptr));
        label_4->setText(QApplication::translate("ArmWidget", "\347\212\266\346\200\201\345\255\227", nullptr));
        label_7->setText(QApplication::translate("ArmWidget", "\344\275\277\350\203\275", nullptr));
        labelMotor0->setText(QApplication::translate("ArmWidget", "\350\207\202\347\224\265\346\234\2721", nullptr));
        labelActualPos0->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelActualVel0->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelStatus0->setText(QApplication::translate("ArmWidget", "0x0000", nullptr));
        checkEnable0->setText(QString());
        labelMotor1->setText(QApplication::translate("ArmWidget", "\350\207\202\347\224\265\346\234\2722", nullptr));
        labelActualPos1->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelActualVel1->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelStatus1->setText(QApplication::translate("ArmWidget", "0x0000", nullptr));
        checkEnable1->setText(QString());
        labelMotor2->setText(QApplication::translate("ArmWidget", "\350\207\202\347\224\265\346\234\2723", nullptr));
        labelActualPos2->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelActualVel2->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelStatus2->setText(QApplication::translate("ArmWidget", "0x0000", nullptr));
        checkEnable2->setText(QString());
        labelMotor3->setText(QApplication::translate("ArmWidget", "\346\211\230\346\235\277\347\224\265\346\234\2721", nullptr));
        labelActualPos3->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelActualVel3->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelStatus3->setText(QApplication::translate("ArmWidget", "0x0000", nullptr));
        checkEnable3->setText(QString());
        labelMotor4->setText(QApplication::translate("ArmWidget", "\346\211\230\346\235\277\347\224\265\346\234\2722", nullptr));
        labelActualPos4->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelActualVel4->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelStatus4->setText(QApplication::translate("ArmWidget", "0x0000", nullptr));
        checkEnable4->setText(QString());
        labelMotor5->setText(QApplication::translate("ArmWidget", "\346\211\230\346\235\277\347\224\265\346\234\2723", nullptr));
        labelActualPos5->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelActualVel5->setText(QApplication::translate("ArmWidget", "0.00", nullptr));
        labelStatus5->setText(QApplication::translate("ArmWidget", "0x0000", nullptr));
        checkEnable5->setText(QString());
        groupBoxCommand->setTitle(QApplication::translate("ArmWidget", "\345\221\275\344\273\244\344\270\213\345\217\221", nullptr));
        label_5->setText(QApplication::translate("ArmWidget", "\347\233\256\346\240\207\351\200\237\345\272\246", nullptr));
        btnSendVel->setText(QApplication::translate("ArmWidget", "\345\217\221\351\200\201\351\200\237\345\272\246", nullptr));
        label_6->setText(QApplication::translate("ArmWidget", "\347\233\256\346\240\207\344\275\215\347\275\256", nullptr));
        btnSendRelaPos->setText(QApplication::translate("ArmWidget", "\347\233\270\345\257\271\344\275\215\347\275\256", nullptr));
        btnSendAbsPos->setText(QApplication::translate("ArmWidget", "\347\273\235\345\257\271\344\275\215\347\275\256", nullptr));
        btnAllZero->setText(QApplication::translate("ArmWidget", "\345\205\250\351\203\250\345\275\222\351\233\266", nullptr));
        btnEnableAll->setText(QApplication::translate("ArmWidget", "\345\205\250\351\203\250\344\275\277\350\203\275", nullptr));
        btnDisableAll->setText(QApplication::translate("ArmWidget", "\345\205\250\351\203\250\347\246\201\347\224\250", nullptr));
        btnEmergencyStop->setText(QApplication::translate("ArmWidget", "\347\264\247\346\200\245\345\201\234\346\255\242", nullptr));
        groupBoxFsa->setTitle(QApplication::translate("ArmWidget", "\347\212\266\346\200\201\346\234\272\346\216\247\345\210\266", nullptr));
        btnFsaShutdown->setText(QApplication::translate("ArmWidget", "Shutdown", nullptr));
        btnFsaSwitchOn->setText(QApplication::translate("ArmWidget", "Switch On", nullptr));
        btnFsaEnableOp->setText(QApplication::translate("ArmWidget", "Enable Op", nullptr));
        btnFsaHalt->setText(QApplication::translate("ArmWidget", "Halt", nullptr));
        btnClearError->setText(QApplication::translate("ArmWidget", "Clear Error", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ArmWidget: public Ui_ArmWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ARMWIDGET_H
