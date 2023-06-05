#ifndef __ZLGCAN_CTRL_H
#define __ZLGCAN_CTRL_H
#include "QTimer"
#include "QThread"
#include "blocking_queue.h"
#include "QObject"
#include "zlgcan.h"
#include "stdint.h"
#include "canbase.h"

class ZlgCan : public CanBase
{
    Q_OBJECT
public:
    explicit ZlgCan();
    ~ZlgCan();

private:
    bool is_open();

private slots:
    void slot_open_device(uint8_t device_index, uint32_t baudrate);
    void slot_close_device();

private:
    DEVICE_HANDLE  device_handle  = INVALID_DEVICE_HANDLE;
    CHANNEL_HANDLE channel_handle = INVALID_DEVICE_HANDLE;

    QThread *thread = nullptr;

    QTimer *recv_timer = nullptr;
    QTimer *send_timer = nullptr;

    uint8_t  device_index = 0;
    uint32_t u32_baudrate = 500000;

    ZCAN_Transmit_Data send_data[CAN_SEND_DATA_SIZE];
    ZCAN_Receive_Data  recv_data[CAN_RECV_DATA_SIZE];

    bool set_baudrate(uint16_t channel_index, uint baudrate);

protected:
    void transmit_task();
    void receive_task();
};

#endif
