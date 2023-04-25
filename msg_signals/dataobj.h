#ifndef DATAOBJ_H
#define DATAOBJ_H

#include <QObject>
#include <QVariant>
#include "value_des.h"

typedef uint32_t obj_value_t;

class DataObj : public QObject
{
    Q_OBJECT

public:
    DataObj(QString     name,
            uint32_t    id,
            QString     type,
            obj_value_t min       = 0,
            obj_value_t max       = 0xFFFFFFFF,
            obj_value_t def       = 0,
            ValueDes    value_des = ValueDes());

signals:
    void sig_write_finish();
    void sig_update(QVariant value);
    void sig_request_read_reg(uint16_t reg_addr, uint16_t reg_len);
    void sig_request_write_reg(uint16_t reg_addr, uint16_t *data, uint16_t reg_len);

public slots:
    void slot_write_finish();
    void slot_read_finish(uint32_t value);
    void slot_request_read_reg();
    void slot_request_write_reg(QVariant value);

public:
    QString     name;
    uint32_t    id;
    QString     type;
    uint16_t    reg_len     = 1;
    obj_value_t min         = 0x0;
    obj_value_t max         = 0xFFFFFFFF;
    obj_value_t def         = 0;
    int         write_value = 0xFFFFFFFF;
    ValueDes    value_des;
};

#endif // DATAOBJ_H
