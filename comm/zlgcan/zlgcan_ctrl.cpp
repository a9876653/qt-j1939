#include "zlgcan_ctrl.h"
#include "string.h"
#include "stdio.h"
#include "zlgcan.h"
#include "QDebug"
#include "utility"
#include "QString"

#define ZLGCAN_DBG(x...) qDebug(x)

const char *baudrate_map[]
    = {"1.0Mbps(75%),(00,14)", "800Kbps(80%),(00,16)", "500Kbps(75%),(01,14)", "250Kbps(75%),(03,14)", "125Kbps(75%),(07,14)"};

ZlgCan::ZlgCan()
{
    receive_timer.stop();
    connect(&receive_timer, &QTimer::timeout, this, &ZlgCan::receive);
}

ZlgCan::~ZlgCan()
{
    receive_timer.stop();
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

bool ZlgCan::open_device(uint8_t channel_index, baudrate_e baudrate)
{
    device_handle = ZCAN_OpenDevice(ZCAN_USBCAN1, 0, 0);
    if (device_handle == INVALID_DEVICE_HANDLE)
    {
        ZLGCAN_DBG("CAN OPEN DEVICE FAILED!");
        return false;
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
        return false;
    }

    if (ZCAN_StartCAN(channel_handle) != STATUS_OK)
    {
        ZLGCAN_DBG("CAN START FAILED!");
        return false;
    }
    receive_timer.start(5);
    ZLGCAN_DBG("CAN INIT SUCCESSFUL BAUDRATE:%s", baudrate_map[baudrate]);

    return true;
}

uint ZlgCan::transmit(uint32_t id, uint flag, uint8_t *data, uint16_t len)
{
    if (len > CAN_MAX_DLEN)
    {
        return 0;
    }
    memset(&transmit_data, 0, sizeof(transmit_data));
    transmit_data.transmit_type = 0;
    uint8_t is_ext_frame        = flag & MSG_FLAG_EXT ? 1 : 0;
    transmit_data.frame.can_id  = MAKE_CAN_ID(id, is_ext_frame, 0, 0);
    transmit_data.frame.can_dlc = len;
    memcpy(transmit_data.frame.data, data, len);
    return ZCAN_Transmit(channel_handle, &transmit_data, 1);
}

void ZlgCan::receive()
{
    uint len;
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
}
