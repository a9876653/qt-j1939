#pragma once
#include "QTimer"
#include "QThread"
#include "blocking_queue.h"
#include "QObject"
#include "stdint.h"
#include <QDebug>
#include "mthread.h"
#include "mthreadqueue.h"

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
    class can_farme_t
    {
    public:
        uint32_t id;
        uint     flag;
        uint8_t  len;
        uint8_t  data[CAN_MAX_DATA_LEN];
    };

public:
    CanBase()
    {
        can_task_thread = new MThread(std::bind(&CanBase::can_task, this));
        transmit_queue.set_max_size(4096);
        recv_queue.set_max_size(4096);
    }

    ~CanBase()
    {
        can_task_thread->stop();
    }

    bool open_device(uint8_t device_index, uint32_t baudrate)
    {
        m_device_index = device_index;
        m_baudrate     = baudrate;

        can_task_thread->start();
        m_request_start = true;
        // emit sig_open_device(device_index, baudrate);
        return true;
    }

    void close_device()
    {
        m_request_stop = true;
        can_task_thread->stop();
        // emit sig_close_device();
    }

    virtual bool is_open()
    {
        return true;
    }

    virtual uint transmit(uint32_t id, uint flag, QVector<uint8_t> array)
    {
        int len = array.size();
        if (!is_open())
        {
            // CAN_DBG("CAN Device is close");
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
        memcpy(transmit_data.data, &array[0], len);
        transmit_queue.enqueue(transmit_data);

        return 1;
    }

    virtual void transmit_task()
    {
    }

    virtual void receive_task()
    {
    }

    bool transmit_dequeue(can_farme_t &transmit_data)
    {
        if (transmit_queue.isEmpty())
        {
            return false;
        }
        return transmit_queue.dequeue(transmit_data);
    }

    bool recv_dequeue(can_farme_t &recv_data)
    {
        if (recv_queue.isEmpty())
        {
            return false;
        }
        return recv_queue.dequeue(recv_data);
    }

    void can_task()
    {
        if (m_request_start)
        {
            m_request_start = false;
            emit sig_open_device(m_device_index, m_baudrate);
        }
        else
        {
            transmit_task();
            receive_task();
            if (extern_task != nullptr)
            {
                extern_task();
            }
            QThread::msleep(1);

            if (m_request_stop)
            {
                m_request_stop = false;
                emit sig_close_device();
            }
        }
    }
signals:
    void sig_receive(uint32_t id, uint flag, QVector<uint8_t> array);
    void sig_open_device(uint8_t device_index, uint32_t baudrate);
    void sig_open_finish(int ret);
    void sig_close_device();

public:
    MThread::thread_task_t extern_task;

    ThreadSafeQueue<can_farme_t> recv_queue;

private:
    ThreadSafeQueue<can_farme_t> transmit_queue;

    MThread *can_task_thread = nullptr;

    uint8_t  m_device_index;
    uint32_t m_baudrate;
    bool     m_request_start = false;
    bool     m_request_stop  = false;
};
