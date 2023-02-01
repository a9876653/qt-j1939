#include "pageparams.h"
#include "ui_pageparams.h"
#include "QTextBrowser"
#include "paramshandle.h"
#include "QDebug"

#define PAGEPARAM_DBG(x...) qDebug(x)

PageParams::PageParams(int src_addr) : src_addr(src_addr), ui(new Ui::PageParams)
{
    ui->setupUi(this);

    params = new ParamsHandle(src_addr);
}

void PageParams::showEvent(QShowEvent *event)
{
    (void)event;

    params->auto_get_param_table(is_auto_get);
}

void PageParams::hideEvent(QHideEvent *event)
{
    (void)event;
    params->auto_get_param_table(false);
}

PageParams::~PageParams()
{
    delete ui;
}

void PageParams::load_json(QString path)
{
    params->load_json(path);
    int cloumnIndex = 0;
    ui->tableWidget->setRowCount(params->param_map.count());
    int i = 0;
    for (ParamData *p : params->param_map)
    {
        cloumnIndex = 0;
        ui->tableWidget->setItem(i, cloumnIndex++, new QTableWidgetItem(p->name));
        // ui->tableWidget->setItem(i, cloumnIndex++, new QTableWidgetItem(p->id));
        ui->tableWidget->setCellWidget(i, cloumnIndex++, p->recv_widget.text);
        ui->tableWidget->setCellWidget(i, cloumnIndex++, p->send_widget.text);
        ui->tableWidget->setCellWidget(i, cloumnIndex++, p->icon_widget);
        i++;
    }
    // ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void PageParams::on_tableWidget_cellClicked(int row, int column)
{
    if (column < 2)
    {
        int        id = params->param_map.keys().at(row);
        ParamData *p  = params->param_map.value(id);
        p->slot_request_read();
    }
}

void PageParams::on_requestPushButton_clicked()
{
    params->get_param_table();
}

void PageParams::on_setButton_clicked()
{
    params->set_param_table();
}

void PageParams::on_autoCheckBox_stateChanged(int arg1)
{
    is_auto_get = arg1;
    params->auto_get_param_table(is_auto_get);
}
