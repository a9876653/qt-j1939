#include "zlgcan_ctrl.h"
#include "string.h"
#include "stdio.h"
#include "zlgcan.h"
#include "QDebug"
#include "utility"
#include "QString"
#include <QThread>

#define ZLGCAN_DBG(x...) qDebug(x)

const char *baudrate_map[]
    = {"1.0Mbps(75%),(00,14)", "800Kbps(80%),(00,16)", "500Kbps(75%),(01,14)", "250Kbps(75%),(03,14)", "125Kbps(75%),(07,14)"};

ZlgCan::ZlgCan()
{
    ZLGCAN_DBG("Main Thread id %d", (int)QThread::currentThreadId());
    connect(this, &ZlgCan::sig_open_device, this, slot_open_device);
    connect(this, &ZlgCan::sig_close_device, this, slot_close_device);

    QThread *t = new QThread;
    this->moveToThread(t);

    send_timer = new QTimer;
    connect(send_timer, &QTimer::timeout, this, &ZlgCan::transmit_task);
    send_timer->start(5);

    recv_timer = new QTimer;
    connect(recv_timer, &QTimer::timeout, this, &ZlgCan::receive_task);
    recv_timer->start(5);
    t->start();
}

ZlgCan::~ZlgCan()
{
    transmit_thread_run = false;
    recv_thread_run     = false;

    ZCAN_CloseDevice(device_handle);
}

bool ZlgCan::set_baudrate(uint16_t channel_index, baudrate_e baudrate)
{
    char path[50]  = {0};
    char value[50] = {0};
    if (baudrate >= CAN_BAUDRATE_ERR)
    {
        ZLGCAN_DBG("CAN SET BAUDRATE ERR!");
        return false;
    }
    sprintf_s(path, "%d/baud_rate_custom", channel_index);
    sprintf(value, "%s", baudrate_map[baudrate]);
    if (ZCAN_SetValue(device_handle, path, value) != 1)
    {
        ZLGCAN_DBG("CAN SET BAUDRATE FAILED!");
        return false;
    }
    return true;
}

bool ZlgCan::set_baudrate(uint16_t channel_index, uint baudrate)
{
    char path[50]  = {0};
    char value[50] = {0};

    sprintf_s(path, "%d/baud_rate", channel_index);
    sprintf(value, "%d", baudrate);
    if (ZCAN_SetValue(device_handle, path, value) != 1)
    {
        ZLGCAN_DBG("CAN SET BAUDRATE FAILED!");
        return false;
    }
    return true;
}

bool ZlgCan::open_device(uint8_t channel_index, uint32_t baudrate)
{
    emit sig_open_device(channel_index, baudrate);
    return true;
}

void ZlgCan::slot_open_device(uint8_t channel_index, uint32_t baudrate)
{
    slot_close_device();

    int ret       = STATUS_OK;
    device_index  = channel_index;
    u32_baudrate  = baudrate;
    device_handle = ZCAN_OpenDevice(ZCAN_USBCAN2, 0, 0);
    if (device_handle == INVALID_DEVICE_HANDLE)
    {
        ZLGCAN_DBG("CAN OPEN DEVICE FAILED!");
        emit sig_open_finish(STATUS_ERR);
        return;
    }

    set_baudrate(channel_index, baudrate);

    ZCAN_CHANNEL_INIT_CONFIG config;
    memset(&config, 0, sizeof(config));
    config.can_type     = TYPE_CAN;
    config.can.mode     = 0;
    config.can.filter   = 0;
    config.can.acc_code = 0;
    config.can.acc_mask = 0xFFFFFFFF;
    channel_handle      = ZCAN_InitCAN(device_handle, channel_index, &config);
    if (channel_handle == INVALID_DEVICE_HANDLE)
    {
        ZLGCAN_DBG("CAN OPEN CAHNNEL FAILED!");
        emit sig_open_finish(STATUS_ERR);
        return;
    }

    if ((ret = ZCAN_StartCAN(channel_handle)) != STATUS_OK)
    {
        ZLGCAN_DBG("CAN START FAILED!");
        emit sig_open_finish(ret);
        return;
    }

    ZLGCAN_DBG("CAN INIT SUCCESSFUL BAUDRATE:%d", baudrate);
    emit sig_open_finish(STATUS_OK);
}

void ZlgCan::close_device()
{
    emit sig_close_device();
}

void ZlgCan::slot_close_device()
{
    ZCAN_CloseDevice(device_handle);
}

uint ZlgCan::transmit(uint32_t id, uint flag, uint8_t *data, uint16_t len)
{
    if (transmit_queue.count() > CAN_SEND_DATA_SIZE)
    {
        ZLGCAN_DBG("CAN Transmit Queue is full");
        return 0;
    }
    if (device_handle == INVALID_DEVICE_HANDLE)
    {
        ZLGCAN_DBG("CAN Device is close");
        return 0;
    }
    if (len > CAN_MAX_DLEN || device_handle == INVALID_DEVICE_HANDLE)
    {
        return 0;
    }
    ZCAN_Transmit_Data transmit_data;
    memset(&transmit_data, 0, sizeof(transmit_data));
    transmit_data.transmit_type = 0;
    uint8_t is_ext_frame        = flag & MSG_FLAG_EXT ? 1 : 0;
    transmit_data.frame.can_id  = MAKE_CAN_ID(id, is_ext_frame, 0, 0);
    transmit_data.frame.can_dlc = len;
    memcpy(transmit_data.frame.data, data, len);
    transmit_queue.enqueue(transmit_data);

    return 1;
}

void ZlgCan::transmit_task()
{
    static bool init = false;
    if (!init)
    {
        ZLGCAN_DBG("CAN Transmit Thread id %d", (int)QThread::currentThreadId());

        init = true;
    }

    while (!transmit_queue.isEmpty())
    // while (transmit_thread_run)
    {
        ZCAN_Transmit_Data transmit_data = transmit_queue.dequeue();

        uint ret = ZCAN_Transmit(channel_handle, &transmit_data, 1);
        if (ret < 1)
        {
            ZLGCAN_DBG("CAN TRANSMIT FAILED ret %d, len %d!", ret, transmit_data.frame.can_dlc);

            if (!open_device(device_index, u32_baudrate))
            {
                ZLGCAN_DBG("CAN RESTART FAILED!");
            }
            ZCAN_Transmit(channel_handle, &transmit_data, 1); // 重发一次
        }
    }
}

void ZlgCan::receive_task()
{
    uint        len;
    static bool init = false;
    if (!init)
    {
        ZLGCAN_DBG("CAN Recv Thread id %d", (int)QThread::currentThreadId());

        init = true;
    }

    // while (recv_thread_run)
    {
        len = ZCAN_GetReceiveNum(channel_handle, TYPE_CAN);
        if (len > 0)
        {
            len = ZCAN_Receive(channel_handle, recv_data, CAN_RECV_DATA_SIZE, 0);
            for (uint i = 0; i < len; i++)
            {
                uint32_t id   = recv_data[i].frame.can_id;
                uint8_t  flag = IS_EFF(id) ? MSG_FLAG_EXT : 0;
                id            = GET_ID(id);
                uint16_t len  = recv_data[i].frame.can_dlc;
                uint8_t *data = (uint8_t *)&recv_data[i].frame.data[0];

                emit sig_receive(id, flag, data, len);
            }
        }
        //    QThread::msleep(5);
    }
}
