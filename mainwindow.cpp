#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "comm/comm_j1939.h"
#include "comm_j1939_port.h"
#include "j1939_modbus_master.h"
#include "frmbootloader.h"
#include "json_file.h"
#include "paramshandle.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    J1939Ins->init();
    J1939DbIns->init();
    msgs = new MsgSignals(true);
    comm_j1939_port_init(msgs->msgs_map);
    ui->tabWidget->addTab(new frmBootloader, "固件升级");
    ui->tabWidget->addTab(new PageMsgDisplay(msgs->msgs_map, false), " 主机");
    connect(J1939Ins, &CommJ1939::sig_recv_pgn_handle, this, &MainWindow::slot_recv_pgn_handle);

    load_cfg_data("./temp/cfg_temp.json");
    on_openDevicePushButton_clicked();
}

MainWindow::~MainWindow()
{
    save_cfg_data("./temp/cfg_temp.json");
    delete msgs;
    delete ui;
}

void MainWindow::save_cfg_data(QString path)
{
    QJsonObject root;
    root.insert(QString("%1").arg("can_index"), ui->canIndexSpinBox->value());
    root.insert(QString("%1").arg("can_baudrate"), ui->baudrateSpinBox->value());
    root.insert(QString("%1").arg("obj_addr"), ui->objAddrspinBox->value());
    root.insert(QString("%1").arg("src_addr"), ui->srcAddrSpinBox->value());
    QJsonDocument tempJdoc(root);
    write_json_file(path, &tempJdoc);
}

void MainWindow::load_cfg_data(QString path)
{
    QJsonObject root;
    int         value = 0;
    if (load_json_file(path, &root))
    {
        QJsonObject root_obj = root;
        if (root_obj.contains("can_index"))
        {
            value = root_obj.value("can_index").toInt();
            ui->canIndexSpinBox->setValue(value);
        }
        if (root_obj.contains("can_baudrate"))
        {
            value = root_obj.value("can_baudrate").toInt();
            ui->baudrateSpinBox->setValue(value);
        }
        if (root_obj.contains("obj_addr"))
        {
            value = root_obj.value("obj_addr").toInt();
            ui->objAddrspinBox->setValue(value);
        }
        if (root_obj.contains("src_addr"))
        {
            value = root_obj.value("src_addr").toInt();
            ui->srcAddrSpinBox->setValue(value);
        }
    }
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
