/*
 * main.cpp
 */

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("MountShow");
    a.setOrganizationName("MountTech");

    MountShow::MainWindow w;
    w.show();
    return a.exec();
}
