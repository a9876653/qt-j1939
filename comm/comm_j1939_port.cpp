#include "comm_j1939.h"
#include "comm_j1939_port.h"
#include "QDebug"
#include "j1939_modbus_master.h"
#include "j1939_event.h"

#define BOOT_PORT_DBG(x...) qDebug(x)

void j1939_err_handle(uint32_t pgn, uint8_t dst, uint8_t src)
{
    (void)pgn;
    (void)src;
    (void)dst;
    BOOT_PORT_DBG("dst %d src %d pgn %d tp err!", dst, src, pgn);
}

void j1939_recv_cb(uint32_t pgn, uint8_t dst, uint8_t src, uint8_t *data, uint16_t len)
{
    (void)dst;
    (void)len;
    (void)data;
    if (pgn == DB_FUNC_READ_HOLDING_REGISTER)
    {
        respond_read_reg_t *ptr = (respond_read_reg_t *)data;
        J1939DbIns->recv_read_reg_handle(src, ptr);
    }
    else if (pgn == PGN_RESPOND_EVENT)
    {
        read_event_respond_t *ptr = (read_event_respond_t *)data;
        j1939_recv_read_event(src, ptr);
    }
    else
    {
        QVector<uint8_t> array(len);
        memcpy(&array[0], data, len);
        J1939Ins->recv_pgn_handle(pgn, src, array);
    }
}

j1939_ret_e comm_j1939_pgn_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    if (msg->len > 8)
    {
        return J1939_EWRONG_DATA_LEN;
    }
    QVector<uint8_t> array(msg->len);
    memcpy(&array[0], msg->data, msg->len);
    J1939Ins->recv_pgn_handle(msg->pgn, msg->src, array);
    return J1939_OK;
}
#include "file_transfer_type.h"
void comm_j1939_port_init(QMap<uint, MsgData *> &msgs_map)
{
    J1939Ins->session_cb_register(j1939_recv_cb, j1939_err_handle);

    for (MsgData *msg_data : msgs_map)
    {
        J1939Ins->pgn_register(msg_data->pgn, 0, comm_j1939_pgn_cb);
    }
    J1939Ins->pgn_register(FILE_MSG_ID_START_RESPONE, 0, comm_j1939_pgn_cb);
    J1939Ins->pgn_register(FILE_MSG_ID_FILE_INFO_RESPONE, 0, comm_j1939_pgn_cb);
    J1939Ins->pgn_register(FILE_MSG_ID_FILE_DATA_RESPONE, 0, comm_j1939_pgn_cb);
    J1939Ins->pgn_register(FILE_MSG_ID_CHECK_CRC_RESPONE, 0, comm_j1939_pgn_cb);
    J1939Ins->pgn_register(BOOT_MSG_ID_START_UPDATE_RESPONE, 0, comm_j1939_pgn_cb);
    J1939Ins->pgn_register(BOOT_MSG_ID_READ_UPDATE_INFO_RESPONE, 0, comm_j1939_pgn_cb);
    j1939_event_init();
}
