#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "comm/comm_j1939.h"
#include "comm_j1939_port.h"
#include "frmbootloader.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    J1939Ins->init();
    msgs = new MsgSignals(true);
    comm_j1939_port_init(msgs->msgs_map);
    ui->tabWidget->addTab(new frmBootloader, "固件升级");
    ui->tabWidget->addTab(new PageMsgDisplay(msgs->msgs_map), " 主机");
    connect(J1939Ins, &CommJ1939::sig_recv_pgn_handle, this, &MainWindow::slot_recv_pgn_handle);

    on_openDevicePushButton_clicked();
}

MainWindow::~MainWindow()
{
    delete msgs;
    delete ui;
}

void MainWindow::on_openDevicePushButton_clicked()
{
    if (!device_status)
    {
        device_status = J1939Ins->open_device(ui->canIndexSpinBox->value(), ui->baudrateSpinBox->value() * 1000);
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

void MainWindow::slot_recv_pgn_handle(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t data_size)
{
    if (!src_page_map.contains(src))
    {
        PageFileMsgDisplay *page = new PageFileMsgDisplay(src);
        ui->tabWidget->addTab(page, QString("从机 - %1").arg(src));
        src_page_map.insert(src, page);
    }
    PageFileMsgDisplay *p = src_page_map.value(src);
    p->parse(pgn, data, data_size);
}

void MainWindow::on_srcAddrSpinBox_valueChanged(int arg1)
{
    J1939Ins->set_src_addr(arg1);
}

void MainWindow::on_objAddrspinBox_valueChanged(int arg1)
{
    J1939Ins->set_dst_addr(arg1);
}
