#include "dataobj.h"
#include "canard_dsdl.h"

DataObj::DataObj(QString     name,
                 QString     name_en,
                 uint32_t    id,
                 QString     type,
                 obj_value_t min,
                 obj_value_t max,
                 obj_value_t def,
                 ValueDes    value_des)
    : name(name), name_en(name_en), id(id), type(type), min(min), max(max), def(def), value_des(value_des)
{
    if (type.contains("32"))
    {
        reg_len = 2;
    }
    else
    {
        reg_len = 1;
    }
    read_value  = def;
    write_value = def;
}

void DataObj::slot_request_read_reg()
{
    emit sig_request_read_reg(id, reg_len);
}

void DataObj::slot_request_write_reg(QVariant value)
{
    QVector<uint16_t> array(reg_len);
    if (type == "int16_t")
    {
        int16_t v   = value.toInt();
        write_value = v;
        memcpy(&array[0], (uint16_t *)&v, 2 * reg_len);
    }
    else if (type == "uint32_t")
    {
        uint32_t v  = value.toUInt();
        write_value = v;
        memcpy(&array[0], (uint16_t *)&v, 2 * reg_len);
    }
    else if (type == "int32_t")
    {
        int32_t v   = value.toInt();
        write_value = v;
        memcpy(&array[0], (uint16_t *)&v, 2 * reg_len);
    }
    else if (type == "int32_t")
    {
        int32_t v = value.toUInt();
        memcpy(&array[0], (uint16_t *)&v, 2 * reg_len);
    }
    else
    {
        uint16_t v  = value.toUInt();
        write_value = v;
        memcpy(&array[0], (uint16_t *)&v, 2 * reg_len);
    }
    emit sig_request_write_reg(id, array);
}

void DataObj::slot_write_finish()
{
    emit sig_write_finish();
}
void DataObj::slot_read_finish(uint32_t v)
{
    QVariant value;
    if (type == "int16_t")
    {
        value = (int16_t)v;
    }
    else if (type == "uint32_t")
    {
        value = (uint32_t)v;
    }
    else if (type == "int32_t")
    {
        value = (int32_t)v;
    }
    else
    {
        value = (uint16_t)v;
    }
    read_value = value;
    emit sig_update(value);
}
