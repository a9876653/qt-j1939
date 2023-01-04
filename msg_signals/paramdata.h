#ifndef PARAMDATA_H
#define PARAMDATA_H

#include "QObject"
#include "CustomTextWidget.h"
#include "comm_j1939.h"

class ParamData : public QObject
{
    Q_OBJECT
public:
    ParamData(int id, QString name, float def_v = 0);

    int              id;
    QString          name;
    CustomTextWidget send_widget;
    CustomTextWidget recv_widget;

    void set_recv_value(int value);
    void request_get_value();

signals:
    void sig_msg_send(uint32_t pgn, uint8_t *data, uint16_t len);

public slots:
    void slot_encode_send();

private:
    uint8_t encode_buff[8];
};

#endif // PARAMDATA_H
