#ifndef PAGEPARSE_H
#define PAGEPARSE_H

#include <QWidget>
#include "dataobjmap.h"

namespace Ui
{
class PageParse;
}

class PageParse : public QWidget
{
    Q_OBJECT

public:
    explicit PageParse(int src_addr, QWidget *parent = nullptr);
    ~PageParse();
    bool load_json(QString name, QString path);
    void set_src_addr(int addr);
signals:
    void sig_request_dst_read_reg(uint16_t dst, uint16_t reg_addr, uint16_t reg_len);
private slots:
    void slot_request_read_reg(uint16_t reg_addr, uint16_t reg_len);

private:
    Ui::PageParse *ui;

    int src_addr;

    DataObjMap *data = nullptr;
};

#endif // PAGEPARSE_H
