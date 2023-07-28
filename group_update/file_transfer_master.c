#include "file_transfer_master.h"
#include "boot_update_node.h"

#define LOG_TAG "file transfer"
#include "QDebug"

#define FILE_MASTER_DBG(x...) qDebug(x)

#define FILE_ASSERT(v) \
    if (v == NULL)     \
        return;

#define FILE_MASTER_IDLE_TIME                   0
#define FILE_MASTER_WAIT_CMD_RESPOND_TIME       30
#define FILE_MASTER_WAIT_INFO_RESPOND_TIME      3000
#define FILE_MASTER_WAIT_FILE_DATA_RESPOND_TIME 1000
#define FILE_MASTER_WAIT_CRC_CHECK_RESPOND_TIME 1000
#define FILE_MASTER_WAIT_JUMP_APP_TIME          1000

#define FILE_MASTER_IDLE_RESEND_CNT                   0
#define FILE_MASTER_WAIT_CMD_RESPOND_RESEND_CNT       100
#define FILE_MASTER_WAIT_INFO_RESPOND_RESEND_CNT      3
#define FILE_MASTER_WAIT_FILE_DATA_RESPOND_RESEND_CNT 3
#define FILE_MASTER_WAIT_CRC_CHECK_RESPOND_RESEND_CNT 3
#define FILE_MASTER_WAIT_JUMP_APP_RESEND_CNT          3

static void file_transfer_step_init(file_transfer_step_t *handle,
                                    uint8_t               step,
                                    uint32_t              over_time,
                                    uint32_t              resend_cnt,
                                    respond_finish_fun    func,
                                    void *                parent)
{
    memset(handle, 0, sizeof(file_transfer_step_t));
    handle->step           = step;
    handle->finish_handle  = func;
    handle->over_time      = over_time;
    handle->resend_cnt_max = resend_cnt;
    handle->parent         = parent;
}

static void file_transfer_master_msg_send(file_transfer_master_t *handle, uint32_t msg_id, uint8_t *data, uint16_t len)
{
    handle->msg_send_id       = msg_id;
    handle->msg_send_buff_len = len;
    memcpy(handle->msg_send_buff, data, len);
    handle->interface->send(handle->msg_send_id, handle->msg_send_buff, handle->msg_send_buff_len);
}

static void file_master_state_machine_switch(file_transfer_step_t *handle, uint8_t step, uint32_t offset)
{
    file_transfer_master_t *file_transfer = (file_transfer_master_t *)handle->parent;
    handle->offset                        = offset;
    file_transfer->cur_step               = switch_state_machine(&file_transfer->root, step);
}

static void file_master_update_check(file_transfer_master_t *handle)
{
    file_process_t *pro = &handle->file_process;
    if(handle->cur_step && handle->cur_step->step != FILE_MASTER_IDLE)
    {
        for (int i = 0; i < pro->obj_num; i++)
        {
            if (pro->obj_addr[i] != FILE_ADDR_INVAILD)
            {
                if (!update_node_is_exist(&handle->node_root, pro->obj_addr[i]))
                {
                    handle->interface->update_failed_cb(pro->obj_addr[i]);
                    pro->obj_addr[i] = FILE_ADDR_INVAILD;
                }
            }
        }
    }
}

static void file_master_idle(state_machine_t *self, state_machine_event_e event)
{
    (void)self;
    (void)event;
}

static void file_master_wait_respond(state_machine_t *self, state_machine_event_e event)
{
    file_transfer_step_t *  handle        = (file_transfer_step_t *)self->args;
    file_transfer_master_t *file_transfer = (file_transfer_master_t *)handle->parent;
    if (event == STATE_MACHINE_EVENT_ENTRY)
    {
        handle->over_tick  = sys_get_ms();
        handle->resend_cnt = 0;
        return;
    }
    if (sys_time_is_elapsed(handle->over_tick, handle->over_time))
    {
        handle->over_tick = sys_get_ms();
        handle->resend_cnt++;
        if (handle->resend_cnt > handle->resend_cnt_max)
        {
            // 移除所有不响应的节点
            remove_update_node_diff(&file_transfer->node_root, handle->step, handle->offset);
            file_master_update_check(file_transfer);
            if (rt_slist_isempty(&file_transfer->node_root) || handle->finish_handle == NULL)
            {
                file_master_state_machine_switch(handle, FILE_MASTER_IDLE, 0); // 升级失败
                file_transfer->interface->update_failed_cb(FILE_ADDR_INVAILD);
            }
            else
            {
                // 切换到下一个状态
                handle->finish_handle(handle);
            }
        }
        else
        {
            // 重发
            file_transfer->interface->send(file_transfer->msg_send_id,
                                           file_transfer->msg_send_buff,
                                           file_transfer->msg_send_buff_len);

            FILE_MASTER_DBG("step %d, offset %d, resend %d , tick %llu, time %llu",
                            handle->step,
                            handle->offset,
                            handle->resend_cnt,
                            handle->over_tick,
                            handle->over_time);
        }
    }
}

static void file_master_cmd_respond_finish(file_transfer_step_t *handle)
{
    file_transfer_master_t *file_transfer = (file_transfer_master_t *)handle->parent;
    file_master_state_machine_switch(handle, FILE_MASTER_WAIT_INFO_RESPOND, 0);
    file_transfer_master_msg_send(file_transfer,
                                  FILE_MSG_ID_FILE_INFO,
                                  (uint8_t *)&file_transfer->file_info,
                                  sizeof(file_info_t));
}
static void file_master_info_respond_finish(file_transfer_step_t *handle)
{
    file_transfer_master_t *file_transfer = (file_transfer_master_t *)handle->parent;

    file_transfer->file_data.offset   = 0;
    file_transfer->file_data.data_len = file_transfer->interface->get_file_data(file_transfer->file_data.offset,
                                                                                file_transfer->file_data.data,
                                                                                FILE_DATA_RECV_MAX_SIZE);
    if (file_transfer->file_data.data_len > 0)
    {
        file_transfer_master_msg_send(file_transfer,
                                      FILE_MSG_ID_FILE_DATA,
                                      (uint8_t *)&file_transfer->file_data,
                                      sizeof(file_data_t));
        file_master_state_machine_switch(handle, FILE_MASTER_WAIT_FILE_DATA_RESPOND, 0);
    }
    else
    {
        uint32_t magic_num = BOOT_MAGIC_NUM;
        file_transfer_master_msg_send(file_transfer, FILE_MSG_ID_CHECK_CRC, (uint8_t *)&magic_num, sizeof(magic_num));
        file_master_state_machine_switch(handle, FILE_MASTER_IDLE, 0);
    }
}
static void file_master_data_respond_finish(file_transfer_step_t *handle)
{
    file_transfer_master_t *file_transfer = (file_transfer_master_t *)handle->parent;
    file_data_t *           file_data     = &file_transfer->file_data;
    file_info_t *           file_info     = &file_transfer->file_info;
    file_data->offset += file_data->data_len;
    file_data->data_len = file_transfer->interface->get_file_data(file_data->offset, file_data->data, FILE_DATA_RECV_MAX_SIZE);
    if (file_data->data_len > 0)
    {
        file_transfer->update_process = 100.0 * file_data->offset / file_info->len;

        file_transfer_master_msg_send(file_transfer, FILE_MSG_ID_FILE_DATA, (uint8_t *)file_data, file_data->data_len + 6);
        file_master_state_machine_switch(handle, FILE_MASTER_WAIT_FILE_DATA_RESPOND, file_data->offset);
    }
    else
    {
        uint32_t magic_num = BOOT_MAGIC_NUM;
        file_transfer_master_msg_send(file_transfer, FILE_MSG_ID_CHECK_CRC, (uint8_t *)&magic_num, sizeof(magic_num));
        file_master_state_machine_switch(handle, FILE_MASTER_WAIT_CRC_CHECK_RESPOND, 0);
    }
}
static void file_master_crc_check_respond_finish(file_transfer_step_t *handle)
{
    file_transfer_master_t *file_transfer = (file_transfer_master_t *)handle->parent;
    file_transfer->update_process         = 100.0;

    file_master_state_machine_switch(handle, FILE_MASTER_IDLE, 0);
}

static void file_master_recv_update_cmd_respond(file_transfer_master_t *file_transfer, uint8_t src_addr, uint8_t ret)
{
    if (file_transfer->cur_step->step == FILE_MASTER_WAIT_CMD_RESPOND)
    {
        if (ret == FILE_RESPONE_OK)
        {
            set_update_node_process(&file_transfer->node_root, src_addr, FILE_MASTER_WAIT_CMD_RESPOND, 0);
        }

        if (judge_update_node_is_same(&file_transfer->node_root, FILE_MASTER_WAIT_CMD_RESPOND, 0))
        {
            file_master_cmd_respond_finish(&file_transfer->transfer_step[FILE_MASTER_WAIT_CMD_RESPOND]);
        }
    }

    file_transfer->interface->transfer_state_cb(src_addr, FILE_TRANSFER_START, 0);

    FILE_MASTER_DBG("src %d recv start %d", ret);
}

static void file_master_recv_file_info_respond(file_transfer_master_t *file_transfer, uint8_t src_addr, uint8_t ret)
{
    if (file_transfer->cur_step->step == FILE_MASTER_WAIT_INFO_RESPOND)
    {
        if (ret == BOOT_UPDATE_RESPONE_OK)
        {
            set_update_node_process(&file_transfer->node_root, src_addr, FILE_MASTER_WAIT_INFO_RESPOND, 0);
        }
        if (judge_update_node_is_same(&file_transfer->node_root, FILE_MASTER_WAIT_INFO_RESPOND, 0))
        {
            file_master_info_respond_finish(&file_transfer->transfer_step[FILE_MASTER_WAIT_INFO_RESPOND]);
        }
    }
    file_transfer->interface->transfer_state_cb(src_addr, FILE_TRANSFER_INFO, 0);

    FILE_MASTER_DBG("src %d recv info %d", ret);
}

static void file_master_recv_file_data_respond(file_transfer_master_t *file_transfer, uint8_t src_addr, respone_file_data_t *ret)
{
    if (file_transfer->cur_step->step == FILE_MASTER_WAIT_FILE_DATA_RESPOND)
    {
        file_data_t *file_data = &file_transfer->file_data;
        if (ret->offset == file_data->offset)
        {
            set_update_node_process(&file_transfer->node_root, src_addr, FILE_MASTER_WAIT_FILE_DATA_RESPOND, file_data->offset);
        }
        if (judge_update_node_is_same(&file_transfer->node_root, FILE_MASTER_WAIT_FILE_DATA_RESPOND, file_data->offset))
        {
            file_master_data_respond_finish(&file_transfer->transfer_step[FILE_MASTER_WAIT_FILE_DATA_RESPOND]);
        }
    }
    file_transfer->interface->transfer_state_cb(src_addr, FILE_TRANSFER_DATA, ret->offset);

    FILE_MASTER_DBG("src %d recv data %d", ret->offset);
}

static void file_master_recv_crc_check_respond(file_transfer_master_t *file_transfer, uint8_t src_addr, uint8_t ret)
{
    (void)ret;
    if (file_transfer->cur_step->step == FILE_MASTER_WAIT_CRC_CHECK_RESPOND)
    {
        if (ret == BOOT_UPDATE_RESPONE_OK)
        {
            set_update_node_process(&file_transfer->node_root, src_addr, FILE_MASTER_WAIT_CRC_CHECK_RESPOND, 0);
        }
        else
        {
            remove_update_node(&file_transfer->node_root, src_addr);
        }

        if (judge_update_node_is_same(&file_transfer->node_root, FILE_MASTER_WAIT_CRC_CHECK_RESPOND, 0))
        {
            file_master_crc_check_respond_finish(&file_transfer->transfer_step[FILE_MASTER_WAIT_CRC_CHECK_RESPOND]);
        }
    }
    if (ret == BOOT_UPDATE_RESPONE_OK)
    {
        file_transfer->interface->transfer_state_cb(src_addr, FILE_TRANSFER_VERIFY, 0);
    }
    else
    {
        file_transfer->interface->transfer_state_cb(src_addr, -FILE_TRANSFER_VERIFY, 0);
    }
}

void file_transfer_master_init(file_transfer_master_t *handle, file_master_if_t *interface)
{
    handle->root.next      = NULL;
    handle->node_root.next = NULL;

    int i = 0;
#define FILE_STEP_INIT(s, fun) file_transfer_step_init(&handle->transfer_step[i++], s, s##_TIME, s##_RESEND_CNT, fun, handle);

    FILE_STEP_INIT(FILE_MASTER_IDLE, NULL);
    FILE_STEP_INIT(FILE_MASTER_WAIT_CMD_RESPOND, file_master_cmd_respond_finish);
    FILE_STEP_INIT(FILE_MASTER_WAIT_INFO_RESPOND, file_master_info_respond_finish);
    FILE_STEP_INIT(FILE_MASTER_WAIT_FILE_DATA_RESPOND, file_master_data_respond_finish);
    FILE_STEP_INIT(FILE_MASTER_WAIT_CRC_CHECK_RESPOND, file_master_crc_check_respond_finish);

    i = 0;
    state_machine_append(&handle->root, FILE_MASTER_IDLE, &handle->transfer_step[i++], file_master_idle);
    state_machine_append(&handle->root, FILE_MASTER_WAIT_CMD_RESPOND, &handle->transfer_step[i++], file_master_wait_respond);
    state_machine_append(&handle->root, FILE_MASTER_WAIT_INFO_RESPOND, &handle->transfer_step[i++], file_master_wait_respond);
    state_machine_append(&handle->root,
                         FILE_MASTER_WAIT_FILE_DATA_RESPOND,
                         &handle->transfer_step[i++],
                         file_master_wait_respond);
    state_machine_append(&handle->root,
                         FILE_MASTER_WAIT_CRC_CHECK_RESPOND,
                         &handle->transfer_step[i++],
                         file_master_wait_respond);

    handle->interface = interface;
    handle->cur_step  = switch_state_machine(&handle->root, FILE_MASTER_IDLE);
}

void file_transfer_master_task(file_transfer_master_t *handle)
{
    FILE_ASSERT(handle);
    FILE_ASSERT(handle->interface);
    FILE_ASSERT(handle->interface->get_file_data);
    FILE_ASSERT(handle->interface->send);
    FILE_ASSERT(handle->interface->transfer_state_cb);
    FILE_ASSERT(handle->interface->update_failed_cb);

    state_machine_run(handle->cur_step);
}

int file_master_recv_handle(file_transfer_master_t *handle, uint8_t src, uint32_t msg_id, uint8_t *data, uint32_t len)
{
    (void)len;
    switch (msg_id)
    {
    case FILE_MSG_ID_START_RESPONE:
        file_master_recv_update_cmd_respond(handle, src, data[0]);
        break;
    case FILE_MSG_ID_FILE_INFO_RESPONE:
        file_master_recv_file_info_respond(handle, src, data[0]);
        break;
    case FILE_MSG_ID_FILE_DATA_RESPONE:
        file_master_recv_file_data_respond(handle, src, (respone_file_data_t *)data);
        break;
    case FILE_MSG_ID_CHECK_CRC_RESPONE:
        file_master_recv_crc_check_respond(handle, src, data[0]);
        break;
    };
    return 0;
}

void file_master_send_update_cmd(file_transfer_master_t *handle, file_process_t *cmd)
{
    file_transfer_master_msg_send(handle,
                                  FILE_MSG_ID_START,
                                  (uint8_t *)cmd,
                                  cmd->obj_num + 2); // send len = cmd:1 + obj_num:1 + addr_size:n
}

void file_master_update_start(file_transfer_master_t *handle, file_process_t *cmd, file_info_t *file_info)
{
    clr_update_root(&handle->node_root);
    for (uint16_t i = 0; i < cmd->obj_num; i++)
    {
        add_update_node(&handle->node_root, cmd->obj_addr[i]);
    }
    memcpy(&handle->file_process, cmd, sizeof(file_process_t));
    handle->file_process.process = 0;
    handle->file_info.len        = file_info->len;
    file_master_send_update_cmd(handle, cmd);
    file_master_state_machine_switch(&handle->transfer_step[FILE_MASTER_WAIT_CMD_RESPOND], FILE_MASTER_WAIT_CMD_RESPOND, 0);
}



void file_master_check_crc_request(file_transfer_master_t *handle)
{
    uint32_t magic_num = BOOT_MAGIC_NUM;
    file_transfer_master_msg_send(handle, FILE_MSG_ID_CHECK_CRC, (uint8_t *)&magic_num, sizeof(magic_num));
}
