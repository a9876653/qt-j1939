#ifndef UDPCAN_H
#define UDPCAN_H

#include <QObject>
#include "canbase.h"
#include <QtNetwork>

#define CAN_FRAME_NUM_MAX  64
#define CAN_CHANNEL_OPEN   1
#define CAN_CHANNEL_CLOSE  2
#define CAN_CONFIG_PORT    50000
#define CAN_SEND_DATA_PORT 50001
#define CAN_RECV_DATA_PORT 50002
#define CAN_ACK_PORT       50003

typedef struct
{
    uint8_t  device_index;
    uint8_t  channel_index;
    uint32_t baudrate;
    uint8_t  ch_ctrl;
} udp_can_config_t;

typedef struct
{
    uint8_t  device_index;
    uint8_t  channel_index;
    uint16_t port;
    int      ack;
} udp_can_ack_t;

typedef struct
{
    uint8_t              device_index;
    uint8_t              channel_index;
    uint16_t             frame_num;
    CanBase::can_frame_t data[CAN_FRAME_NUM_MAX];
} udp_can_data_t;

class UdpCan : public CanBase
{
    Q_OBJECT
public:
    UdpCan();
    ~UdpCan();

private:
    bool is_open();

private slots:
    void slot_open_device(uint8_t device_index, uint8_t ch_index, uint32_t baudrate);
    void slot_close_device();

    void slot_ready_read_data();
    void slot_ready_read_ack();

private:
    int  device_index  = 0;
    int  channel_index = 0;
    bool started       = false;

    QUdpSocket *udp_recv_data_socket = nullptr;
    QUdpSocket *udp_recv_ack_socket  = nullptr;
    QUdpSocket *udp_send_socket      = nullptr;

    uint32_t u32_baudrate = 500000;

protected:
    void transmit_task();
    void receive_task();
};

#endif // UDPCAN_H
