#include "udpcan.h"

UdpCan::UdpCan()
{
    connect(this, &UdpCan::sig_open_device, this, slot_open_device, Qt::QueuedConnection);
    connect(this, &UdpCan::sig_close_device, this, slot_close_device, Qt::QueuedConnection);
}

UdpCan::~UdpCan()
{
    delete udp_recv_data_socket;
    delete udp_recv_ack_socket;
    delete udp_send_socket;
}

void UdpCan::slot_open_device(uint8_t dev_index, uint8_t ch_index, uint32_t baudrate)
{
    QUdpSocket send_socket;
    if (udp_recv_data_socket == nullptr)
    {
        udp_recv_data_socket = new QUdpSocket();
        udp_recv_data_socket->bind(QHostAddress::Any, CAN_RECV_DATA_PORT);
        connect(udp_recv_data_socket, &QUdpSocket::readyRead, this, &UdpCan::slot_ready_read_data);
    }

    if (udp_recv_ack_socket == nullptr)
    {
        udp_recv_ack_socket = new QUdpSocket();
        udp_recv_ack_socket->bind(QHostAddress::Any, CAN_ACK_PORT);
        connect(udp_recv_ack_socket, &QUdpSocket::readyRead, this, &UdpCan::slot_ready_read_ack);
    }

    if (udp_send_socket == nullptr)
    {
        udp_send_socket = new QUdpSocket();
    }

    udp_can_config_t cfg;
    device_index      = dev_index;
    channel_index     = ch_index;
    u32_baudrate      = baudrate;
    cfg.device_index  = device_index;
    cfg.channel_index = channel_index;
    cfg.baudrate      = u32_baudrate;
    cfg.ch_ctrl       = CAN_CHANNEL_OPEN;
    send_socket.writeDatagram((char *)&cfg, sizeof(udp_can_config_t), QHostAddress("127.0.0.1"), CAN_CONFIG_PORT);
    started = true;
}

void UdpCan::slot_close_device()
{
    QUdpSocket       send_socket;
    udp_can_config_t cfg;
    cfg.device_index  = device_index;
    cfg.channel_index = channel_index;
    cfg.baudrate      = u32_baudrate;
    cfg.ch_ctrl       = CAN_CHANNEL_CLOSE;
    send_socket.writeDatagram((char *)&cfg, sizeof(udp_can_config_t), QHostAddress("127.0.0.1"), CAN_CONFIG_PORT);
    started = false;
}

void UdpCan::slot_ready_read_ack()
{
    while (udp_recv_ack_socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udp_recv_ack_socket->pendingDatagramSize());
        udp_recv_ack_socket->readDatagram(datagram.data(), datagram.size());
        udp_can_ack_t *ack = (udp_can_ack_t *)datagram.data();
        if (ack->device_index == device_index && ack->channel_index == channel_index)
        {
            if (ack->port == CAN_CONFIG_PORT)
            {
                if (ack->ack > 0)
                {
                    emit sig_open_finish(STATUS_OK);
                }
                else
                {
                    emit sig_open_finish(STATUS_OFFLINE);
                }
            }
        }
    }
}

void UdpCan::slot_ready_read_data()
{
    while (udp_recv_data_socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udp_recv_data_socket->pendingDatagramSize());
        udp_recv_data_socket->readDatagram(datagram.data(), datagram.size());
        udp_can_data_t *can_data = (udp_can_data_t *)datagram.data();
        if (can_data->device_index == device_index && can_data->channel_index == channel_index)
        {
            for (int i = 0; i < can_data->frame_num; i++)
            {
                recv_queue.enqueue(can_data->data[i]);
            }
        }
    }
}

bool UdpCan::is_open()
{
    return started;
}

void UdpCan::transmit_task()
{
    if (!is_open())
    {
        return;
    }
    udp_can_data_t can_data;
    can_data.device_index  = device_index;
    can_data.channel_index = channel_index;
    can_data.frame_num     = 0;
    int frame_num          = 0;
    while (frame_num < CAN_SEND_DATA_SIZE && transmit_dequeue(can_data.data[frame_num]))
    {
        frame_num++;
    }
    if (frame_num > 0)
    {
        QUdpSocket send_socket;
        can_data.frame_num = frame_num;
        send_socket.writeDatagram((char *)&can_data,
                                  4 + can_data.frame_num * sizeof(CanBase::can_frame_t),
                                  QHostAddress("127.0.0.1"),
                                  CAN_SEND_DATA_PORT);
    }
}

void UdpCan::receive_task()
{
}
