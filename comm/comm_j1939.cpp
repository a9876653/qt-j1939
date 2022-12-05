#include "comm_j1939.h"
#include "QDateTime"

#define J1939_SRC_ADDR  0x40 // 默认源地址
#define PGN_REG_NUM     100  // 注册的PGN数（单帧），只接收
#define SESSION_REG_NUM 200  // 注册的会话数（多帧），包含发送和接收

using namespace CommJ1939;

j1939_t j1939_ins;
ZlgCan  zlgcan_ins;
QTimer  j1939_poll_timer;

void CommJ1939::j1939_recv(uint32_t id, uint flag, uint8_t *data, uint16_t len)
{
    (void)flag;
    j1939_receive_handle(&j1939_ins, id, data, len);
}

int CommJ1939::j1939_can_write(uint32_t id, uint8_t *data, uint8_t len)
{
    if (zlgcan_ins.transmit(id, MSG_FLAG_EXT, data, len) == 1)
    {
        return len;
    }
    return 0;
}

void CommJ1939::j1939_poll(void)
{
    j1939_tp_poll(&j1939_ins);
}

void CommJ1939::init()
{
    zlgcan_ins.open_device(0, CAN_BAUDRATE_500K);
    memset(&j1939_ins, 0, sizeof(j1939_ins));
    j1939_init(&j1939_ins, J1939_SRC_ADDR, PGN_REG_NUM, SESSION_REG_NUM, j1939_can_write);
    j1939_poll_timer.start(5);
    j1939_poll_timer.connect(&j1939_poll_timer, &QTimer::timeout, &CommJ1939::j1939_poll);
    zlgcan_ins.connect(&zlgcan_ins, &ZlgCan::sig_receive, &CommJ1939::j1939_recv);
}

int CommJ1939::j1939_boot_msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, uint8_t *data, uint16_t len, uint32_t timeout)
{
    j1939_ret_e ret = J1939_OK;
    if (len > 8)
    {
        if (dst == ADDRESS_GLOBAL)
        {
            ret = j1939_tp_bam(&j1939_ins, pgn, priority, dst, data, len);
        }
        else
        {
            ret = j1939_tp(&j1939_ins, pgn, priority, j1939_ins.node_addr, dst, data, len);
        }
    }
    else
    {
        ret = j1939_send_msg(&j1939_ins, pgn, priority, dst, data, len, timeout);
    }
    if (ret == J1939_OK)
    {
        return len;
    }
    return 0;
}

int CommJ1939::j1939_boot_pgn_register(const uint32_t pgn, uint8_t code, pgn_callback_t cb)
{
    return j1939_pgn_register(&j1939_ins, pgn, code, cb);
}
int CommJ1939::j1939_boot_tp_rx_register(uint8_t              src,
                                         uint8_t              dst,
                                         session_get_data_fun get_data,
                                         session_recv_fun     rec_finish,
                                         session_err_fun      err_handle)
{
    return j1939_tp_rx_register(&j1939_ins, src, dst, get_data, rec_finish, err_handle);
}
