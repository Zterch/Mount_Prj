/********************************************************************************
** Form generated from reading UI file 'logwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGWIDGET_H
#define UI_LOGWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LogWidget
{
public:
    QVBoxLayout *verticalLayout;
    QTextEdit *textLog;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *comboLogLevel;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnClear;

    void setupUi(QWidget *LogWidget)
    {
        if (LogWidget->objectName().isEmpty())
            LogWidget->setObjectName(QString::fromUtf8("LogWidget"));
        LogWidget->resize(400, 600);
        verticalLayout = new QVBoxLayout(LogWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        textLog = new QTextEdit(LogWidget);
        textLog->setObjectName(QString::fromUtf8("textLog"));
        textLog->setReadOnly(true);

        verticalLayout->addWidget(textLog);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(LogWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        comboLogLevel = new QComboBox(LogWidget);
        comboLogLevel->setObjectName(QString::fromUtf8("comboLogLevel"));

        horizontalLayout->addWidget(comboLogLevel);

        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        btnClear = new QPushButton(LogWidget);
        btnClear->setObjectName(QString::fromUtf8("btnClear"));

        horizontalLayout->addWidget(btnClear);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(LogWidget);

        QMetaObject::connectSlotsByName(LogWidget);
    } // setupUi

    void retranslateUi(QWidget *LogWidget)
    {
        LogWidget->setWindowTitle(QApplication::translate("LogWidget", "\350\277\220\350\241\214\346\227\245\345\277\227", nullptr));
        label->setText(QApplication::translate("LogWidget", "\346\227\245\345\277\227\347\272\247\345\210\253", nullptr));
        btnClear->setText(QApplication::translate("LogWidget", "\346\270\205\347\251\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogWidget: public Ui_LogWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGWIDGET_H
