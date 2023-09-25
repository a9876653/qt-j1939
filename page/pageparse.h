#ifndef PAGEPARSE_H
#define PAGEPARSE_H

#include <QWidget>
#include "dataobjmap.h"
#include "mtabwidget.h"

namespace Ui
{
class PageParse;
}

class PageParse : public QWidget
{
    Q_OBJECT

public:
    explicit PageParse(int src_addr, QString dir, QWidget *parent = nullptr);
    ~PageParse();
    bool load_json(QString name, QString path);
    void set_src_addr(int addr);
signals:
    void sig_request_dst_read_reg(uint16_t dst, uint16_t reg_addr, uint16_t reg_len);
private slots:
    void slot_request_read_reg(uint16_t reg_addr, uint16_t reg_len);
    void save_read_data_file();

private:
    Ui::PageParse *ui;

    int src_addr;

    QTabWidget *tabWidget = nullptr;

    DataObjMap      *data = nullptr;
    QList<DataObj *> save_obj_list;

    QTimer      csv_timer;
    QStringList csv_header;
    QString     csv_path;
};

#endif // PAGEPARSE_H
