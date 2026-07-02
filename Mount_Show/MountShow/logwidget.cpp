/*
 * logwidget.cpp
 */

#include "logwidget.h"
#include "ui_logwidget.h"

namespace MountShow {

LogWidget::LogWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogWidget)
    , m_maxLineCount(1000)
{
    ui->setupUi(this);
    setupUi();
}

LogWidget::~LogWidget()
{
    delete ui;
}

void LogWidget::setupUi()
{
    connect(ui->btnClear, &QPushButton::clicked, this, &LogWidget::clearLog);

    // 默认显示所有级别
    ui->comboLogLevel->addItem("全部", -1);
    ui->comboLogLevel->addItem("信息", 0);
    ui->comboLogLevel->addItem("警告", 1);
    ui->comboLogLevel->addItem("错误", 2);
}

void LogWidget::appendLog(const QString &msg, int level)
{
    QString prefix;
    QColor color;
    switch (level) {
    case 2:
        prefix = "[错误] ";
        color = QColor("#FF6B6B");
        break;
    case 1:
        prefix = "[警告] ";
        color = QColor("#FFA502");
        break;
    default:
        prefix = "[信息] ";
        color = QColor("#2ED573");
        break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString fullMsg = QString("%1 %2%3").arg(timestamp).arg(prefix).arg(msg);

    ui->textLog->append(fullMsg);

    QTextCursor cursor = ui->textLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(cursor);

    // 限制最大行数
    if (ui->textLog->document()->blockCount() > m_maxLineCount) {
        QTextCursor clearCursor(ui->textLog->document());
        clearCursor.movePosition(QTextCursor::Start);
        clearCursor.select(QTextCursor::BlockUnderCursor);
        clearCursor.removeSelectedText();
    }
}

void LogWidget::clearLog()
{
    ui->textLog->clear();
}

void LogWidget::setMaxLineCount(int count)
{
    m_maxLineCount = count;
}

} // namespace MountShow
