#include "pagefilemsgdisplay.h"
#include "ui_pagefilemsgdisplay.h"
#include "filehandle.h"
#include "QDebug"
#define PAGEFILE_DBG(x...) qDebug(x)

PageFileMsgDisplay::PageFileMsgDisplay(uint8_t src) : src(src), ui(new Ui::PageFileMsgDisplay)
{
    ui->setupUi(this);
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

void PageFileMsgDisplay::parse(uint32_t pgn, uint8_t *data, uint16_t len)
{
    for (PageMsgDisplay *p : msg_page_map)
    {
        if (p->msgs_map.contains(pgn))
        {
            MsgData *msg_data = p->msgs_map.value(pgn);
            msg_data->decode(data, len);
            // break;
        }
    }
}
