#include "comm_j1939.h"
#include "comm_j1939_port.h"
#include "QDebug"

#define BOOT_PORT_DBG(x...) qDebug(x)

void j1939_err_handle(uint32_t pgn, uint8_t src)
{
    (void)pgn;
    (void)src;
}

void j1939_recv_cb(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t len)
{
    (void)src;
    (void)len;
    (void)data;
    J1939Ins->recv_pgn_handle(pgn, src, data, len);
}

j1939_ret_e comm_j1939_pgn_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    J1939Ins->recv_pgn_handle(msg->pgn, msg->src, msg->data, msg->len);
    return J1939_OK;
}

uint8_t temp_buff[128][2][512];
void    comm_j1939_port_init(QMap<uint, MsgData> &msgs_map)
{
    for (int i = 0; i < 26; i++)
    {
        J1939Ins->tp_rx_data_register(i, J1939_SRC_ADDR, temp_buff[i][0], 512, j1939_recv_cb, NULL);
        J1939Ins->tp_rx_data_register(i, ADDRESS_GLOBAL, temp_buff[i][1], 512, j1939_recv_cb, NULL);
    }
    for (MsgData msg_data : msgs_map)
    {
        J1939Ins->pgn_register(msg_data.pgn, 0, comm_j1939_pgn_cb);
    }
}
