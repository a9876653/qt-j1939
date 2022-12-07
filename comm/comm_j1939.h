#ifndef __COMM_j1939
#define __COMM_j1939

#include "j1939_config.h"
#include "j1939_user_if.h"
#include "zlgcan_ctrl.h"
#include "singleton.h"

#define J1939_SRC_ADDR  0x7E // 默认源地址
#define PGN_REG_NUM     4096 // 注册的PGN数（单帧），只接收
#define SESSION_REG_NUM 2048 // 注册的会话数（多帧），包含发送和接收

class CommJ1939 : public ZlgCan
{
    Q_OBJECT
public:
    void init();

    void poll(void);
    void set_src_addr(uint8_t addr);
    int  msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, uint8_t *data, uint16_t len, uint32_t timeout);
    int  pgn_register(const uint32_t pgn, uint8_t code, pgn_callback_t cb);
    int  tp_rx_register(uint8_t              src,
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

    int  can_write(uint32_t id, uint8_t *data, uint8_t len);
    void can_recv(uint32_t id, uint flag, uint8_t *data, uint16_t len);

    void recv_pgn_handle(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t data_size);

signals:
    void sig_recv_pgn_handle(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t data_size);

private:
    j1939_t j1939_ins;
    QTimer  j1939_poll_timer;
};

#define J1939Ins Singleton<CommJ1939>::getInstance()

#endif
