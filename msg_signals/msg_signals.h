#ifndef MSG_SIGNALS_H
#define MSG_SIGNALS_H

#include <QObject>
#include <QtCore>
#include "msgdata.h"

class MsgSignals : public QObject
{
    Q_OBJECT
public:
    MsgSignals(bool is_master = false);
    ~MsgSignals();
    QMap<uint, MsgData *> msgs_map;
    // QMap<QString, QMap<uint, MsgData *>> file_msg_map;
public slots:
    void slot_msg_send();

private:
    bool load_json(QString path);
    void json_items_handle(QJsonDocument *jdoc);

    bool load_temp_file(QString path);

    void save_msg_send_data(QString path);

    bool is_master;
};

#endif // MSG_SIGNALS_H
