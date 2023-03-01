#include "pagefilemsgdisplay.h"
#include "ui_pagefilemsgdisplay.h"
#include "filehandle.h"
#include "QDebug"
#define PAGEFILE_DBG(x...) qDebug(x)

PageFileMsgDisplay::PageFileMsgDisplay(uint8_t src) : src(src), ui(new Ui::PageFileMsgDisplay)
{
    ui->setupUi(this);
    udp_socket      = new QUdpSocket(this);
    MsgSignals *msg = new MsgSignals();
    for (QString msg_key : msg->file_msg_map.keys())
    {
        PageMsgDisplay *page = new PageMsgDisplay(msg->file_msg_map.value(msg_key), src);
        msg_page_map.insert(msg_key, page);
        ui->tabWidget->addTab(page, msg_key);
    }
    QString       dir      = "./cfg/regs_json";
    QFileInfoList fileList = GetFileList(dir); //获取目录下所有的文件
    for (QFileInfo info : fileList)
    {
        PAGEFILE_DBG() << "file path:" << info.filePath() << "   file name:" << info.fileName();
        if (info.filePath().contains(".json")) // 只处理JSON文件
        {
            PageParams *param_page = new PageParams(src);
            param_page->load_json(info.filePath());
            page_params_map.insert(info.filePath(), param_page);
            ui->tabWidget->addTab(param_page, info.fileName());
        }
    }
}

PageFileMsgDisplay::~PageFileMsgDisplay()
{
    delete ui;
}

void PageFileMsgDisplay::msg_data_polt_send(MsgData *msg_data)
{
    QJsonObject root;
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

    QJsonDocument tempJdoc(root);
    QByteArray    array = tempJdoc.toJson();
    //发送数据，参数1:要发送的数据char*类型，参数2:撒送数据的大小，参数3:ip地址,参数4:端口号
    udp_socket->writeDatagram(array.data(), array.size(), QHostAddress("127.0.0.1"), 9870);
}

void PageFileMsgDisplay::parse(uint32_t pgn, uint8_t *data, uint16_t len)
{
    for (PageMsgDisplay *p : msg_page_map)
    {
        if (p->msgs_map.contains(pgn))
        {
            MsgData *msg_data = p->msgs_map.value(pgn);
            msg_data->decode(data, len);
            msg_data_polt_send(msg_data);
            // break;
        }
    }
}
