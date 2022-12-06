#ifndef __ZLGCAN_CTRL_H
#define __ZLGCAN_CTRL_H
#include "QTimer"
#include "QObject"
#include "zlgcan.h"
#include "stdint.h"

/** CAN  **/
#define BIT(x)        (1 << (x))
#define MSG_FLAG_STD  (0)
#define MSG_FLAG_EXT  BIT(0) // 扩展帧
#define MSG_FLAG_RTR  BIT(1) // 远程帧
#define MSG_FLAG_ECHO BIT(3)

#define CAN_RECV_DATA_SIZE 100

typedef enum
{
    CAN_BAUDRATE_1000K = 0,
    CAN_BAUDRATE_800K,
    CAN_BAUDRATE_500K,
    CAN_BAUDRATE_250K,
    CAN_BAUDRATE_125K,
    CAN_BAUDRATE_ERR,
} baudrate_e;

class ZlgCan : public QObject
{
    Q_OBJECT
public:
    explicit ZlgCan();
    ~ZlgCan();
    bool open_device(uint8_t device_index, baudrate_e baudrate);
    bool open_device(uint8_t device_index, uint32_t baudrate);
    uint transmit(uint32_t id, uint flag, uint8_t *data, uint16_t len);

    void close_device();

signals:
    void sig_receive(uint32_t id, uint flag, uint8_t *data, uint16_t len);

private:
    DEVICE_HANDLE  device_handle;
    CHANNEL_HANDLE channel_handle;
    QTimer         receive_timer;

    uint8_t    device_index  = 0;
    uint32_t   u32_baudrate  = 500000;
    baudrate_e baudrate_enum = CAN_BAUDRATE_500K;

    ZCAN_Receive_Data  recv_data[CAN_RECV_DATA_SIZE];
    ZCAN_Transmit_Data transmit_data;

    bool set_baudrate(uint16_t channel_index, baudrate_e baudrate);
    bool set_baudrate(uint16_t channel_index, uint baudrate);
    void receive();
};

#endif
