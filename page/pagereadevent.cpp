#include "pagereadevent.h"
#include "ui_pagereadevent.h"
#include "dataobjmap.h"
#include "filehandle.h"
#include <QFile>
#include <QFileDialog>
#include "QDebug"
#include <QMessageBox>

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
    event->request_event_cnt(read_type);
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

void PageReadEvent::slot_recv_read_event(read_event_respond_t respond)
{
    if (respond.index == (uint32_t)-1 || respond.index == (uint32_t)end_index)
    {
        stop_read();
        return;
    }
    int column = 0;
    int row    = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row + 1);
    if (read_type == SYS_EVENT_TYPE_ERR)
    {
        err_event_t *err_event = (err_event_t *)respond.data;

        event_id_t err_event_id = err_event->id;
        event_id_t event_id     = ERR_EVENT_ID_DECODE(err_event_id);
        event_id_t id           = CELL_EVENT_ID_CONVER(event_id);
        QString    id_s         = QString("%1(0x%2)").arg(id).arg(err_event_id, 4, 16, QLatin1Char('0'));
        QString    start_time   = timestamp_ms_to_qstring(err_event->start_time);
        QString    end_time     = timestamp_ms_to_qstring(err_event->end_time);
        QString    des          = "None";
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
    }
    else if (read_type == SYS_EVENT_TYPE_INFO)
    {
        info_event_respond_t *event = (info_event_respond_t *)respond.data;

        QString start_time = timestamp_ms_to_qstring(event->start_time);

        insert_item(row, column++, "");
        insert_item(row, column++, start_time);
        insert_item(row, column++, "");
        insert_item(row, column++, QString(QLatin1String(event->info.data, strnlen(event->info.data, INFO_SAVE_DATA_MAX_SIZE))));
    }
    read_index++;
    event->request_event(read_type, read_index);
    read_timer.start(read_timeout_ms);
}

void PageReadEvent::start_read(int start_index, int stop_index)
{
    read_index  = start_index;
    end_index   = stop_index;
    timeout_cnt = 0;
    read_timer.start(read_timeout_ms);
    event->request_event(read_type, read_index);
}

void PageReadEvent::stop_read()
{
    read_timer.stop();
    PAGEEVENT_DBG("停止读取事件");
}

void PageReadEvent::read_timeout()
{
    event->request_event(read_type, read_index);
    timeout_cnt++;
    if (timeout_cnt > 5)
    {
        stop_read();
    }
}

void PageReadEvent::on_readEventBtn_clicked()
{
    int row_cnt = ui->tableWidget->rowCount();
    for (int i = 0; i < row_cnt; i++)
    {
        ui->tableWidget->removeRow(0);
    }
    start_read(ui->startSpinBox->value(), ui->readCntSpinBox->value() + 1);
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

void PageReadEvent::on_saveBtn_clicked()
{
    QString filepath = QFileDialog::getSaveFileName(this, tr("Save"), ".", tr(" (*.csv)"));
    if (!filepath.isEmpty())
    {
        int            row    = ui->tableWidget->rowCount();
        int            column = ui->tableWidget->columnCount();
        QList<QString> header_list;
        for (int i = 0; i < column; i++)
        {
            header_list.append(ui->tableWidget->horizontalHeaderItem(i)->text());
        }

        for (int i = 0; i < row; i++)
        {
            QList<QString> str_list;
            for (int j = 0; j < column; j++)
            {
                str_list.append(ui->tableWidget->item(i, j)->text());
            }
            CsvAppend(filepath, header_list, str_list);
        }
    }
}

void PageReadEvent::on_cleanBtn_clicked()
{
    QMessageBox::StandardButton rb
        = QMessageBox::question(NULL, "清除事件", "是否清除所有事件?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (rb == QMessageBox::Yes)
    {
        event->request_event_format(read_type);
        PAGEEVENT_DBG("清除所有事件");
    }
}

void PageReadEvent::on_eventTypeComboBox_currentIndexChanged(int index)
{
    read_type = index;
}
