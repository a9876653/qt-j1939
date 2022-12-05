#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "comm/comm_j1939.h"
#include "boot_update/boot_port.h"
#include "boot_update/frmbootloader.h"
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
    boot_port_init();
    connect(&test_timer, &QTimer::timeout, this, &MainWindow::slot_test_send);
    test_timer.start(1000);
    ui->gridLayout->addWidget(new frmBootloader);
}

MainWindow::~MainWindow()
{
    delete ui;
}
