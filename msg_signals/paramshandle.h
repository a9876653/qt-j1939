#ifndef PARAMSHANDLE_H
#define PARAMSHANDLE_H

#include "QObject"
#include "paramdata.h"

class ParamsHandle : public QObject
{
    Q_OBJECT
public:
    ParamsHandle(int src_addr);
    QString path;

    bool load_json(QString path);

    void auto_get_param_table(bool en);
    void set_param_table();
    void get_param_table();

    QMap<int, ParamData *> param_map;

private:
    int src_addr = 0;

    bool is_auto_get = false;

    int      request_get_index = 0;
    int      request_set_index = 0;
    uint16_t request_reg_addr;

    QTimer request_get_timer;
    QTimer request_set_timer;

    void json_items_handle(QJsonDocument *jdoc);

    void request_get_regs();

private slots:
    void slot_request_get_timeout();
    void slot_request_set_timeout();
    void slot_respond_read_reg(uint16_t reg_addr, QByteArray array);
};

#endif // PARAMSHANDLE_H
