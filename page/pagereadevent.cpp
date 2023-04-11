#include "pagereadevent.h"
#include "ui_pagereadevent.h"
#include "dataobjmap.h"

#include "QDebug"

#define PAGEEVENT_DBG(x...) qDebug(x)

PageReadEvent::PageReadEvent(int src, QWidget *parent) : QWidget(parent), ui(new Ui::PageReadEvent), src(src)
{
    ui->setupUi(this);
    event = get_j1939_event_ins(src);
    while (event == nullptr)
    {
        event = get_j1939_event_ins(src);
    }
    connect(event, &J1939Event::sig_recv_event_cnt, this, &PageReadEvent::slot_recv_read_event_cnt);
    connect(event, &J1939Event::sig_recv_event, this, &PageReadEvent::slot_recv_read_event);

    connect(&read_timer, &QTimer::timeout, this, &PageReadEvent::read_timeout);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //先自适应宽度
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); //然后设置要根据内容使用宽度的列
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
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(row, column, item);
}

QString timestamp_ms_to_qstring(uint64_t timestamp_ms)
{
    if (timestamp_ms == 0)
    {
        return QString("None");
    }
    struct tm    time;
    uint16_t     time_ms  = timestamp_ms % 1000;
    const time_t time_sec = timestamp_ms / 1000;
    gmtime_r(&time_sec, &time);
    time.tm_year = time.tm_year + 1900;
    time.tm_mon  = time.tm_mon + 1;

    return QString("%1/%2/%3-%4:%5:%6:%7")
        .arg(time.tm_year)
        .arg(time.tm_mon)
        .arg(time.tm_mday)
        .arg(time.tm_hour)
        .arg(time.tm_min)
        .arg(time.tm_sec)
        .arg(time_ms);
}

void PageReadEvent::slot_recv_read_event(read_event_respond_t respond)
{
    if (respond.index == (uint32_t)-1)
    {
        stop_read();
        return;
    }
    int column = 0;
    int row    = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row + 1);
    int     err_event_id = respond.err_event.id;
    int     event_id     = ERR_EVENT_ID_DECODE(err_event_id);
    int     id           = CELL_EVENT_ID_CONVER(event_id);
    QString id_s         = QString("%1(0x%2)").arg(id).arg(err_event_id, 4, 16, QLatin1Char('0'));
    QString start_time   = timestamp_ms_to_qstring(respond.err_event.start_time);
    QString end_time     = timestamp_ms_to_qstring(respond.err_event.end_time);
    QString des          = "None";
    if (DataObjMapIns->param_map.contains(id))
    {
        DataObj *obj = DataObjMapIns->param_map.value(id);
        des          = "";
        if (IS_CELL_EVENT(event_id))
        {
            des = QString("电芯 %1 触发故障:").arg(CELL_EVENT_INDEX_CONVER(event_id));
        }
        des += obj->name;
    }

    insert_item(row, column++, id_s);
    insert_item(row, column++, start_time);
    insert_item(row, column++, end_time);
    insert_item(row, column++, des);
    read_index++;
    event->request_event(read_index);
    read_timer.start(read_timeout_ms);
}

void PageReadEvent::start_read(int start_index)
{
    read_index  = start_index;
    timeout_cnt = 0;
    read_timer.start(read_timeout_ms);
    event->request_event(read_index);
}

void PageReadEvent::stop_read()
{
    read_timer.stop();
    PAGEEVENT_DBG("停止读取事件");
}

void PageReadEvent::read_timeout()
{
    event->request_event(read_index);
    timeout_cnt++;
    if (timeout_cnt > 3)
    {
        stop_read();
    }
}

void PageReadEvent::on_readEventBtn_clicked()
{
    start_read(ui->spinBox->value());
}

void PageReadEvent::on_readAllBtn_clicked()
{
    int row_cnt = ui->tableWidget->rowCount();
    for (int i = 0; i < row_cnt; i++)
    {
        ui->tableWidget->removeRow(0);
    }
    start_read();
}
