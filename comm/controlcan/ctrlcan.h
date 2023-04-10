#pragma once

#include <QObject>
#include "ControlCAN.h"
#include "canbase.h"

class CtrlCan : public CanBase
{
#define INVALID_DEVICE_HANDLE 0
    Q_OBJECT
public:
    explicit CtrlCan();
    ~CtrlCan();

private:
    bool is_open();

private slots:
    void slot_open_device(uint8_t device_index, uint32_t baudrate);
    void slot_close_device();

private:
    int  device_index  = 0;
    int  channel_index = 0;
    bool started       = false;
    int  dev_type      = VCI_USBCAN2;

    QThread *thread = nullptr;

    QTimer *recv_timer = nullptr;
    QTimer *send_timer = nullptr;

    uint32_t u32_baudrate = 500000;

    VCI_CAN_OBJ recv_data[CAN_RECV_DATA_SIZE];

    bool set_baudrate(uint16_t channel_index, uint baudrate);

protected:
    void transmit_task();
    void receive_task();
};
