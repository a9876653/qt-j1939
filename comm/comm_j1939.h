#ifndef __COMM_j1939
#define __COMM_j1939

#include "j1939_config.h"
#include "j1939_user_if.h"
#include "zlgcan_ctrl.h"
#include "singleton.h"

namespace CommJ1939
{

void init();

void j1939_poll(void);
void j1939_set_src_addr(uint8_t addr);
int  j1939_boot_msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, uint8_t *data, uint16_t len, uint32_t timeout);
int  j1939_boot_pgn_register(const uint32_t pgn, uint8_t code, pgn_callback_t cb);
int  j1939_boot_tp_rx_register(uint8_t              src,
                               uint8_t              dst,
                               session_get_data_fun get_data,
                               session_recv_fun     rec_finish,
                               session_err_fun      err_handle);

int  j1939_can_write(uint32_t id, uint8_t *data, uint8_t len);
void j1939_recv(uint32_t id, uint flag, uint8_t *data, uint16_t len);

}; // namespace CommJ1939

#endif
