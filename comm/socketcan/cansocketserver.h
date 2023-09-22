#ifndef CANSOCKETSERVER_H
#define CANSOCKETSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include "mthread.h"
#include "canbase.h"
#include "cansocketdef.h"

class CanSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit CanSocketServer(QString name, CanBase *can);

signals:
private:
    void slot_new_connect();
    void slot_ready_read();
    void respond_ack(QLocalSocket *s, socket_can_t *socket_can);
    void slot_can_recv_timeout();
    void slot_disconnect();

private:
    QString       dev_name;
    CanBase      *can_dev = nullptr;
    QLocalServer *server  = nullptr;

    int device_index  = 0;
    int channel_index = 0;

    QTimer                recv_timer;
    QList<QLocalSocket *> newsocket_map;
};

#endif // CANSOCKET_H
