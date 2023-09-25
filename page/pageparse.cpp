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
#include "json_file.h"

#define PAGEPARSE_DBG(x...) qDebug(x)

PageParse::PageParse(int src_addr, QString dir, QWidget *parent) : QWidget(parent), ui(new Ui::PageParse), src_addr(src_addr)
{
    ui->setupUi(this);
    tabWidget = new MTabWidget();
    tabWidget->setTabPosition(QTabWidget::South); // 标题显示在下方

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

    ui->verticalLayout_2->addWidget(tabWidget);

    csv_header.append("时间");
    csv_header.append("时间UTC");
    for (DataObj *obj : save_obj_list)
    {
        csv_header.append(obj->name_en);
    }

    connect(&csv_timer, &QTimer::timeout, this, &PageParse::save_read_data_file);
    csv_timer.start(1000);
}

PageParse::~PageParse()
{
    delete ui;
    delete data;
}

void PageParse::save_read_data_file()
{
    QList<QString> data;
    QDateTime      time;
    QDate          time_data = time.currentDateTime().date();
    QString        time_s    = QString("%1%2%3")
                         .arg(time_data.year())
                         .arg(time_data.month(), 2, 10, QChar('0'))
                         .arg(time_data.day(), 2, 10, QChar('0'));
    csv_path = QString("./save_data/db-%1-%2.csv").arg(time_s).arg(src_addr);
    data.append(get_current_time_ms_qstring());
    data.append(QString("%1").arg(sys_get_ms()));

    for (DataObj *obj : save_obj_list)
    {
        data.append(QString("%1").arg(obj->read_value.toString()));
    }

    CsvAppend(csv_path, csv_header, data);
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
    save_obj_list.append(page->save_obj_list);
    tabWidget->addTab(page, name);
    return true;
}

void PageParse::slot_request_read_reg(uint16_t reg_addr, uint16_t reg_len)
{
    emit sig_request_dst_read_reg(src_addr, reg_addr, reg_len);
}
