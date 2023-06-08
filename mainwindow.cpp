#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "comm/comm_j1939.h"
#include "comm_j1939_port.h"
#include "j1939_modbus_master.h"
#include "frmbootloader.h"
#include "json_file.h"
#include "paramshandle.h"
#include "enetconfig.h"
#include "mtableview.h"
#include "mwritereadwidget.h"
#include "mwritereadtable.h"
#include "pageparse.h"
#include <Windows.h>
#include <QDebug>

void alloc_console()
{
    AllocConsole();
    SetConsoleTitleA("调试窗口");
    freopen("CON", "w", stdout); //将输出定向到控制台
    freopen("CON", "w", stderr); //将输出定向到控制台
}

void free_console()
{
    fclose(stdout);
    fclose(stderr);
    FreeConsole();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    alloc_console();

    ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // 隐藏控制台

    J1939Ins;
    J1939DbIns;

    msgs = new MsgSignals(true);

    load_cfg_data("./temp/cfg_temp.json");

    comm_j1939_port_init(msgs->msgs_map);

    ui->tabWidget->addTab(new EnetConfig(), "网卡配置");

    ui->tabWidget->addTab(new frmBootloader, "固件升级");
    src_page_parse = new PageParse(ui->objAddrspinBox->value());
    ui->tabWidget->addTab(src_page_parse, "本地读取服务");
    connect(J1939Ins, &CommJ1939::sig_recv_pgn_handle, this, &MainWindow::slot_recv_pgn_handle, Qt::QueuedConnection);
    connect(J1939Ins, &CommJ1939::sig_open_finish, this, &MainWindow::slot_recv_comm_status, Qt::QueuedConnection);

    on_openDevicePushButton_clicked();
}

MainWindow::~MainWindow()
{
    save_cfg_data("./temp/cfg_temp.json");
    delete ui;
    Singleton<CommJ1939Db>::destory();
    Singleton<CommJ1939>::destory();
    free_console();
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

void MainWindow::slot_recv_comm_status(int ret)
{
    if (ret == STATUS_OK)
    {
        ui->openDevicePushButton->setText("关闭设备");
        device_status = true;
    }
    else if (ret == STATUS_OFFLINE)
    {
        ui->openDevicePushButton->setText("打开设备");
        device_status = false;
    }
    else
    {
        ui->openDevicePushButton->setText("打开失败");
        device_status = false;
    }
}

void MainWindow::on_openDevicePushButton_clicked()
{
    if (!device_status)
    {
        J1939Ins->open_device(ui->canIndexSpinBox->value(), ui->baudrateSpinBox->value() * 1000);
    }
    else
    {
        J1939Ins->close_device();
    }
}

void MainWindow::slot_recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> data)
{
    (void)pgn;
    (void)data;

    if (!src_page_map.contains(src))
    {
        Pages *page   = new Pages();
        page->can_dbc = new PageFileMsgDisplay(src);
        ui->tabWidget->addTab(page->can_dbc, QString("CANDBC解析 %1").arg(src));
        // page->can_modbus = new PageParse(src);
        // ui->tabWidget->addTab(page->can_modbus, QString("MB协议解析 - %1").arg(src));
        src_page_map.insert(src, page);
    }
    Pages *p = src_page_map.value(src);
    p->can_dbc->parse(pgn, data);
}

void MainWindow::on_srcAddrSpinBox_valueChanged(int arg1)
{
    J1939Ins->set_src_addr(arg1);
}

void MainWindow::on_objAddrspinBox_valueChanged(int arg1)
{
    J1939Ins->set_dst_addr(arg1);
    if (src_page_parse != nullptr)
    {
        src_page_parse->set_src_addr(arg1);
    }
}

void MainWindow::on_consoleBtn_clicked()
{
    if (!console_is_show)
    {
        ::ShowWindow(::GetConsoleWindow(), SW_SHOW);
        console_is_show = true;
    }
    else
    {
        ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
        console_is_show = false;
    }
}
