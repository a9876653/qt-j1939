#ifndef CTRLCANDEV_H
#define CTRLCANDEV_H

#include <QObject>
#include <QDebug>
#include "ControlCAN.h"
#include "ctrlcanchannel.h"
#include "mthreadperiodtask.h"

class CtrlCanDev : public QObject
{
    Q_OBJECT
public:
    CtrlCanDev(int dev_index)
    {
        device_index = dev_index;
        comm_thread  = new MThreadPeriodTask(std::bind(&CtrlCanDev::can_dev_task, this));
        this->moveToThread(comm_thread);
        comm_thread->start(3);
    }
    ~CtrlCanDev()
    {
        VCI_CloseDevice(dev_type, device_index);
    }

    bool open_device(uint8_t ch_index, uint32_t baudrate)
    {
        if (!is_had_open())
        {
            // VCI_CloseDevice(dev_type, device_index);
            if (VCI_OpenDevice(dev_type, device_index, 0) != STATUS_OK)
            {
                qDebug() << "打开设备失败，请检查设备类型和设备索引号是否正确";
                return false;
            }
        }
        if (can_channel_map.contains(ch_index))
        {
            CtrlCanChannel *ch = can_channel_map[ch_index];
            return ch->open_device(device_index, ch_index, baudrate);
        }
        else
        {
            CtrlCanChannel *ch = new CtrlCanChannel(device_index, ch_index);
            can_channel_map.insert(ch_index, ch);
            return ch->open_device(device_index, ch_index, baudrate);
        }
    }

    void close_device(uint8_t ch_index = 0xFF)
    {
        if (can_channel_map.contains(ch_index))
        {
            CtrlCanChannel *ch = can_channel_map[ch_index];
            return ch->close_device();
        }
        if (is_all_closed())
        {
            VCI_CloseDevice(dev_type, device_index);
        }
    }

    void check_close_device()
    {
        if (is_all_closed())
        {
            VCI_CloseDevice(dev_type, device_index);
        }
    }

    CtrlCanChannel *get_channel_ins(uint8_t ch_index)
    {
        if (can_channel_map.contains(ch_index))
        {
            return can_channel_map[ch_index];
        }
        return nullptr;
    }

private:
    bool is_had_open()
    {
        foreach (CtrlCanChannel *ch, can_channel_map)
        {
            if (ch->is_open())
            {
                return true;
            }
        }
        return false;
    }
    bool is_all_closed()
    {
        foreach (CtrlCanChannel *ch, can_channel_map)
        {
            if (ch->is_open())
            {
                return false;
            }
        }
        return true;
    }

    void can_dev_task()
    {
        foreach (CtrlCanChannel *ch, can_channel_map)
        {
            ch->can_task();
        }
    }

public:
    QMap<int, CtrlCanChannel *> can_channel_map;

private:
    int                device_index = 0;
    int                dev_type     = VCI_USBCAN2;
    MThreadPeriodTask *comm_thread  = nullptr;
};

#endif // CTRLCANDEV_H
