#include "j1939.h"
#include "drv_can.h"
#include "elog.h"
#include "system.h"

#define J1939_1_DBG(x...) log_i(x)

#define j1939_1_CAN_INS   CAN1_INSTANCE
#define j1939_1_SRC_ADDR  0x40
#define j1939_1_DEST_ADDR 0x20

j1939_t j1939_1_ins;

void j1939_1_recv(const can_receive_message_struct msg)
{
    j1939_receive_handle(&j1939_1_ins, msg.rx_efid, (uint8_t *)msg.rx_data, msg.rx_dlen);
}

int j1939_1_can_init(void)
{
#define CAN_RX_CB_REGISTER(ins, id, cb)                          \
    static can_rec_cb_t cb##_node = CAN_EXT_REC_CB_INIT(id, cb); \
    drv_can_install_rec_cb(ins, &cb##_node);

    CAN_RX_CB_REGISTER(j1939_1_CAN_INS, CAN_LISTEN_ALL_ID, j1939_1_recv);
    return drv_can_init(j1939_1_CAN_INS);
}

int j1939_1_can_write(uint32_t id, uint8_t *data, uint8_t len)
{
    drv_can_send_data(j1939_1_CAN_INS, id, MSG_FLAG_EXT, data, len);
}

uint8_t j1939_1_test_data[128];

int get_1_data(uint32_t pgn, uint8_t src, uint8_t **data, uint16_t *len)
{
    *data = j1939_1_test_data;
    *len  = sizeof(j1939_1_test_data);
    J1939_1_DBG("j1939_1 src %d, pgn 0x%x get data", src, pgn);
}

void err_1_handle(uint32_t pgn, uint8_t src)
{
    J1939_1_DBG("j1939_1 src %d, pgn 0x%x transfer err", src, pgn);
}

void rec_1_finish_handle(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t len)
{
    J1939_1_DBG("j1939_1 src %d, pgn 0x%x transfer finish, len %d", src, pgn, len);
}

void j1939_1_ins_init(void)
{
    for (int i = 0; i < sizeof(j1939_1_test_data); i++)
    {
        j1939_1_test_data[i] = i;
    }

    j1939_1_ins.interface.get_time_ms = sys_get_ms;
    j1939_init(&j1939_1_ins, j1939_1_SRC_ADDR, j1939_1_can_init, j1939_1_can_write);
    j1939_tp_rx_register(&j1939_1_ins, j1939_1_DEST_ADDR, j1939_1_SRC_ADDR, get_1_data, rec_1_finish_handle, err_1_handle);
    // j1939_tp_rx_register(&j1939_1_ins, j1939_1_DEST_ADDR, j1939_1_SRC_ADDR, get_1_data, rec_1_finish_handle, err_1_handle);
    j1939_tp_rx_register(&j1939_1_ins, j1939_1_DEST_ADDR, ADDRESS_NULL, get_1_data, rec_1_finish_handle, err_1_handle);
}

void j1939_1_poll(void)
{
    j1939_tp_poll(&j1939_1_ins);
}

#define BMT_ID (0x1816FEFE)
typedef struct
{
    uint64_t bmt_batterytemp0  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp1  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp2  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp3  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp4  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp5  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp6  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp7  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp8  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp9  : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp10 : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp11 : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp12 : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp13 : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp14 : 8; /**  #rang: -50..200 #unit: degC */
    uint64_t bmt_batterytemp15 : 8; /**  #rang: -50..200 #unit: degC */
} bmt_t;
bmt_t bmt_1_test;

void j1939_1_bam_test(void)
{
    static uint8_t test_cnt = 0;
#define BMT_SET(x) bmt_1_test.bmt_batterytemp##x = x * 2;
    BMT_SET(0);
    BMT_SET(1);
    BMT_SET(2);
    BMT_SET(3);
    BMT_SET(4);
    BMT_SET(5);
    BMT_SET(6);
    BMT_SET(7);
    BMT_SET(8);
    BMT_SET(9);
    BMT_SET(10);
    BMT_SET(11);
    BMT_SET(12);
    BMT_SET(13);
    BMT_SET(14);
    test_cnt++;
    bmt_1_test.bmt_batterytemp15++;
    // send_tp_bam(&j1939_1_ins, 0x1CFE, J1939_PRIORITY_DEFAULT, ADDRESS_NULL, (uint8_t *)&bmt_1_test, sizeof(bmt_1_test));
   // send_tp_bam(&j1939_1_ins, 0x1CFE, J1939_PRIORITY_DEFAULT, ADDRESS_NULL, (uint8_t *)&bmt_1_test, sizeof(bmt_1_test));
}

void j1939_1_test(void)
{
    
      j1939_1_bam_test();
    // sys_delay_ms(1000);
    // j1939_tp(&j1939_1_ins, 0x1600, J1939_PRIORITY_DEFAULT, j1939_1_ins.node_addr, j1939_1_DEST_ADDR, (uint8_t *)&bmt_1_test , sizeof(bmt_1_test));
    // send_tp_bam(&j1939_1_ins, 0x17EF, J1939_PRIORITY_DEFAULT, ADDRESS_NULL, (uint8_t *)&bmt_1_test, sizeof(bmt_1_test));
    // sys_delay_ms(1000);
}