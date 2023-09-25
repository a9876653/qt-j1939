#include "ctrlcanchannel.h"
#include <QVector>
#include <QDebug>

#define CANCTRL_DBG(x...) qDebug(x)

CtrlCanChannel::CtrlCanChannel(uint8_t dev_index, uint8_t ch_index)
{
    device_index  = dev_index;
    channel_index = ch_index;
    memset(&errinfo, 0, sizeof(errinfo));
    connect(this, &CtrlCanChannel::sig_open_device, this, &CtrlCanChannel::slot_open_device);
    connect(this, &CtrlCanChannel::sig_close_device, this, &CtrlCanChannel::slot_close_device);
}

CtrlCanChannel::~CtrlCanChannel()
{
    slot_close_device();
}

bool CtrlCanChannel::is_open()
{
    return started;
}

void CtrlCanChannel::slot_open_device(uint8_t dev_index, uint8_t ch_index, uint32_t baudrate)
{
    device_index  = dev_index;
    channel_index = ch_index;

    baudrate_map_t baudrate_temp = BAUDRATE_INIT(500000, 0x00, 0x1C);
    vci_config.AccCode           = 0;
    vci_config.AccMask           = 0xFFFFFFFF;

    for (int i = 0; i < baudrate_map.count(); i++)
    {
        baudrate_temp = baudrate_map[i];
        if (baudrate == baudrate_temp.baudrate)
        {
            break;
        }
    }
    vci_config.Timing0 = baudrate_temp.timing0;
    vci_config.Timing1 = baudrate_temp.timing1;

    vci_config.Filter = 1;
    vci_config.Mode   = 0;

    if (VCI_InitCAN(dev_type, device_index, channel_index, &vci_config) != STATUS_OK)
    {
        CANCTRL_DBG("设备%d通道%d初始化CAN失败!", device_index, channel_index);
        slot_close_device();
        return;
    }

    if (VCI_StartCAN(dev_type, device_index, channel_index) != STATUS_OK)
    {
        CANCTRL_DBG("设备%d通道%d启动CAN失败!", device_index, channel_index);
        slot_close_device();
        return;
    }

    CANCTRL_DBG("设备%d通道%d初始化成功波特率:%d", device_index, channel_index, baudrate);

    started = true;
    emit sig_open_finish(STATUS_OK);
}

void CtrlCanChannel::slot_close_device()
{
    started = false;
    emit sig_open_finish(STATUS_OFFLINE);
}

void CtrlCanChannel::slot_restart_device()
{
    if (errinfo.ErrCode & ERR_CMDFAILED)
    {
        if (VCI_InitCAN(dev_type, device_index, channel_index, &vci_config) != STATUS_OK)
        {
            CANCTRL_DBG("设备%d通道%d重启,初始化CAN失败!", device_index, channel_index);
            return;
        }
    }

    if (errinfo.ErrCode & ERR_CAN_BUSOFF || errinfo.ErrCode & ERR_CAN_BUSERR)
    {
        CANCTRL_DBG("设备%d通道%d重启", device_index, channel_index);
        if (VCI_ResetCAN(dev_type, device_index, channel_index) != STATUS_OK)
        {
            CANCTRL_DBG("设备%d通道%d重启, 复位CAN失败!", device_index, channel_index);
            return;
        }
    }

    if (VCI_StartCAN(dev_type, device_index, channel_index) != STATUS_OK)
    {
        CANCTRL_DBG("设备%d通道%d重启,启动CAN失败!", device_index, channel_index);
        return;
    }
    CANCTRL_DBG("设备%d通道%d重启成功", device_index, channel_index);
}

void CtrlCanChannel::transmit_task()
{
    if (!is_open())
    {
        return;
    }

    can_frame_t tx_data;
    int         frame_num = 0;
    while (frame_num < CAN_SEND_DATA_SIZE && transmit_dequeue(tx_data))
    {
        VCI_CAN_OBJ *obj = &send_data[frame_num];
        memset(obj, 0, sizeof(VCI_CAN_OBJ));
        uint8_t is_ext_frame = tx_data.flag & MSG_FLAG_EXT ? 1 : 0;
        obj->ExternFlag      = is_ext_frame;
        obj->ID              = tx_data.id;
        obj->DataLen         = tx_data.len;
        memcpy(obj->Data, tx_data.data, tx_data.len);
        frame_num++;
    }

    int ret = VCI_Transmit(dev_type, device_index, channel_index, send_data, frame_num);
    if (ret < frame_num)
    {
        VCI_ReadErrInfo(dev_type, device_index, channel_index, &errinfo); //读取错误信息

        CANCTRL_DBG("设备%d通道%d 发送失败 ret %d, num %d!, errcode 0x%08x",
                    device_index,
                    channel_index,
                    ret,
                    frame_num,
                    errinfo.ErrCode);
        if (errinfo.ErrCode > 0)
        {
            slot_restart_device();
        }
    }
}

void CtrlCanChannel::receive_task()
{
    int len;

    if (!is_open())
    {
        return;
    }
    len = VCI_GetReceiveNum(dev_type, device_index, channel_index); //查看缓冲区有多少帧数据
    if (len > 0)
    {
        len = VCI_Receive(dev_type, device_index, channel_index, recv_data, CAN_RECV_DATA_SIZE, 0); //接收数据
        if (len < 0)                                                                                //读取错误
        {
            VCI_ReadErrInfo(dev_type, device_index, channel_index, &errinfo); //读取错误信息
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                uint8_t dlc = recv_data[i].DataLen;
                if (dlc > 0 && dlc <= 8)
                {
                    can_frame_t frame;

                    frame.id   = recv_data[i].ID;
                    frame.flag = recv_data[i].ExternFlag ? MSG_FLAG_EXT : 0;
                    frame.len  = dlc;
                    memcpy(frame.data, recv_data[i].Data, dlc);
                    recv_enqueue(frame);
                }
            }
        }
    }
}
