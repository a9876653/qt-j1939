#ifndef MSGDATA_H
#define MSGDATA_H

#include <QWidget>
#include "CustomTextWidget.h"

class MsgData
{
public:
    uint32_t id;
    uint32_t pgn;
    QString  name;
    class Signal
    {
    public:
        QString  name;
        uint32_t bit_length;
        uint32_t start_bit;
        bool     is_float;
        bool     is_signed;
        float    value_f;
        uint     value_u;
        int      value_i;

        CustomTextWidget send_widget;
        CustomTextWidget rec_dec_widget;
        CustomTextWidget rec_hex_widget;
    };
    QList<Signal *> signals_list;

    void decode(uint8_t *data, uint16_t data_size);
};

#endif // MSGDATA_H
