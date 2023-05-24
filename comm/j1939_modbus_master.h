#pragma once

#include "QMap"
#include "QObject"
#include "singleton.h"
#include "j1939_modbus.h"
#include "comm_j1939.h"
#include "comm_j1939_port.h"
#include "mthread.h"

class CommDbValue : public QObject
{
    Q_OBJECT
public:
    CommDbValue(uint16_t reg_addr, uint16_t reg_size) : reg_addr(reg_addr), reg_size(reg_size)
    {
    }
    uint16_t reg_addr;
    uint16_t reg_size;
signals:
    void sig_read_finish(uint32_t value);
    void sig_write_finish();
};

class CommJ1939Db : public QObject
{
    Q_OBJECT
public:
    typedef struct
    {
        uint8_t  dst;
        uint16_t reg_addr;
        uint16_t reg_len;
    } db_request_read_t;

public:
    CommJ1939Db();
    void         init();
    CommDbValue *db_reg_register(uint16_t src_addr, uint16_t reg_addr, uint16_t reg_size);
    void         recv_read_reg_handle(uint16_t src_addr, respond_read_reg_t *ptr);
    void         recv_write_reg_handle(uint16_t src_addr, request_write_reg_t *ptr);
    void         recv_write_mul_reg_handle(uint16_t src_addr, respond_write_mul_reg_t *ptr);
signals:
    void sig_recv_read_reg(uint16_t reg_addr, QByteArray array);
    void sig_recv_write_reg(uint16_t reg_addr, uint16_t reg_value);
    void sig_recv_write_mul_reg(uint16_t reg_addr, uint8_t reg_len);

public slots:
    void slot_request_dst_read_reg(uint16_t dst, uint16_t reg_addr, uint16_t reg_len);
    void slot_request_dst_write_reg(uint16_t dst, uint16_t reg_addr, uint16_t reg_value);
    void slot_request_dst_write_mul_reg(uint16_t dst, uint16_t reg_addr, QVector<uint16_t> array);
    void slot_request_read_reg(uint16_t reg_addr, uint16_t reg_len);
    void slot_request_write_reg(uint16_t reg_addr, QVector<uint16_t> array);

private:
    int msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, QByteArray array);

    void request_read_thread_task();

    QMap<uint16_t, CommDbValue *> db_map;

    QQueue<db_request_read_t> db_read_queue;

    MThread   *request_read_thread = nullptr;
    QSemaphore read_wait_sem;
    QSemaphore read_request_sem;
};

#define J1939DbIns Singleton<CommJ1939Db>::getInstance()
