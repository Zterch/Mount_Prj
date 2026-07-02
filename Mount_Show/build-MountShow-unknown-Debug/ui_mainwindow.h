/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QWidget *controlPanel;
    QVBoxLayout *verticalLayoutControl;
    QGroupBox *groupBoxConnection;
    QFormLayout *formLayoutConnection;
    QLabel *labelSuprIp;
    QLineEdit *lineEditSuprIp;
    QLabel *labelSuprPort;
    QSpinBox *spinBoxSuprPort;
    QHBoxLayout *horizontalLayoutConnection;
    QPushButton *btnConnect;
    QPushButton *btnDisconnect;
    QGroupBox *groupBoxMaster;
    QVBoxLayout *verticalLayoutMaster;
    QPushButton *btnCreateMasters;
    QPushButton *btnScanSlaves;
    QPushButton *btnStartOp;
    QPushButton *btnDestroyMaster;
    QGroupBox *groupBoxGlobalFsa;
    QVBoxLayout *verticalLayoutGlobalFsa;
    QPushButton *btnGlobalShutdown;
    QPushButton *btnGlobalSwitchOn;
    QPushButton *btnGlobalEnableOp;
    QPushButton *btnGlobalHalt;
    QPushButton *btnGlobalClearError;
    QGroupBox *groupBoxExtension;
    QVBoxLayout *verticalLayoutExtension;
    QLabel *labelExtension;
    QSpacerItem *verticalSpacer;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1400, 900);
        MainWindow->setMinimumSize(QSize(1200, 700));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        controlPanel = new QWidget(MainWindow);
        controlPanel->setObjectName(QString::fromUtf8("controlPanel"));
        verticalLayoutControl = new QVBoxLayout(controlPanel);
        verticalLayoutControl->setObjectName(QString::fromUtf8("verticalLayoutControl"));
        groupBoxConnection = new QGroupBox(controlPanel);
        groupBoxConnection->setObjectName(QString::fromUtf8("groupBoxConnection"));
        formLayoutConnection = new QFormLayout(groupBoxConnection);
        formLayoutConnection->setObjectName(QString::fromUtf8("formLayoutConnection"));
        labelSuprIp = new QLabel(groupBoxConnection);
        labelSuprIp->setObjectName(QString::fromUtf8("labelSuprIp"));

        formLayoutConnection->setWidget(0, QFormLayout::LabelRole, labelSuprIp);

        lineEditSuprIp = new QLineEdit(groupBoxConnection);
        lineEditSuprIp->setObjectName(QString::fromUtf8("lineEditSuprIp"));

        formLayoutConnection->setWidget(0, QFormLayout::FieldRole, lineEditSuprIp);

        labelSuprPort = new QLabel(groupBoxConnection);
        labelSuprPort->setObjectName(QString::fromUtf8("labelSuprPort"));

        formLayoutConnection->setWidget(1, QFormLayout::LabelRole, labelSuprPort);

        spinBoxSuprPort = new QSpinBox(groupBoxConnection);
        spinBoxSuprPort->setObjectName(QString::fromUtf8("spinBoxSuprPort"));
        spinBoxSuprPort->setMaximum(65535);
        spinBoxSuprPort->setValue(33333);

        formLayoutConnection->setWidget(1, QFormLayout::FieldRole, spinBoxSuprPort);

        horizontalLayoutConnection = new QHBoxLayout();
        horizontalLayoutConnection->setObjectName(QString::fromUtf8("horizontalLayoutConnection"));
        btnConnect = new QPushButton(groupBoxConnection);
        btnConnect->setObjectName(QString::fromUtf8("btnConnect"));

        horizontalLayoutConnection->addWidget(btnConnect);

        btnDisconnect = new QPushButton(groupBoxConnection);
        btnDisconnect->setObjectName(QString::fromUtf8("btnDisconnect"));
        btnDisconnect->setEnabled(false);

        horizontalLayoutConnection->addWidget(btnDisconnect);


        formLayoutConnection->setLayout(2, QFormLayout::SpanningRole, horizontalLayoutConnection);


        verticalLayoutControl->addWidget(groupBoxConnection);

        groupBoxMaster = new QGroupBox(controlPanel);
        groupBoxMaster->setObjectName(QString::fromUtf8("groupBoxMaster"));
        verticalLayoutMaster = new QVBoxLayout(groupBoxMaster);
        verticalLayoutMaster->setObjectName(QString::fromUtf8("verticalLayoutMaster"));
        btnCreateMasters = new QPushButton(groupBoxMaster);
        btnCreateMasters->setObjectName(QString::fromUtf8("btnCreateMasters"));
        btnCreateMasters->setEnabled(false);

        verticalLayoutMaster->addWidget(btnCreateMasters);

        btnScanSlaves = new QPushButton(groupBoxMaster);
        btnScanSlaves->setObjectName(QString::fromUtf8("btnScanSlaves"));
        btnScanSlaves->setEnabled(false);

        verticalLayoutMaster->addWidget(btnScanSlaves);

        btnStartOp = new QPushButton(groupBoxMaster);
        btnStartOp->setObjectName(QString::fromUtf8("btnStartOp"));
        btnStartOp->setEnabled(false);

        verticalLayoutMaster->addWidget(btnStartOp);

        btnDestroyMaster = new QPushButton(groupBoxMaster);
        btnDestroyMaster->setObjectName(QString::fromUtf8("btnDestroyMaster"));
        btnDestroyMaster->setEnabled(false);

        verticalLayoutMaster->addWidget(btnDestroyMaster);


        verticalLayoutControl->addWidget(groupBoxMaster);

        groupBoxGlobalFsa = new QGroupBox(controlPanel);
        groupBoxGlobalFsa->setObjectName(QString::fromUtf8("groupBoxGlobalFsa"));
        verticalLayoutGlobalFsa = new QVBoxLayout(groupBoxGlobalFsa);
        verticalLayoutGlobalFsa->setObjectName(QString::fromUtf8("verticalLayoutGlobalFsa"));
        btnGlobalShutdown = new QPushButton(groupBoxGlobalFsa);
        btnGlobalShutdown->setObjectName(QString::fromUtf8("btnGlobalShutdown"));
        btnGlobalShutdown->setEnabled(false);

        verticalLayoutGlobalFsa->addWidget(btnGlobalShutdown);

        btnGlobalSwitchOn = new QPushButton(groupBoxGlobalFsa);
        btnGlobalSwitchOn->setObjectName(QString::fromUtf8("btnGlobalSwitchOn"));
        btnGlobalSwitchOn->setEnabled(false);

        verticalLayoutGlobalFsa->addWidget(btnGlobalSwitchOn);

        btnGlobalEnableOp = new QPushButton(groupBoxGlobalFsa);
        btnGlobalEnableOp->setObjectName(QString::fromUtf8("btnGlobalEnableOp"));
        btnGlobalEnableOp->setEnabled(false);

        verticalLayoutGlobalFsa->addWidget(btnGlobalEnableOp);

        btnGlobalHalt = new QPushButton(groupBoxGlobalFsa);
        btnGlobalHalt->setObjectName(QString::fromUtf8("btnGlobalHalt"));
        btnGlobalHalt->setEnabled(false);

        verticalLayoutGlobalFsa->addWidget(btnGlobalHalt);

        btnGlobalClearError = new QPushButton(groupBoxGlobalFsa);
        btnGlobalClearError->setObjectName(QString::fromUtf8("btnGlobalClearError"));
        btnGlobalClearError->setEnabled(false);

        verticalLayoutGlobalFsa->addWidget(btnGlobalClearError);


        verticalLayoutControl->addWidget(groupBoxGlobalFsa);

        groupBoxExtension = new QGroupBox(controlPanel);
        groupBoxExtension->setObjectName(QString::fromUtf8("groupBoxExtension"));
        verticalLayoutExtension = new QVBoxLayout(groupBoxExtension);
        verticalLayoutExtension->setObjectName(QString::fromUtf8("verticalLayoutExtension"));
        labelExtension = new QLabel(groupBoxExtension);
        labelExtension->setObjectName(QString::fromUtf8("labelExtension"));
        labelExtension->setAlignment(Qt::AlignCenter);

        verticalLayoutExtension->addWidget(labelExtension);


        verticalLayoutControl->addWidget(groupBoxExtension);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayoutControl->addItem(verticalSpacer);

        MainWindow->setCentralWidget(controlPanel);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Mount \346\234\272\346\242\260\350\207\202\344\270\212\344\275\215\346\234\272\346\216\247\345\210\266\347\263\273\347\273\237", nullptr));
        groupBoxConnection->setTitle(QApplication::translate("MainWindow", "\350\277\236\346\216\245\346\216\247\345\210\266", nullptr));
        labelSuprIp->setText(QApplication::translate("MainWindow", "supr IP:", nullptr));
        labelSuprPort->setText(QApplication::translate("MainWindow", "supr Port:", nullptr));
        btnConnect->setText(QApplication::translate("MainWindow", "\350\277\236\346\216\245", nullptr));
        btnDisconnect->setText(QApplication::translate("MainWindow", "\346\226\255\345\274\200", nullptr));
        groupBoxMaster->setTitle(QApplication::translate("MainWindow", "\344\270\273\347\253\231\346\223\215\344\275\234", nullptr));
        btnCreateMasters->setText(QApplication::translate("MainWindow", "\345\210\233\345\273\272\344\270\273\347\253\231", nullptr));
        btnScanSlaves->setText(QApplication::translate("MainWindow", "\346\211\253\346\217\217\344\273\216\347\253\231", nullptr));
        btnStartOp->setText(QApplication::translate("MainWindow", "\345\220\257\345\212\250\346\223\215\344\275\234", nullptr));
        btnDestroyMaster->setText(QApplication::translate("MainWindow", "\351\224\200\346\257\201\344\270\273\347\253\231", nullptr));
        groupBoxGlobalFsa->setTitle(QApplication::translate("MainWindow", "\345\205\250\345\261\200\347\212\266\346\200\201\346\234\272", nullptr));
        btnGlobalShutdown->setText(QApplication::translate("MainWindow", "Shutdown", nullptr));
        btnGlobalSwitchOn->setText(QApplication::translate("MainWindow", "Switch On", nullptr));
        btnGlobalEnableOp->setText(QApplication::translate("MainWindow", "Enable Op", nullptr));
        btnGlobalHalt->setText(QApplication::translate("MainWindow", "Halt", nullptr));
        btnGlobalClearError->setText(QApplication::translate("MainWindow", "Clear Error", nullptr));
        groupBoxExtension->setTitle(QApplication::translate("MainWindow", "\346\211\251\345\261\225\346\216\245\345\217\243", nullptr));
        labelExtension->setText(QApplication::translate("MainWindow", "\351\242\204\347\225\231\357\274\232\345\272\225\347\233\230\343\200\201\347\256\227\346\263\225\343\200\201IO\347\255\211", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
