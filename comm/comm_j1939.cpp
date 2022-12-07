#include "comm_j1939.h"
#include "QDateTime"

int j1939_can_write(uint32_t id, uint8_t *data, uint8_t len)
{
    return J1939Ins->can_write(id, data, len);
}

void CommJ1939::can_recv(uint32_t id, uint flag, uint8_t *data, uint16_t len)
{
    (void)flag;
    j1939_receive_handle(&j1939_ins, id, data, len);
}

int CommJ1939::can_write(uint32_t id, uint8_t *data, uint8_t len)
{
    if (transmit(id, MSG_FLAG_EXT, data, len) == 1)
    {
        return len;
    }
    return 0;
}

void CommJ1939::poll(void)
{
    j1939_tp_poll(&j1939_ins);
}

void CommJ1939::init()
{
    memset(&j1939_ins, 0, sizeof(j1939_ins));
    j1939_init(&j1939_ins, J1939_SRC_ADDR, PGN_REG_NUM, SESSION_REG_NUM, j1939_can_write);
    j1939_poll_timer.start(5);
    j1939_poll_timer.connect(&j1939_poll_timer, &QTimer::timeout, this, &CommJ1939::poll);
    connect(this, &ZlgCan::sig_receive, this, &CommJ1939::can_recv);
}

int CommJ1939::msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, uint8_t *data, uint16_t len, uint32_t timeout)
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

void CommJ1939::recv_pgn_handle(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t data_size)
{
    emit this->sig_recv_pgn_handle(pgn, src, data, data_size);
}

int CommJ1939::pgn_register(const uint32_t pgn, uint8_t code, pgn_callback_t cb)
{
    return j1939_pgn_register(&j1939_ins, pgn, code, cb);
}

int CommJ1939::tp_rx_register(uint8_t              src,
                              uint8_t              dst,
                              session_get_data_fun get_data,
                              session_recv_fun     rec_finish,
                              session_err_fun      err_handle)
{
    return j1939_tp_rx_register(&j1939_ins, src, dst, get_data, rec_finish, err_handle);
}

int CommJ1939::tp_rx_data_register(uint8_t          src,
                                   uint8_t          dst,
                                   uint8_t         *data,
                                   uint16_t         data_size,
                                   session_recv_fun rec_finish,
                                   session_err_fun  err_handle)
{
    return j1939_tp_rx_data_register(&j1939_ins, src, dst, data, data_size, rec_finish, err_handle);
}
