/*
 * logwidget.h
 * 日志显示组件
 */

#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QDateTime>

namespace Ui {
class LogWidget;
}

namespace MountShow {

class LogWidget : public QWidget {
    Q_OBJECT

public:
    explicit LogWidget(QWidget *parent = nullptr);
    ~LogWidget() override;

    // 日志级别：0=信息 1=警告 2=错误
    void appendLog(const QString &msg, int level = 0);

    // 清除日志
    void clearLog();

    // 设置最大日志行数
    void setMaxLineCount(int count);

private:
    void setupUi();

    Ui::LogWidget *ui;
    int m_maxLineCount;
};

} // namespace MountShow

#endif // LOGWIDGET_H
