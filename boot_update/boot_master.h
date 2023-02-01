#ifndef __BOOT_MASTER_H
#define __BOOT_MASTER_H

// #include "app_handle.h"
#include "boot_header.h"

/** 外部实现部分 **/
void     boot_send(uint32_t msg_id, uint8_t *data, uint16_t len);
void     boot_update_failed(void);
uint32_t get_file_data(uint32_t offset, uint8_t *data, uint16_t len);

/** 外部调用部分 **/
float get_update_process(void);

void boot_master_poll();

void boot_send_update_cmd(cmd_update_t *handle);

void boot_master_update_satrt(cmd_update_t *cmd_update, app_info_t *app_info);
void boot_recv_update_cmd_respond(uint8_t src_addr, uint8_t ret);
void boot_recv_file_info_respond(uint8_t src_addr, uint8_t ret);
void boot_recv_file_data_respond(uint8_t src_addr, respone_file_data_t *ret);
void boot_recv_crc_check_respond(uint8_t src_addr, uint8_t ret);
void boot_recv_boot_heartbeat(uint8_t src_addr);
void boot_recv_app_heartbeat(uint8_t src_addr);

void boot_update_crc_check_respond_finish(void);

void enter_boot(uint8_t *obj_addr, uint8_t obj_num);

void enter_app();

void app_info_request();

#endif
