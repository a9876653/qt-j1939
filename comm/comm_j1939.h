#ifndef __COMM_j1939
#define __COMM_j1939

#include "j1939_config.h"
#include "j1939_user_if.h"
#include "zlgcan_ctrl.h"
#include "singleton.h"
#include "ctrlcan.h"
#include "mthreadperiodtask.h"

#define J1939_SRC_ADDR  0x7E // 默认源地址
#define PGN_REG_NUM     4096 // 注册的PGN数（单帧），只接收
#define SESSION_REG_NUM 2048 // 注册的会话数（多帧），包含发送和接收

class CommJ1939 : public QObject
{
    Q_OBJECT
public:
    CommJ1939();
    ~CommJ1939();

    bool is_open();

    bool open_device(uint8_t device_index, uint32_t baudrate);

    void close_device();

    void    set_src_addr(uint8_t addr);
    void    set_dst_addr(uint8_t addr);
    uint8_t get_src_addr();
    uint8_t get_dst_addr();

    int pgn_register(const uint32_t pgn, uint8_t code, pgn_callback_t cb);
    int session_cb_register(session_recv_fun recv_cb, session_err_fun err_cb);

    int can_write(uint32_t id, QVector<uint8_t> array);

    void recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> array);

private:
    void init();
    int  msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, QVector<uint8_t> array, uint32_t timeout);
    void can_recv_task();
    void poll(void);

public slots:
    void slot_msg_send(uint32_t pgn, QVector<uint8_t> array);
    void slot_request_pgn(uint32_t pgn, uint8_t dst, uint16_t len);

signals:
    int  sig_msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, QVector<uint8_t> array, uint32_t timeout);
    void sig_recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> array);
    void sig_open_finish(int ret);

private:
    MThreadPeriodTask *comm_thread = nullptr;

    CanBase *can_dev = nullptr;

    j1939_t j1939_ins;
    QTimer  j1939_poll_timer;
    uint8_t dst_addr = ADDRESS_GLOBAL;
};

#define J1939Ins Singleton<CommJ1939>::getInstance()

#endif
