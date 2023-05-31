#include "pagefilemsgdisplay.h"
#include "ui_pagefilemsgdisplay.h"
#include "filehandle.h"
#include "QDebug"
#include "pagereadevent.h"
#include "system.h"
#define PAGEFILE_DBG(x...) qDebug(x)

PageFileMsgDisplay::PageFileMsgDisplay(uint8_t src) : src(src), ui(new Ui::PageFileMsgDisplay)
{
    ui->setupUi(this);
    udp_socket                = new QUdpSocket(this);
    MsgSignals    *msg        = new MsgSignals();
    PageReadEvent *read_event = new PageReadEvent(src);
    ui->tabWidget->addTab(read_event, "事件读取");
    for (QString msg_key : msg->file_msg_map.keys())
    {
        PageMsgDisplay *page = new PageMsgDisplay(msg->file_msg_map.value(msg_key), src);
        msg_page_map.insert(msg_key, page);
        ui->tabWidget->addTab(page, msg_key);
    }
    csv_time_key = "时间";
    csv_map.insert(csv_time_key, get_current_time_ms_qstring());
    for (MsgData *m : msg->msgs_map)
    {
        for (Signal *sig : m->signals_list)
        {
            csv_map.insert(sig->name, sig->value_s);
        }
    }

    QDateTime time;
    QDate     time_data = time.currentDateTime().date();
    QString   time_s    = QString("%1%2%3")
                         .arg(time_data.year())
                         .arg(time_data.month(), 2, 10, QChar('0'))
                         .arg(time_data.day(), 2, 10, QChar('0'));
    csv_path = QString("./save_data/%1-%2.csv").arg(time_s).arg(src);

    connect(&csv_timer, &QTimer::timeout, this, &PageFileMsgDisplay::csv_save_file);
    csv_timer.start(1000);
}

PageFileMsgDisplay::~PageFileMsgDisplay()
{
    delete ui;
}

void PageFileMsgDisplay::msg_data_polt_send(MsgData *msg_data)
{
    QJsonObject root;
    QJsonObject src_obj;
    QJsonObject node;

    for (Signal *s : msg_data->signals_list)
    {
        QString name = QString("%1").arg(s->name);
        if (s->is_float)
        {
            node.insert(name, s->value_f);
        }
        else if (s->is_signed)
        {
            node.insert(name, s->value_i);
        }
        else
        {
            node.insert(name, (int)s->value_u);
        }
    }
    src_obj.insert(msg_data->name, node);
    root.insert(QString("src %1").arg(src), src_obj);
    QJsonDocument tempJdoc(root);
    QByteArray    array = tempJdoc.toJson();
    //发送数据，参数1:要发送的数据char*类型，参数2:撒送数据的大小，参数3:ip地址,参数4:端口号
    udp_socket->writeDatagram(array.data(), array.size(), QHostAddress("127.0.0.1"), 9870);
}

void PageFileMsgDisplay::write_csv_data(MsgData *msg_data)
{
    if (csv_map.contains(csv_time_key) && msg_data != nullptr)
    {
        csv_map[csv_time_key] = get_current_time_ms_qstring();
        for (Signal *s : msg_data->signals_list)
        {
            if (csv_map.contains(s->name))
            {
                csv_map[s->name] = s->value_s;
            }
        }
    }
}

void PageFileMsgDisplay::csv_save_file()
{
    QDateTime time;
    QDate     time_data = time.currentDateTime().date();
    QString   time_s    = QString("%1%2%3")
                         .arg(time_data.year())
                         .arg(time_data.month(), 2, 10, QChar('0'))
                         .arg(time_data.day(), 2, 10, QChar('0'));
    csv_path = QString("./save_data/%1-%2.csv").arg(time_s).arg(src);
    CsvAppend(csv_path, csv_map.keys(), csv_map.values());
}

void PageFileMsgDisplay::parse(uint32_t pgn, QByteArray data)
{
    for (PageMsgDisplay *p : msg_page_map)
    {
        if (p->msgs_map.contains(pgn))
        {
            MsgData *msg_data = p->msgs_map.value(pgn);
            msg_data->decode(data);
            msg_data_polt_send(msg_data);
            write_csv_data(msg_data);
            // break;
        }
    }
}
