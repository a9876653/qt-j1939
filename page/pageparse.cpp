#include "pageparse.h"
#include "ui_pageparse.h"
#include "filehandle.h"
#include "canard_dsdl.h"
#include "j1939_modbus_master.h"
#include "QDebug"
#include "mlabel.h"
#include "mlabeltable.h"
#include "mtableview.h"
#include "mvaluelabeltable.h"
#include "mwritereadtable.h"
#include <QLayout>
#include "pagewidgetscollect.h"

#define PAGEPARSE_DBG(x...) qDebug(x)

PageParse::PageParse(int src_addr, QString dir, QWidget *parent) : QWidget(parent), ui(new Ui::PageParse), src_addr(src_addr)
{
    ui->setupUi(this);
    data = new DataObjMap(src_addr);
    connect(this, &PageParse::sig_request_dst_read_reg, J1939DbIns, &CommJ1939Db::slot_request_dst_read_reg);

    // QString       dir      = "./cfg/page_json";
    QFileInfoList fileList = GetFileList(dir); //获取目录下所有的文件
    for (QFileInfo info : fileList)
    {
        PAGEPARSE_DBG() << "file path:" << info.filePath() << "   file name:" << info.fileName();
        if (info.filePath().contains(".json")) // 只处理JSON文件
        {
            load_json(info.fileName().replace(".json", ""), info.filePath());
        }
    }
}

PageParse::~PageParse()
{
    delete ui;
    delete data;
}

void PageParse::set_src_addr(int addr)
{
    src_addr = addr;
}

bool PageParse::load_json(QString name, QString path)
{
    QJsonDocument jsonCfgDoc;
    QFile         file_read(path);
    QFileInfo     fileInfo = QFileInfo(path);

    if (!file_read.open(QIODevice::ReadOnly))
    {
        PAGEPARSE_DBG("read json file failed");
        return false;
    }
    QByteArray read_array = file_read.readAll();
    file_read.close();

    QJsonParseError e;
    jsonCfgDoc = QJsonDocument::fromJson(read_array, &e);

    if (e.error != QJsonParseError::NoError && !jsonCfgDoc.isNull())
    {
        PAGEPARSE_DBG("read json file error %d", e.error);
        return false;
    }
    // json_items_handle(name, &jsonCfgDoc);
    PageWidgetsCollect *page = new PageWidgetsCollect(data);
    connect(page, &PageWidgetsCollect::sig_request_read_reg, this, &PageParse::slot_request_read_reg);
    page->json_items_handle(&jsonCfgDoc);
    ui->tabWidget->addTab(page, name);
    return true;
}

void PageParse::slot_request_read_reg(uint16_t reg_addr, uint16_t reg_len)
{
    emit sig_request_dst_read_reg(src_addr, reg_addr, reg_len);
}
