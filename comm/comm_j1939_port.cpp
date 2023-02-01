#include "comm_j1939.h"
#include "comm_j1939_port.h"
#include "QDebug"
#include "j1939_modbus_master.h"

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
    if (pgn == DB_FUNC_READ_HOLDING_REGISTER)
    {
        respond_read_reg_t *ptr = (respond_read_reg_t *)data;
        J1939DbIns->recv_read_reg_handle(src, ptr);
    }
    else
    {
        J1939Ins->recv_pgn_handle(pgn, src, data, len);
    }
}

j1939_ret_e comm_j1939_pgn_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    J1939Ins->recv_pgn_handle(msg->pgn, msg->src, msg->data, msg->len);
    return J1939_OK;
}

void comm_j1939_port_init(QMap<uint, MsgData *> &msgs_map)
{
    for (int i = 0; i < J1939_NODE_ADDR_MAX; i++)
    {
        uint8_t *src_data = (uint8_t *)malloc(512);
        J1939Ins->tp_rx_data_register(i, J1939_SRC_ADDR, src_data, 512, j1939_recv_cb, NULL);
        uint8_t *global_data = (uint8_t *)malloc(512);
        J1939Ins->tp_rx_data_register(i, ADDRESS_GLOBAL, global_data, 512, j1939_recv_cb, NULL);
    }
    for (MsgData *msg_data : msgs_map)
    {
        J1939Ins->pgn_register(msg_data->pgn, 0, comm_j1939_pgn_cb);
    }
}
