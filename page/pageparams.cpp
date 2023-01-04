#include "pageparams.h"
#include "ui_pageparams.h"
#include "QTextBrowser"
#include "paramshandle.h"
#include "QDebug"

#define PAGEPARAM_DBG(x...) qDebug(x)

PageParams::PageParams() : ui(new Ui::PageParams)
{
    ui->setupUi(this);
    int cloumnIndex = 0;
    ui->tableWidget->setRowCount(params.param_vector.count());
    for (int i = 0; i < params.param_vector.count(); i++)
    {
        ParamData *p = params.param_vector[i];
        cloumnIndex  = 0;
        ui->tableWidget->setItem(i, cloumnIndex++, new QTableWidgetItem(p->name));
        ui->tableWidget->setCellWidget(i, cloumnIndex++, p->recv_widget.text);
        ui->tableWidget->setCellWidget(i, cloumnIndex++, p->send_widget.text);
    }
    // ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(&request_get_timer, &QTimer::timeout, this, &PageParams::slot_request_get_timeout);
    request_get_timer.setInterval(5);
    request_get_timer.stop();

    connect(&request_set_timer, &QTimer::timeout, this, &PageParams::slot_request_set_timeout);
    request_set_timer.setInterval(5);
    request_set_timer.stop();
}

PageParams::~PageParams()
{
    delete ui;
}

void PageParams::slot_request_get_timeout()
{
    if (request_get_index >= params.param_vector.count())
    {
        request_get_timer.stop();
        PAGEPARAM_DBG("请求参数列表完成");
    }
    else
    {
        ParamData *p = params.param_vector[request_get_index];
        p->request_get_value();
        request_get_index++;
    }
}

void PageParams::slot_request_set_timeout()
{
    if (request_set_index >= params.param_vector.count())
    {
        request_set_timer.stop();
        PAGEPARAM_DBG("请求设置参数列表完成");
    }
    else
    {
        ParamData *p = params.param_vector[request_set_index];
        p->slot_encode_send();
        request_set_index++;
    }
}

void PageParams::on_tableWidget_cellClicked(int row, int column)
{
    if (column < 2)
    {
        ParamData *p = params.param_vector[row];
        p->request_get_value();
    }
}

void PageParams::on_requestPushButton_clicked()
{
    request_get_index = 0;
    request_get_timer.start();
    PAGEPARAM_DBG("开始请求参数列表");
}

void PageParams::on_setButton_clicked()
{
    request_set_index = 0;
    request_set_timer.start();
    PAGEPARAM_DBG("开始设置参数列表");
}
