#pragma once

#include "file_transfer_type.h"
#include "state_machine.h"
#include "boot_header.h"
#include "system.h"

enum file_transfer_master_e
{
    FILE_MASTER_IDLE = 0,
    FILE_MASTER_WAIT_CMD_RESPOND,
    FILE_MASTER_WAIT_INFO_RESPOND,
    FILE_MASTER_WAIT_FILE_DATA_RESPOND,
    FILE_MASTER_WAIT_CRC_CHECK_RESPOND,
    FILE_MASTER_STEP_NUM,
};

typedef struct file_transfer_step_s file_transfer_step_t;

typedef void (*respond_finish_fun)(file_transfer_step_t *handle);

struct file_transfer_step_s
{
    void *             parent;
    respond_finish_fun finish_handle;
    uint32_t           step;
    uint32_t           offset;
    timeMs_t           over_time;
    timeMs_t           over_tick;
    uint32_t           resend_cnt;
    uint32_t           resend_cnt_max;
};
#ifdef __cplusplus

#include <functional>
typedef struct
{
    std::function<int(uint32_t msg_id, uint8_t *data, uint16_t len)> send;

    std::function<void(uint8_t result)> update_failed_cb;

    std::function<uint32_t(uint32_t offset, uint8_t *buff, uint32_t buff_size)> get_file_data;

    std::function<void(uint8_t src, int result, uint32_t offset)> transfer_state_cb;

} file_master_if_t;
#else
typedef struct
{
    int (*send)(uint32_t msg_id, uint8_t *data, uint16_t len);
    void (*update_failed_cb)(uint8_t result);
    uint32_t (*get_file_data)(uint32_t offset, uint8_t *buff, uint32_t buff_size);
    void (*verify_finish_cb)(uint8_t src, int result);

} file_master_if_t;
#endif

typedef struct
{
    rt_slist_t           root;
    state_machine_t *    cur_step;
    rt_slist_t           node_root;
    file_transfer_step_t transfer_step[FILE_MASTER_STEP_NUM];

    file_master_if_t *interface;

    file_info_t    file_info;
    file_data_t    file_data;
    cmd_update_t   cmd_update;
    file_process_t file_process;

    float update_process;

    uint32_t msg_send_id;
    uint8_t  msg_send_buff[FILE_DATA_RECV_MAX_SIZE + 10];
    uint16_t msg_send_buff_len;
} file_transfer_master_t;

void file_transfer_master_init(file_transfer_master_t *handle, file_master_if_t *interface);
void file_transfer_master_task(file_transfer_master_t *handle);
int  file_master_recv_handle(file_transfer_master_t *handle, uint8_t src, uint32_t msg_id, uint8_t *data, uint32_t len);
void file_master_update_start(file_transfer_master_t *handle, file_process_t *cmd, file_info_t *file_info);
void file_master_send_update_cmd(file_transfer_master_t *handle, file_process_t *cmd);
void file_master_check_crc_request(file_transfer_master_t *handle);
