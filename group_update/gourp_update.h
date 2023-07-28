#pragma once

#include <QObject>
#include <QTimer>
#include "file_transfer_type.h"

typedef struct
{
    uint8_t process;
    uint8_t obj_num;
    uint8_t obj_addr[OBJ_ADDR_MAX_NUM];
} update_info_t;

class GroupUpdate : public QObject
{
    Q_OBJECT
public:
    GroupUpdate();

    void start(QVector<uint8_t> array);
    void stop();
    void request_update_info();
signals:
    void sig_update_state(update_info_t info);
    void sig_dbg_info(QString &s);

private:
    int send(uint32_t msg_id, uint8_t *data, uint16_t len);

    void slot_timeout();
    void slot_recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> data);

private:
    QTimer        m_timer;
    update_info_t m_update_info;
};
