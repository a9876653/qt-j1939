#include "comm_j1939.h"
#include "QDateTime"
#include "pgn.h"

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

void CommJ1939::set_src_addr(uint8_t addr)
{
    // 改变源地址的订阅哈希值
    for (int i = 0; i < J1939_NODE_ADDR_MAX; i++)
    {
        j1939_sessions_t *handle = &j1939_ins.sessions;
        j1939_session_t  *sess   = j1939_session_search_addr(handle, i, j1939_ins.node_addr);
        uint16_t          key    = (i << 8) | j1939_ins.node_addr;
        hasht_delete(&handle->sessions, key);
        uint16_t new_key = (i << 8) | addr;
        hasht_insert(&handle->sessions, new_key, sess);
    }
    j1939_ins.node_addr = addr;
}

void CommJ1939::set_dst_addr(uint8_t addr)
{
    dst_addr = addr;
}

uint8_t CommJ1939::get_src_addr()
{
    return j1939_ins.node_addr;
}

uint8_t CommJ1939::get_dst_addr()
{
    return dst_addr;
}

void CommJ1939::slot_msg_send(uint32_t pgn, uint8_t *data, uint16_t len)
{
    msg_send(pgn, J1939_PRIORITY_DEFAULT, dst_addr, data, len, J1939_DEF_TIMEOUT);
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

void CommJ1939::slot_request_pgn(uint32_t pgn, uint8_t dst, uint16_t len)
{
    uint8_t data[5] = {
        PGN_SPECIFIC(pgn),
        PGN_FORMAT(pgn),
        PGN_DATA_PAGE(pgn),
        (len & 0xFF),
        (len & 0xFF00) >> 8,
    };
    j1939_send_msg(&j1939_ins, RAC, J1939_PRIORITY_DEFAULT, dst, data, sizeof(data), J1939_DEF_TIMEOUT);
}
