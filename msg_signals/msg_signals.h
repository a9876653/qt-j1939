#ifndef MSG_SIGNALS_H
#define MSG_SIGNALS_H

#include <QObject>
#include <QtCore>
#include "msgdata.h"

class MsgSignals : public QObject
{
    Q_OBJECT
public:
    MsgSignals();
    QMap<uint, MsgData *> msgs_map;

private:
    bool load_json(QString path);
    void json_items_handle(QJsonDocument *jdoc);
};

#endif // MSG_SIGNALS_H
