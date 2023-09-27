#ifndef CTRLCANDEV_H
#define CTRLCANDEV_H

#include <QObject>
#include <QDebug>
#include "ControlCAN.h"
#include "ctrlcanchannel.h"
#include "mthreadperiodtask.h"
#include "cansocketserver.h"

class CtrlCanDev : public QObject
{
    Q_OBJECT
public:
    CtrlCanDev(int dev_index)
    {
        device_index = dev_index;
        comm_thread  = new MThread(std::bind(&CtrlCanDev::can_dev_task, this), std::bind(&CtrlCanDev::init, this));
        this->moveToThread(comm_thread);
        comm_thread->start();
    }
    ~CtrlCanDev()
    {
        VCI_CloseDevice(dev_type, device_index);
    }

    void init()
    {
        CtrlCanChannel *ch = nullptr;
        for (int k = 0; k < 4; k++)
        {
            QString name = QString("/candev%1ch%2").arg(device_index).arg(k);
            ch           = new CtrlCanChannel(device_index, k);
            can_channel_map.insert(k, ch);
            CanSocketServer *can = new CanSocketServer(name, ch);
            cansocket_dev_map.insert(name, can);
        }
    }

    bool open_device(uint8_t ch_index, uint32_t baudrate)
    {
        CtrlCanChannel *ch = nullptr;
        if (can_channel_map.contains(ch_index))
        {
            ch = can_channel_map[ch_index];
        }
        else
        {
            qDebug("打开设备%d通道%d不存在", device_index, ch_index);
            return false;
        }
        if (!is_had_open())
        {
            // VCI_CloseDevice(dev_type, device_index);
            if (VCI_OpenDevice(dev_type, device_index, 0) != STATUS_OK)
            {
                qDebug() << "打开设备失败，请检查设备类型和设备索引号是否正确";
                return false;
            }
        }
        is_open_ok = true;
        return ch->open_device(device_index, ch_index, baudrate);
    }

    void close_device(uint8_t ch_index = 0xFF)
    {
        if (can_channel_map.contains(ch_index))
        {
            CtrlCanChannel *ch = can_channel_map[ch_index];
            return ch->close_device();
        }
        if (is_open_ok && !is_had_open())
        {
            VCI_CloseDevice(dev_type, device_index);
            is_open_ok = false;
        }
    }

    void check_close_device()
    {
        if (is_open_ok && (!is_had_open() || is_had_err()))
        {
            VCI_CloseDevice(dev_type, device_index);
            is_open_ok = false;
        }
        if (!is_open_ok && is_had_open() && is_had_err())
        {
            is_open_ok = VCI_OpenDevice(dev_type, device_index, 0) == STATUS_OK;
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
    bool is_had_err()
    {
        foreach (CtrlCanChannel *ch, can_channel_map)
        {
            if (ch->is_err())
            {
                return true;
            }
        }
        return false;
    }

    void can_dev_task()
    {
        foreach (CtrlCanChannel *ch, can_channel_map)
        {
            ch->can_task();
        }
        check_close_device();
        QThread::msleep(3);
    }

public:
    QMap<int, CtrlCanChannel *> can_channel_map;

private:
    int device_index = 0;
    int dev_type     = VCI_USBCAN2;

    bool is_open_ok = false;

    QMap<QString, CanSocketServer *> cansocket_dev_map;

    MThread *comm_thread = nullptr;
};

#endif // CTRLCANDEV_H
