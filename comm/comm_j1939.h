#ifndef __COMM_j1939
#define __COMM_j1939

#include "j1939_config.h"
#include "j1939_user_if.h"
#include "zlgcan_ctrl.h"
#include "singleton.h"
#include "ctrlcan.h"

#define J1939_SRC_ADDR  0x7E // 默认源地址
#define PGN_REG_NUM     4096 // 注册的PGN数（单帧），只接收
#define SESSION_REG_NUM 2048 // 注册的会话数（多帧），包含发送和接收

class CommJ1939 : public QObject
{
    Q_OBJECT
public:
    CommJ1939();
    void init();

    bool open_device(uint8_t device_index, uint32_t baudrate);

    void close_device();

    void    poll(void);
    void    set_src_addr(uint8_t addr);
    void    set_dst_addr(uint8_t addr);
    uint8_t get_src_addr();
    uint8_t get_dst_addr();

    int pgn_register(const uint32_t pgn, uint8_t code, pgn_callback_t cb);
    int tp_rx_register(uint8_t              src,
                       uint8_t              dst,
                       session_get_data_fun get_data,
                       session_recv_fun     rec_finish,
                       session_err_fun      err_handle);

    int tp_rx_data_register(uint8_t          src,
                            uint8_t          dst,
                            uint8_t         *data,
                            uint16_t         data_size,
                            session_recv_fun rec_finish,
                            session_err_fun  err_handle);

    int  can_write(uint32_t id, QByteArray array);
    void can_recv(uint32_t id, uint flag, QByteArray array);

    void recv_pgn_handle(uint32_t pgn, uint8_t src, QByteArray array);

private:
    int msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, QByteArray array, uint32_t timeout);

public slots:
    void slot_msg_send(uint32_t pgn, QByteArray array);
    void slot_request_pgn(uint32_t pgn, uint8_t dst, uint16_t len);

signals:
    int  sig_msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, QByteArray array, uint32_t timeout);
    void sig_recv_pgn_handle(uint32_t pgn, uint8_t src, QByteArray array);
    void sig_open_finish(int ret);

private:
    CanBase *can_dev = nullptr;
    j1939_t  j1939_ins;
    QTimer   j1939_poll_timer;
    uint8_t  dst_addr = ADDRESS_GLOBAL;
};

#define J1939Ins Singleton<CommJ1939>::getInstance()

#endif
