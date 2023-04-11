#include "pagereadevent.h"
#include "ui_pagereadevent.h"

PageReadEvent::PageReadEvent(int src, QWidget *parent) : QWidget(parent), ui(new Ui::PageReadEvent), src(src)
{
    ui->setupUi(this);
    event = get_j1939_event_ins(src);
    connect(event, &J1939Event::sig_recv_event_cnt, this, &PageReadEvent::slot_recv_read_event_cnt);
}

PageReadEvent::~PageReadEvent()
{
    delete ui;
}

void PageReadEvent::slot_recv_read_event_cnt(int cnt)
{
    ui->eventCntLabel->setText(QString("%1").arg(cnt));
}

void PageReadEvent::on_readEventCntBtn_clicked()
{
    event->request_event_cnt();
}

void PageReadEvent::insert_item(int row, int column, QString s)
{
    QTableWidgetItem *item = new QTableWidgetItem(s);
    QFont             font = item->font();
    item->setFlags(item->flags() & (~Qt::ItemFlag::ItemIsEditable));
    item->setFont(font);
    ui->tableWidget->setItem(row, column, item);
}

void PageReadEvent::slot_recv_read_event(read_event_respond_t respond)
{
    int column = 0;
    int row    = ui->tableWidget->rowCount() + 1;
    ui->tableWidget->setRowCount(row);

    QString id         = QString("%1").arg(respond.index);
    QString start_time = QString("%1").arg(respond.err_event.start_time);
    QString end_time   = QString("%1").arg(respond.err_event.end_time);

    insert_item(row, column++, id);
    insert_item(row, column++, start_time);
    insert_item(row, column++, end_time);

    // ui->tableWidget->
}

void PageReadEvent::on_pushButton_clicked()
{
}

void PageReadEvent::read_timeout()
{
}
