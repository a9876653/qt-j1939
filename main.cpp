#include "mainwindow.h"
#include "boot_port.h"
#include "j1939_event_type.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<QVector<uint8_t>>("QVector<uint8_t>");
    qRegisterMetaType<app_info_str_t>("app_info_str_t");
    qRegisterMetaType<read_event_respond_t>("read_event_respond_t");
    QApplication a(argc, argv);
    MainWindow   w;
    w.show();
    return a.exec();
}
