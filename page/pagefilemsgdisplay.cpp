#include "pagefilemsgdisplay.h"
#include "ui_pagefilemsgdisplay.h"

PageFileMsgDisplay::PageFileMsgDisplay(uint8_t src) : src(src), ui(new Ui::PageFileMsgDisplay)
{
    ui->setupUi(this);
    MsgSignals *msg = new MsgSignals();
    for (QString msg_key : msg->file_msg_map.keys())
    {
        PageMsgDisplay *page = new PageMsgDisplay(msg->file_msg_map.value(msg_key));
        msg_page_map.insert(msg_key, page);
        ui->tabWidget->addTab(page, msg_key);
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
            break;
        }
    }
}
