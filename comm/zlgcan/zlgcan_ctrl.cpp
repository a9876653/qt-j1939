#include "zlgcan_ctrl.h"
#include "string.h"
#include "stdio.h"
#include "zlgcan.h"
#include "QDebug"
#include "utility"
#include "QString"
#include <QThread>

#define ZLGCAN_DBG(x...) qDebug(x)

ZlgCan::ZlgCan()
{
    ZLGCAN_DBG("Main Thread id %d", (int)QThread::currentThreadId());
    connect(this, &ZlgCan::sig_open_device, this, slot_open_device);
    connect(this, &ZlgCan::sig_close_device, this, slot_close_device);
}

ZlgCan::~ZlgCan()
{
    slot_close_device();
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

void ZlgCan::slot_open_device(uint8_t dev_index, uint8_t ch_index, uint32_t baudrate)
{
    slot_close_device();

    int ret       = STATUS_OK;
    device_index  = dev_index;
    channel_index = ch_index;
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

void ZlgCan::slot_close_device()
{
    ZCAN_CloseDevice(device_handle);
    emit sig_open_finish(STATUS_OFFLINE);
}

void ZlgCan::restart_device()
{
    ZCAN_CloseDevice(device_handle);
    device_handle = ZCAN_OpenDevice(ZCAN_USBCAN2, 0, 0);
    if (device_handle == INVALID_DEVICE_HANDLE)
    {
        ZLGCAN_DBG("CAN RESET OPEN DEVICE FAILED!");
        return;
    }

    set_baudrate(channel_index, u32_baudrate);

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
        ZLGCAN_DBG("CAN RESET OPEN CAHNNEL FAILED!");
        return;
    }

    if ((ZCAN_StartCAN(channel_handle)) != STATUS_OK)
    {
        ZLGCAN_DBG("CAN RESET START FAILED!");
        return;
    }
    ZLGCAN_DBG("CAN RESET INIT SUCCESSFUL");
}

bool ZlgCan::is_open()
{
    if (device_handle == INVALID_DEVICE_HANDLE)
    {
        return false;
    }

    if (channel_handle == INVALID_DEVICE_HANDLE)
    {
        return false;
    }

    return true;
}

void ZlgCan::transmit_task()
{
    can_frame_t tx_data;
    uint        send_cnt = 0;
    while (transmit_dequeue(tx_data))
    {
        if (tx_data.len > 0)
        {
            ZCAN_Transmit_Data *p = &send_data[send_cnt];
            memset(p, 0, sizeof(ZCAN_Transmit_Data));
            p->transmit_type     = 0;
            uint8_t is_ext_frame = tx_data.flag & MSG_FLAG_EXT ? 1 : 0;
            p->frame.can_id      = MAKE_CAN_ID(tx_data.id, is_ext_frame, 0, 0);
            p->frame.can_dlc     = tx_data.len;
            memcpy(p->frame.data, tx_data.data, tx_data.len);
            send_cnt++;
        }
    }
    uint ret = ZCAN_Transmit(channel_handle, send_data, send_cnt);
    if (ret < send_cnt)
    {
        ZLGCAN_DBG("CAN TRANSMIT FAILED ret %d, len %d!", ret, send_cnt);

        restart_device();
    }
}

void ZlgCan::receive_task()
{
    int len;
    len = ZCAN_GetReceiveNum(channel_handle, TYPE_CAN);
    if (len > 0)
    {
        len = ZCAN_Receive(channel_handle, recv_data, CAN_RECV_DATA_SIZE, 0);
        for (int i = 0; i < len; i++)
        {
            uint8_t dlc = recv_data[i].frame.can_dlc;
            if (dlc > 0 && dlc <= 8)
            {
                can_frame_t frame;
                uint32_t    id = recv_data[i].frame.can_id;
                frame.id       = id;
                frame.flag     = IS_EFF(id) ? MSG_FLAG_EXT : 0;
                frame.id       = GET_ID(id);
                frame.len      = dlc;
                memcpy(frame.data, recv_data[i].frame.data, dlc);
                recv_queue.enqueue(frame);
            }
        }
    }
}
