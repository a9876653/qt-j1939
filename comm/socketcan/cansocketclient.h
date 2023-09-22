#ifndef CANSOCKETCLIENT_H
#define CANSOCKETCLIENT_H

#include <QObject>
#include "cansocketserver.h"
#include "mthread.h"
#include "cansocketdef.h"

class CanSocketClient : public CanBase
{
    Q_OBJECT
public:
    explicit CanSocketClient();

signals:

private:
    bool is_open();
    bool connect_server(uint8_t device_index, uint8_t ch_index);
    void disconnect_server();

    bool request_candev_ctrl(QLocalSocket *socket, uint8_t ctrl, uint32_t baudrate);
    int  request_candev_send(QLocalSocket *socket, socket_can_data_t data);
    void open_device();

    void wait_timer_start();
    void wait_timer_stop();
private slots:

    void slot_open_device(uint8_t device_index, uint8_t ch_index, uint32_t baudrate);
    void slot_close_device();

    void slot_ready_read_data();

    void slot_wait_timeout();

private:
    int      device_index  = 0;
    int      channel_index = 0;
    bool     started       = false;
    uint32_t u32_baudrate  = 500000;

    bool request_open  = false;
    bool request_close = false;

    const int client_wait_timeout = 1000;

    QTimer *wait_timer = nullptr;

    QLocalSocket *client = nullptr;

protected:
    void transmit_task();
    void receive_task();
};

#endif // CANSOCKETCLIENT_H
