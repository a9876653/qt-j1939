#ifndef MSGDATA_H
#define MSGDATA_H

#include <QWidget>
#include "CustomTextWidget.h"
#include "comm_j1939.h"

class Signal
{
public:
    QString          name;
    uint32_t         bit_length;
    uint32_t         start_bit;
    bool             is_float;
    bool             is_signed;
    float            value_f;
    uint             value_u;
    int              value_i;
    QString          value_s;
    CustomTextWidget send_widget;
    CustomTextWidget rec_dec_widget;
    CustomTextWidget rec_hex_widget;
};

class MsgData : public QObject
{
    Q_OBJECT
public:
    uint32_t id;
    uint32_t pgn;
    QString  name;
    uint16_t msg_len;

    QList<Signal *> signals_list;
    uint8_t         encode_buff[J1939_DEF_BUFF_SIZE];

    void decode(QByteArray array);

signals:
    void sig_msg_send(uint32_t pgn, QByteArray array);
    void sig_request_pgn(uint32_t pgn, uint8_t dst, uint16_t len);

public slots:
    void slot_encode_send();
    void slot_request_pgn(uint8_t src);
};

#endif // MSGDATA_H
