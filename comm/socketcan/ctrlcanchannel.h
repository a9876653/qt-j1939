#ifndef CTRLCANCHANNEL_H
#define CTRLCANCHANNEL_H

#include <QObject>
#include "ControlCAN.h"
#include "canbase.h"

class CtrlCanChannel : public CanBase
{
    Q_OBJECT
public:
    CtrlCanChannel(uint8_t device_index, uint8_t ch_index);
    ~CtrlCanChannel();
    bool is_open();

private slots:
    void slot_open_device(uint8_t device_index, uint8_t ch_index, uint32_t baudrate);
    void slot_close_device();
    void slot_restart_device();

public:
    int device_index  = 0;
    int channel_index = 0;

private:
    int  dev_type = VCI_USBCAN2;
    bool started  = false;

    uint32_t u32_baudrate = 500000;

    VCI_CAN_OBJ     recv_data[CAN_RECV_DATA_SIZE];
    VCI_CAN_OBJ     send_data[CAN_SEND_DATA_SIZE];
    VCI_ERR_INFO    errinfo;    //错误结构体
    VCI_INIT_CONFIG vci_config; // 配置

protected:
    void transmit_task();
    void receive_task();

private:
    typedef struct
    {
        uint32_t baudrate;
        uint32_t timing0;
        uint32_t timing1;
    } baudrate_map_t;

#define BAUDRATE_INIT(b, t0, t1) \
    {                            \
        b, t0, t1                \
    }

    const QVector<baudrate_map_t> baudrate_map = {
        BAUDRATE_INIT(125000, 0x03, 0x1C),
        BAUDRATE_INIT(250000, 0x01, 0x1C),
        BAUDRATE_INIT(500000, 0x00, 0x1C),
        BAUDRATE_INIT(1000000, 0x00, 0x14),
    };
};

#endif // CTRLCANCHANNEL_H
