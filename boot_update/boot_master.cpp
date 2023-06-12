#include "system.h"
#include "rt_slist.h"
#include "boot_master.h"
#include "boot_update_node.h"
#include "string.h"
#include "middle_signal.h"

#define BOOT_MASTER_DBG(x...) MiddleSignalIns->dbg_info(x)

#define MSG_RESEND_MAX_NUM 3

#define BOOT_MASTER_IDLE_TIME                   0
#define BOOT_MASTER_WAIT_CMD_RESPOND_TIME       30
#define BOOT_MASTER_WAIT_INFO_RESPOND_TIME      3000
#define BOOT_MASTER_WAIT_FILE_DATA_RESPOND_TIME 1000
#define BOOT_MASTER_WAIT_CRC_CHECK_RESPOND_TIME 1000
#define BOOT_MASTER_WAIT_JUMP_APP_TIME          1000

#define BOOT_MASTER_IDLE_RESEND_CNT                   0
#define BOOT_MASTER_WAIT_CMD_RESPOND_RESEND_CNT       100
#define BOOT_MASTER_WAIT_INFO_RESPOND_RESEND_CNT      3
#define BOOT_MASTER_WAIT_FILE_DATA_RESPOND_RESEND_CNT 3
#define BOOT_MASTER_WAIT_CRC_CHECK_RESPOND_RESEND_CNT 3
#define BOOT_MASTER_WAIT_JUMP_APP_RESEND_CNT          3

enum boot_master_e
{
    BOOT_MASTER_IDLE = 0,
    BOOT_MASTER_WAIT_CMD_RESPOND,
    BOOT_MASTER_WAIT_INFO_RESPOND,
    BOOT_MASTER_WAIT_FILE_DATA_RESPOND,
    BOOT_MASTER_WAIT_CRC_CHECK_RESPOND,
    BOOT_MASTER_WAIT_JUMP_APP,
};

typedef enum
{
    STATE_MACHINE_EVENT_RUN   = 0,
    STATE_MACHINE_EVENT_ENTRY = 1,
    STATE_MACHINE_EVENT_ARG   = 2,
} state_machine_event_e;

typedef struct
{
    state_machine_event_e event;
    uint32_t              arg;
} state_machine_event_t;

typedef void (*respond_finish_fun)(void);

typedef struct state_machine_s
{
    void (*func)(struct state_machine_s *self, state_machine_event_e event);
    respond_finish_fun finish_handle;

    uint32_t step;
    uint32_t offset;
    timeMs_t over_time;
    timeMs_t over_tick;
    uint32_t resend_cnt;
    uint32_t resend_cnt_max;
} state_machine_t;

#define STATE_MACHINE_INIT(f, finish_fun, s)                                                                                    \
    {                                                                                                                           \
        .func = f, .finish_handle = finish_fun, .step = s, .offset = 0, .over_time = s##_TIME, .over_tick = 0, .resend_cnt = 0, \
        .resend_cnt_max = s##_RESEND_CNT                                                                                        \
    }

void boot_update_idle(state_machine_t *self, state_machine_event_e event);
void boot_update_wait_respond(state_machine_t *self, state_machine_event_e event);

void boot_update_cmd_respond_finish(void);
void boot_update_info_respond_finish(void);
void boot_update_file_data_respond_finish(void);
void boot_update_crc_check_respond_finish(void);

app_info_t   file_info;
file_data_t  file_data;
cmd_update_t cmd_update;

uint32_t msg_send_id = 0;
uint8_t  msg_send_buff[FILE_DATA_RECV_MAX_SIZE + 10];
uint16_t msg_send_buff_len = 0;

float update_process = 0;

rt_slist_t update_root = {NULL};

state_machine_t boot_state_machine_map[]
    = {STATE_MACHINE_INIT(boot_update_idle, NULL, BOOT_MASTER_IDLE),
       STATE_MACHINE_INIT(boot_update_wait_respond, boot_update_cmd_respond_finish, BOOT_MASTER_WAIT_CMD_RESPOND),
       STATE_MACHINE_INIT(boot_update_wait_respond, boot_update_info_respond_finish, BOOT_MASTER_WAIT_INFO_RESPOND),
       STATE_MACHINE_INIT(boot_update_wait_respond, boot_update_file_data_respond_finish, BOOT_MASTER_WAIT_FILE_DATA_RESPOND),
       STATE_MACHINE_INIT(boot_update_wait_respond, boot_update_crc_check_respond_finish, BOOT_MASTER_WAIT_CRC_CHECK_RESPOND),
       STATE_MACHINE_INIT(boot_update_wait_respond, boot_update_crc_check_respond_finish, BOOT_MASTER_WAIT_JUMP_APP)};

state_machine_t *current_step = boot_state_machine_map;

void boot_master_poll(void)
{
    if (current_step != NULL)
    {
        if (current_step->func)
        {
            current_step->func(current_step, STATE_MACHINE_EVENT_RUN);
        }
    }
}

void switch_state_machine(uint32_t step, uint32_t offset)
{
    for (uint i = 0; i < ARRAY_LEN(boot_state_machine_map); i++)
    {
        if (boot_state_machine_map[i].step == step)
        {
            current_step = &boot_state_machine_map[i];
            if (current_step->func)
            {
                current_step->offset = offset;
                current_step->func(current_step, STATE_MACHINE_EVENT_ENTRY);
            }
            return;
        }
    }
}

void boot_update_idle(state_machine_t *self, state_machine_event_e event)
{
    (void)self;
    (void)event;
}

void msg_send(uint32_t msg_id, uint8_t *data, uint16_t len)
{
    msg_send_id       = msg_id;
    msg_send_buff_len = len;
    memcpy(msg_send_buff, data, len);
    boot_send(msg_send_id, msg_send_buff, msg_send_buff_len);
}

void boot_update_wait_respond(state_machine_t *self, state_machine_event_e event)
{
    if (event == STATE_MACHINE_EVENT_ENTRY)
    {
        self->over_tick  = sys_get_ms();
        self->resend_cnt = 0;
        return;
    }
    if (sys_time_is_elapsed(self->over_tick, self->over_time))
    {
        self->over_tick = sys_get_ms();
        self->resend_cnt++;
        if (self->resend_cnt > self->resend_cnt_max)
        {
            // 移除所有不响应的节点
            remove_update_node_diff(&update_root, self->step, self->offset);
            if (rt_slist_isempty(&update_root) || self->finish_handle == NULL)
            {
                switch_state_machine(BOOT_MASTER_IDLE, 0); // 升级失败
                boot_update_failed();
            }
            else
            {
                // 切换到下一个状态
                self->finish_handle();
            }
        }
        else
        {
            // 重发
            boot_send(msg_send_id, msg_send_buff, msg_send_buff_len);
            BOOT_MASTER_DBG("step %d, offset %d, resend %d , tick %llu, time %llu",
                            self->step,
                            self->offset,
                            self->resend_cnt,
                            self->over_tick,
                            self->over_time);
        }
    }
}

void boot_send_update_cmd(cmd_update_t *handle)
{
    clr_update_root(&update_root);
    for (uint16_t i = 0; i < handle->obj_num; i++)
    {
        add_update_node(&update_root, handle->obj_addr[i]);
    }
    memcpy(&cmd_update, handle, sizeof(cmd_update_t));
    msg_send(BOOT_MSG_ID_UPDATE_CMD,
             (uint8_t *)&cmd_update,
             cmd_update.obj_num + 2); // send len = cmd:1 + obj_num:1 + addr_size:n
}

void boot_master_update_satrt(cmd_update_t *cmd_update, app_info_t *app_info)
{
    update_process = 0;
    boot_send_update_cmd(cmd_update);
    memcpy(&file_info, app_info, sizeof(file_info));
    switch_state_machine(BOOT_MASTER_WAIT_CMD_RESPOND, 0);
}

void boot_update_cmd_respond_finish(void)
{
    switch_state_machine(BOOT_MASTER_WAIT_INFO_RESPOND, 0);
    msg_send(BOOT_MSG_ID_FILE_INFO, (uint8_t *)&file_info, sizeof(file_info));
}

void boot_update_info_respond_finish(void)
{
    switch_state_machine(BOOT_MASTER_WAIT_FILE_DATA_RESPOND, 0);
    file_data.offset   = 0;
    file_data.data_len = get_file_data(file_data.offset, file_data.data, FILE_DATA_RECV_MAX_SIZE);
    if (file_data.data_len > 0)
    {
        msg_send(BOOT_MSG_ID_FILE_DATA, (uint8_t *)&file_data, sizeof(file_data));
        switch_state_machine(BOOT_MASTER_WAIT_FILE_DATA_RESPOND, 0);
    }
    else
    {
        uint32_t magic_num = BOOT_MAGIC_NUM;
        msg_send(BOOT_MSG_ID_CHECK_CRC, (uint8_t *)&magic_num, sizeof(magic_num));
        switch_state_machine(BOOT_MASTER_WAIT_CRC_CHECK_RESPOND, 0);
    }
}

void boot_update_file_data_respond_finish(void)
{
    file_data.offset += file_data.data_len;
    file_data.data_len = get_file_data(file_data.offset, file_data.data, FILE_DATA_RECV_MAX_SIZE);
    if (file_data.data_len > 0)
    {
        update_process = 100.0 * file_data.offset / file_info.app_size;
        msg_send(BOOT_MSG_ID_FILE_DATA, (uint8_t *)&file_data, sizeof(file_data));
        switch_state_machine(BOOT_MASTER_WAIT_FILE_DATA_RESPOND, file_data.offset);
    }
    else
    {
        uint32_t magic_num = BOOT_MAGIC_NUM;
        msg_send(BOOT_MSG_ID_CHECK_CRC, (uint8_t *)&magic_num, sizeof(magic_num));
        switch_state_machine(BOOT_MASTER_WAIT_CRC_CHECK_RESPOND, 0);
    }
}

void boot_update_crc_check_respond_finish(void)
{
    update_process = 100.0;
    switch_state_machine(BOOT_MASTER_IDLE, 0);
    uint32_t magic_num = BOOT_MAGIC_NUM;
    msg_send(BOOT_MSG_ID_JUMP_APP, (uint8_t *)&magic_num, sizeof(magic_num));
}

void boot_recv_update_cmd_respond(uint8_t src_addr, uint8_t ret)
{
    if (current_step->step == BOOT_MASTER_WAIT_CMD_RESPOND)
    {
        if (ret == BOOT_UPDATE_RESPONE_OK)
        {
            set_update_node_process(&update_root, src_addr, BOOT_MASTER_WAIT_CMD_RESPOND, 0);
        }
        if (judge_update_node_is_same(&update_root, BOOT_MASTER_WAIT_CMD_RESPOND, 0))
        {
            boot_update_cmd_respond_finish();
        }
    }
}

void boot_recv_file_info_respond(uint8_t src_addr, uint8_t ret)
{
    if (current_step->step == BOOT_MASTER_WAIT_INFO_RESPOND)
    {
        if (ret == BOOT_UPDATE_RESPONE_OK)
        {
            set_update_node_process(&update_root, src_addr, BOOT_MASTER_WAIT_INFO_RESPOND, 0);
        }
        if (judge_update_node_is_same(&update_root, BOOT_MASTER_WAIT_INFO_RESPOND, 0))
        {
            boot_update_info_respond_finish();
        }
    }
}

void boot_recv_file_data_respond(uint8_t src_addr, respone_file_data_t *ret)
{
    if (current_step->step == BOOT_MASTER_WAIT_FILE_DATA_RESPOND)
    {
        if (ret->offset == file_data.offset)
        {
            set_update_node_process(&update_root, src_addr, BOOT_MASTER_WAIT_FILE_DATA_RESPOND, file_data.offset);
        }
        if (judge_update_node_is_same(&update_root, BOOT_MASTER_WAIT_FILE_DATA_RESPOND, file_data.offset))
        {
            boot_update_file_data_respond_finish();
        }
    }
}

void boot_recv_crc_check_respond(uint8_t src_addr, uint8_t ret)
{
    (void)ret;
    if (current_step->step == BOOT_MASTER_WAIT_CRC_CHECK_RESPOND)
    {
        // if (ret == BOOT_UPDATE_RESPONE_OK)
        {
            set_update_node_process(&update_root, src_addr, BOOT_MASTER_WAIT_CRC_CHECK_RESPOND, 0);
        }
        if (judge_update_node_is_same(&update_root, BOOT_MASTER_WAIT_CRC_CHECK_RESPOND, 0))
        {
            boot_update_crc_check_respond_finish();
        }
    }
}

void boot_recv_boot_heartbeat(uint8_t src_addr)
{
    (void)src_addr;
}

void boot_recv_app_heartbeat(uint8_t src_addr)
{
    (void)src_addr;
}

float get_update_process(void)
{
    return update_process;
}

void enter_boot(uint8_t *obj_addr, uint8_t obj_num)
{
    cmd_update_t cmd_update;
    cmd_update.update_cmd = BOOT_UPDATE_CMD;
    cmd_update.obj_num    = obj_num;
    for (int i = 0; i < cmd_update.obj_num; i++)
    {
        cmd_update.obj_addr[i] = obj_addr[i];
    }
    boot_send(BOOT_MSG_ID_UPDATE_CMD,
              (uint8_t *)&cmd_update,
              cmd_update.obj_num + 2); // send len = cmd:1 + obj_num:1 + addr_size:n
}

void enter_app(void)
{
    uint32_t magic_num = BOOT_MAGIC_NUM;
    boot_send(BOOT_MSG_ID_JUMP_APP, (uint8_t *)&magic_num, sizeof(magic_num));
}

void app_info_request(void)
{
    uint32_t magic_num = BOOT_MAGIC_NUM;
    boot_send(BOOT_MSG_ID_READ_APP_INFO, (uint8_t *)&magic_num, sizeof(magic_num));
}
