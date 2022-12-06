#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "comm/comm_j1939.h"

void MainWindow::slot_test_send()
{
    // uint8_t test[8];
    // uint    ret = CommJ1939::j1939_boot_msg_send(0x1200, 6, 20, test, 8, 200);
    // (void)ret;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    J1939Ins->init();
    // boot_port_init();
    connect(&test_timer, &QTimer::timeout, this, &MainWindow::slot_test_send);
    test_timer.start(1000);
    msgs.load_json("./cfg/", "bms_protocol_bmu_bcu.json");
    ui->tabWidget->addTab(new PageMsgDisplay(msgs.msgs_map), " 主机");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openDevicePushButton_clicked()
{
    if (!device_status)
    {
        device_status = J1939Ins->open_device(0, 500000);
        if (device_status)
        {
            ui->openDevicePushButton->setText("关闭设备");
        }
        else
        {
            ui->openDevicePushButton->setText("打开失败");
        }
    }
    else
    {
        J1939Ins->close_device();
        device_status = false;
        ui->openDevicePushButton->setText("打开设备");
    }
}
