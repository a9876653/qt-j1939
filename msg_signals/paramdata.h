#ifndef PARAMDATA_H
#define PARAMDATA_H

#include "QObject"
#include "CustomTextWidget.h"
#include "comm_j1939.h"
#include "frmcentericon.h"
#include "widgetnumlineedit.h"

class ParamData : public QObject
{
    Q_OBJECT
public:
    ParamData(int id, QString name, QString type, float def_v = 0);

    int               id;
    int8_t            reg_len;
    QString           name;
    QString           type;
    WidgetNumLineEdit send_widget;
    CustomTextWidget  recv_widget;
    frmCenterIcon    *icon_widget;
    int               write_value = 0;

signals:
    void sig_request_read_reg(uint16_t reg_addr, uint16_t reg_len);
    void sig_request_write_reg(uint16_t reg_addr, QVector<uint16_t> array);

public slots:
    void slot_set_icon(bool ok);
    void slot_request_write();
    void slot_request_read();
    void slot_write_finish();
    void slot_read_finish(uint32_t value);
};

#endif // PARAMDATA_H
