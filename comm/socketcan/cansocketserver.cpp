#include "cansocketserver.h"

CanSocketServer::CanSocketServer(QString name, CanBase *can) : dev_name(name), can_dev(can)
{
    server = new QLocalServer();
    server->listen(name);
    connect(server, &QLocalServer::newConnection, this, &CanSocketServer::slot_new_connect);
    connect(&recv_timer, &QTimer::timeout, this, &CanSocketServer::slot_can_recv_timeout);
}

void CanSocketServer::slot_new_connect()
{
    QLocalSocket *socket = server->nextPendingConnection();
    newsocket_map.append(socket);
    connect(socket, &QLocalSocket::readyRead, this, &CanSocketServer::slot_ready_read);
    connect(socket, &QLocalSocket::disconnected, this, &CanSocketServer::slot_disconnect);
}

void CanSocketServer::slot_disconnect()
{
    QLocalSocket *s = qobject_cast<QLocalSocket *>(sender());
    if (!s)
    {
        return;
    }
    newsocket_map.removeOne(s);
    qDebug() << "disconnected ";
}

void CanSocketServer::respond_ack(QLocalSocket *s, socket_can_t *socket_can)
{
    QByteArray array(8, 0);

    memcpy(array.data(), socket_can, 8);
    array[0] = SOCKET_CAN_CMD_ACK;
    s->write(array);
}

void CanSocketServer::slot_ready_read()
{
    QLocalSocket *s = qobject_cast<QLocalSocket *>(sender());
    if (!s)
    {
        return;
    }
    // while (s->canReadLine())
    {
        QByteArray    r          = s->readAll();
        socket_can_t *socket_can = (socket_can_t *)r.data();
        switch (socket_can->cmd)
        {
        case SOCKET_CAN_CMD_CFG:
            if (socket_can->cfg.ch_ctrl == SOCKET_CAN_CHANNEL_OPEN)
            {
                bool ret = can_dev->open_device(socket_can->device_index, socket_can->channel_index, socket_can->cfg.baudrate);
                socket_can->ack.cmd = socket_can->cmd;
                socket_can->ack.ack = ret ? 1 : -1;
                if (ret)
                {
                    recv_timer.start(5);
                }
                respond_ack(s, socket_can);
            }
            else if (socket_can->cfg.ch_ctrl == SOCKET_CAN_CHANNEL_CLOSE)
            {
                can_dev->close_device();
                socket_can->ack.cmd = socket_can->cmd;
                socket_can->ack.ack = 0;
                recv_timer.stop();
                respond_ack(s, socket_can);
            }
            break;
        case SOCKET_CAN_CMD_DATA:
            int num = socket_can->data.frame_num;
            for (int i = 0; i < num; i++)
            {
                can_dev->transmit(socket_can->data.data[i]);
            }
            socket_can->ack.cmd = socket_can->cmd;
            socket_can->ack.ack = num;
            respond_ack(s, socket_can);
            break;
        }
    }
}

void CanSocketServer::slot_can_recv_timeout()
{
    socket_can_t socket_can;
    socket_can.cmd              = SOCKET_CAN_CMD_DATA;
    socket_can.device_index     = device_index;
    socket_can.channel_index    = channel_index;
    socket_can_data_t *can_data = &socket_can.data;
    can_data->frame_num         = 0;
    for (int i = 0; i < SOCKET_CAN_FRAME_NUM_MAX; i++)
    {
        CanBase::can_frame_t frame;
        if (can_dev->recv_dequeue(frame))
        {
            memcpy(&can_data->data[i], &frame, sizeof(frame));
            can_data->frame_num++;
        }
        else
        {
            break;
        }
    }
    if (can_data->frame_num > 0)
    {
        int w_len = qMin(sizeof(socket_can_t), can_data->frame_num * sizeof(CanBase::can_frame_t) + 8);

        QByteArray array(w_len, 0);
        memcpy(array.data(), &socket_can, w_len);
        foreach (QLocalSocket *s, newsocket_map)
        {
            if (s->isValid() && s->isOpen())
            {
                s->write(array);
            }
        }
    }
}
