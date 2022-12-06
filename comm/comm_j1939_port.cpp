#include "comm_j1939.h"
#include "QDebug"

#define BOOT_PORT_DBG(x...) qDebug(x)
typedef int (*session_get_data_fun)(uint32_t pgn, uint8_t src, uint8_t **data, uint16_t *len);
typedef void (*session_err_fun)(uint32_t pgn, uint8_t src);
typedef void (*session_recv_fun)(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t len);
int j1939_get_data(uint32_t pgn, uint8_t src, uint8_t **data, uint16_t *len)
{
    (void)pgn;
    (void)src;
    *data = new uint8_t[512];
    *len  = 512;
    return *len;
}

void j1939_err_handle(uint32_t pgn, uint8_t src)
{
}

void j1939_recv_cb(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t len)
{
    (void)src;
    (void)len;
    (void)data;
    if (pgn == BOOT_MSG_ID_READ_APP_INFO_RESPONE)
    {
        app_info_str_t str = get_app_info_str((app_info_t *)data);
        MiddleSignalIns->app_info_update(str);
    }
    BOOT_PORT_DBG("boot recv pgn 0x%04x, src: %d, len %d", pgn, src, len);
}

j1939_ret_e comm_j1939_pgn_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;

    return J1939_OK;
}

void comm_j1939_port_init(void)
{
    for (int i = 0; i < ADDR_NUM_MAX; i++)
    {
        J1939Ins->tp_rx_register(i, J1939_SRC_ADDR, j1939_get_data, j1939_recv_cb, NULL);
    }
    J1939Ins->pgn_register(BOOT_MSG_ID_UPDATE_CMD_RESPONE, 0, comm_j1939_pgn_cb);
    J1939Ins->pgn_register(BOOT_MSG_ID_FILE_INFO_RESPONE, 0, comm_j1939_pgn_cb);
    J1939Ins->pgn_register(BOOT_MSG_ID_FILE_DATA_RESPONE, 0, comm_j1939_pgn_cb);
    J1939Ins->pgn_register(BOOT_MSG_ID_CHECK_CRC_RESPONE, 0, comm_j1939_pgn_cb);
}
