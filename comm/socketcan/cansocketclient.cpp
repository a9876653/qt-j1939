#include "cansocketclient.h"
#include <QDebug>
#include <QProcess>

CanSocketClient::CanSocketClient()
{
    //    QProcess process(this);
    //    process.startDetached("qservertest.exe");
    connect(this, &CanSocketClient::sig_open_device, this, slot_open_device, Qt::QueuedConnection);
    connect(this, &CanSocketClient::sig_close_device, this, slot_close_device, Qt::QueuedConnection);
}

bool CanSocketClient::is_open()
{
    return started;
}

bool CanSocketClient::socket_is_ok(QLocalSocket *socket)
{
    if (socket == nullptr)
    {
        return false;
    }
    if (!socket->isValid())
    {
        return false;
    }
    if (!socket->isOpen())
    {
        return false;
    }
    return true;
}

void CanSocketClient::socket_close(QLocalSocket *socket)
{
    if (socket)
    {
        socket->disconnectFromServer();
        socket->close();
    }
}

void CanSocketClient::wait_timer_start()
{
    if (wait_timer == nullptr)
    {
        wait_timer = new QTimer();
        connect(wait_timer, &QTimer::timeout, this, &CanSocketClient::slot_wait_timeout);
    }
    wait_timer->setInterval(client_wait_timeout);
    wait_timer->start();
}
void CanSocketClient::wait_timer_stop()
{
    if (wait_timer)
    {
        wait_timer->stop();
    }
}

void CanSocketClient::slot_wait_timeout()
{
    request_open = true;
}

void CanSocketClient::slot_ready_read_data()
{
    QLocalSocket *s = qobject_cast<QLocalSocket *>(sender());
    if (!s)
    {
        return;
    }
    // while (s->readChannelCount())
    {
        QByteArray        r          = s->readAll();
        socket_can_t     *socket_can = (socket_can_t *)r.data();
        uint8_t           cmd        = socket_can->cmd;
        socket_can_ack_t *can_ack    = &socket_can->ack;
        int               num        = socket_can->data.frame_num;
        wait_timer_stop();
        switch (cmd)
        {
        case SOCKET_CAN_CMD_DATA:

            for (int i = 0; i < num; i++)
            {
                recv_enqueue(socket_can->data.data[i]);
            }
            break;
        case SOCKET_CAN_CMD_ACK:

            if (can_ack->cmd == SOCKET_CAN_CMD_CFG)
            {
                if (can_ack->ack == 1)
                {
                    qDebug() << "设备 " << device_index << "通道 " << channel_index << "打开成功";
                    emit sig_open_finish(STATUS_OK);
                }
                else if (can_ack->ack < 0)
                {
                    request_open = true;
                    qDebug() << "设备 " << device_index << "通道 " << channel_index << "打开失败";
                    emit sig_open_finish(STATUS_ERR);
                }
                else if (can_ack->ack == 0)
                {
                }
            }
            else if (can_ack->cmd == SOCKET_CAN_CMD_DATA)
            {
                if (can_ack->ack < 0)
                {
                    qDebug() << "设备 " << device_index << "通道 " << channel_index << "发送失败";
                    request_open = true;
                }
            }
            break;
        };
    }
}

bool CanSocketClient::connect_server(uint8_t device_index, uint8_t ch_index)
{
    if (client == nullptr)
    {
        client = new QLocalSocket;
        connect(client, &QLocalSocket::readyRead, this, &CanSocketClient::slot_ready_read_data);
    }
    else
    {
        delete client;
        client = new QLocalSocket;
        connect(client, &QLocalSocket::readyRead, this, &CanSocketClient::slot_ready_read_data);
    }

    QString name = QString("candev%1ch%2").arg(device_index).arg(ch_index);
    client->connectToServer(name);
    if (client->waitForConnected(client_wait_timeout))
    {
        qDebug("Can设备%d通道%d连接成功", device_index, channel_index);
        return true;
    }
    else
    {
        //        CtrlCanDev *dev = nullptr;
        //        if (!can_dev_map.contains(device_index))
        //        {
        //            qDebug("找不到Can设备%d通道%d，重新创建", device_index, channel_index);
        //            dev = new CtrlCanDev(device_index);
        //            can_dev_map.insert(device_index, dev);
        //            QThread::msleep(1000);
        //        }
        socket_close(client);
        client->connectToServer(name);
        if (client->waitForConnected(client_wait_timeout))
        {
            qDebug("创建Can设备%d通道%d并连接成功", device_index, channel_index);
            return true;
        }
    }

    return false;
}

void CanSocketClient::disconnect_server()
{
    if (client != nullptr)
    {
        client->disconnectFromServer();
        client->close();
    }
}

bool CanSocketClient::request_candev_ctrl(QLocalSocket *socket, uint8_t ctrl, uint32_t baudrate)
{
    if (!socket_is_ok(socket))
    {
        socket_close(client);
        return false;
    }
    QByteArray    array(16, 0);
    socket_can_t *socket_can  = (socket_can_t *)array.data();
    socket_can->cmd           = SOCKET_CAN_CMD_CFG;
    socket_can->device_index  = device_index;
    socket_can->channel_index = channel_index;
    socket_can->cfg.ch_ctrl   = ctrl;
    socket_can->cfg.baudrate  = baudrate;
    if (socket->write(array) <= 0)
    {
        socket->close();
    }
    else
    {
        wait_timer_start();
    }
    return true;
}

void CanSocketClient::open_device()
{
    if (!connect_server(device_index, channel_index))
    {
        qDebug("Can设备%d通道%d连接失败", device_index, channel_index);
        disconnect_server();
        return;
    }

    if (!request_candev_ctrl(client, SOCKET_CAN_CHANNEL_OPEN, u32_baudrate))
    {
        qDebug("Can设备%d通道%d打开失败", device_index, channel_index);
        disconnect_server();
        return;
    }
}

void CanSocketClient::slot_open_device(uint8_t dev_index, uint8_t ch_index, uint32_t baudrate)
{
    started       = true;
    device_index  = dev_index;
    channel_index = ch_index;
    u32_baudrate  = baudrate;
    request_open  = true;
    wait_timer_start();
}

void CanSocketClient::slot_close_device()
{
    started       = false;
    request_close = true;
    request_candev_ctrl(client, SOCKET_CAN_CHANNEL_CLOSE, u32_baudrate);
    wait_timer_stop();
    emit sig_open_finish(STATUS_OFFLINE);
}

int CanSocketClient::request_candev_send(QLocalSocket *socket, socket_can_data_t data)
{
    if (!socket_is_ok(socket) || data.frame_num > SOCKET_CAN_FRAME_NUM_MAX)
    {
        socket_close(client);
        return 0;
    }
    int           data_len = data.frame_num * sizeof(can_frame_t);
    QByteArray    array(data_len + 8, 0);
    socket_can_t *socket_can  = (socket_can_t *)array.data();
    socket_can->cmd           = SOCKET_CAN_CMD_DATA;
    socket_can->device_index  = device_index;
    socket_can->channel_index = channel_index;
    memcpy(&socket_can->data, &data, data_len);
    if (socket->write(array) <= 0)
    {
        socket->close();
    }
    else
    {
        wait_timer_start();
    }
    return data.frame_num;
}

void CanSocketClient::transmit_task()
{
    if (!is_open())
    {
        return;
    }
    socket_can_data_t can_data;
    can_data.frame_num = 0;
    int frame_num      = 0;
    while (frame_num < SOCKET_CAN_FRAME_NUM_MAX && transmit_dequeue(can_data.data[frame_num]))
    {
        frame_num++;
    }
    if (frame_num > 0)
    {
        can_data.frame_num = frame_num;
        request_candev_send(client, can_data);
    }
}

void CanSocketClient::receive_task()
{
    if (!is_open())
    {
        return;
    }
    if (request_open)
    {
        request_open = false;
        open_device();
    }
    if (request_close)
    {
    }
}
