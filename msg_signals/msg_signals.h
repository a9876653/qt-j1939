#ifndef MSG_SIGNALS_H
#define MSG_SIGNALS_H

#include <QObject>
#include <QtCore>
#include "msgdata.h"

class MsgSignals
{
public:
    MsgSignals();

    bool load_json(QString path, QString name);
    void json_items_handle(QJsonDocument *jdoc);

    QMap<uint, MsgData> msgs_map;
};

#endif // MSG_SIGNALS_H
