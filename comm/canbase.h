#pragma once
#include "QTimer"
#include "QThread"
#include "blocking_queue.h"
#include "QObject"
#include "stdint.h"
#include <QDebug>

#define CAN_DBG(x...) qDebug(x)

/** CAN  **/
#define BIT(x)        (1 << (x))
#define MSG_FLAG_STD  (0)
#define MSG_FLAG_EXT  BIT(0) // 扩展帧
#define MSG_FLAG_RTR  BIT(1) // 远程帧
#define MSG_FLAG_ECHO BIT(3)

#define CAN_MAX_DATA_LEN   8
#define CAN_RECV_DATA_SIZE 100
#define CAN_SEND_DATA_SIZE 100

typedef enum
{
    CAN_BAUDRATE_1000K = 0,
    CAN_BAUDRATE_800K,
    CAN_BAUDRATE_500K,
    CAN_BAUDRATE_250K,
    CAN_BAUDRATE_125K,
    CAN_BAUDRATE_ERR,
} baudrate_e;

typedef enum
{
    CAN_OPEN_SUCCESS = 0,
    CAN_OPEN_FAILED,
} can_device_err_e;

class CanBase : public QObject
{
    Q_OBJECT
public:
    typedef struct
    {
        uint32_t id;
        uint     flag;
        uint8_t  len;
        uint8_t  data[CAN_MAX_DATA_LEN];
    } can_farme_t;

public:
    bool open_device(uint8_t device_index, uint32_t baudrate)
    {
        emit sig_open_device(device_index, baudrate);
        return true;
    }

    void close_device()
    {
        emit sig_close_device();
    }

    virtual bool is_open()
    {
        return true;
    }

    virtual uint transmit(uint32_t id, uint flag, uint8_t *data, uint16_t len)
    {
        if (transmit_queue.count() > CAN_SEND_DATA_SIZE)
        {
            CAN_DBG("CAN Transmit Queue is full");
            return 0;
        }
        if (!is_open())
        {
            CAN_DBG("CAN Device is close");
            return 0;
        }
        if (len > CAN_MAX_DATA_LEN)
        {
            return 0;
        }

        can_farme_t transmit_data;
        memset(&transmit_data, 0, sizeof(transmit_data));
        transmit_data.id   = id;
        transmit_data.len  = len;
        transmit_data.flag = flag;
        memcpy(transmit_data.data, data, len);
        transmit_queue.enqueue(transmit_data);

        return 1;
    }

    bool transmit_dequeue(can_farme_t &transmit_data)
    {
        if (transmit_queue.isEmpty())
        {
            return false;
        }
        transmit_data = transmit_queue.dequeue();
        return true;
    }

signals:
    void sig_receive(uint32_t id, uint flag, uint8_t *data, uint16_t len);
    void sig_open_device(uint8_t device_index, uint32_t baudrate);
    void sig_open_finish(int ret);
    void sig_close_device();

private:
    QQueue<can_farme_t> transmit_queue;
};