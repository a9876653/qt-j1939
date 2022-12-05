#include "boot_header.h"
#include "boot_master.h"
#include "boot_port.h"
#include "comm_j1939.h"
#include "QDebug"
#include "middle_signal.h"
using namespace CommJ1939;

#define BOOT_PORT_DBG(x...) MiddleSignalIns->dbg_info(x)

uint8_t dest_addr = ADDRESS_GLOBAL;
#define J1939_DEF_PRIORITY  3
#define J1939_SEND_TIME_OUT 200

void boot_send(uint32_t msg_id, uint8_t *data, uint16_t len)
{
    j1939_boot_msg_send(msg_id, J1939_DEF_PRIORITY, dest_addr, data, len, J1939_SEND_TIME_OUT);
}

void boot_update_failed(void)
{
    MiddleSignalIns->update_failed();
    BOOT_PORT_DBG("boot_update_failed");
}

j1939_ret_e boot_cmd_respond_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    boot_recv_update_cmd_respond(msg->src, msg->data[0]);
    BOOT_PORT_DBG("src %d cmd respond ok", msg->src);
    return J1939_OK;
}

j1939_ret_e boot_file_info_respond_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    boot_recv_file_info_respond(msg->src, msg->data[0]);
    BOOT_PORT_DBG("src %d file info respond ok", msg->src);
    return J1939_OK;
}

j1939_ret_e boot_file_data_respond_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    respone_file_data_t *ret = (respone_file_data_t *)msg->data;
    boot_recv_file_data_respond(msg->src, ret);
    BOOT_PORT_DBG("src %d file data respond offset %d", msg->src, ret->offset);
    return J1939_OK;
}

j1939_ret_e boot_check_crc_respond_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    boot_recv_crc_check_respond(msg->src, msg->data[0]);
    BOOT_PORT_DBG("src %d check crc respond %d", msg->src, msg->data[0]);
    return J1939_OK;
}

int j1939_update_cmd_get_data(uint32_t pgn, uint8_t src, uint8_t **data, uint16_t *len)
{
    (void)pgn;
    (void)src;
    static uint8_t temp_data[512];
    *data = temp_data;
    *len  = sizeof(temp_data);
    return *len;
}

void boot_port_poll(void)
{
    boot_master_poll();
}

#include "QFile"
#include "QFileDialog"

void qstring_to_char_array(QString str, char *buffer, uint16_t len)
{
    memset(buffer, 0, len);
    QByteArray array = str.toUtf8();
    for (uint16_t i = 0; i < array.count(); i++)
    {
        buffer[i] = array.at(i);
        if (i > len)
            return;
    }
}

file_data_info_t file_data_info;

app_info_str_t get_app_info_str(app_info_t *info)
{
    app_info_str_t str;

    QString board_ver = QString("V%1.%2.%3")
                            .arg((info->board_version & 0xFF000000) >> 24)
                            .arg((info->board_version & 0xFF0000) >> 16)
                            .arg((info->board_version & 0xFF00) >> 8);
    QString frimware_ver = QString("V%1.%2.%3")
                               .arg((info->firmware_version & 0xFF000000) >> 24)
                               .arg((info->firmware_version & 0xFF0000) >> 16)
                               .arg((info->firmware_version & 0xFF00) >> 8);

    str.appName     = QString(QLatin1String(info->app_name, 50));
    str.boardId     = QString("0x%1").arg(info->board_id, 0, 16);
    str.boardVer    = board_ver;
    str.firmwareVer = frimware_ver;
    str.buildTime   = QString(QLatin1String(info->build_time, 19));
    str.gitCommit   = QString(QLatin1String(info->app_git_commit, 40));
    str.size        = QString("%1").arg(info->app_size);
    str.startAddr   = QString("0x%1").arg(info->app_start_address, 0, 16);
    str.crc         = QString("0x%1").arg(info->app_crc, 0, 16);
    return str;
}

file_data_info_t *set_file_data_info(QString file_path)
{
    QFile             file(file_path);
    QFileInfo         file_info(file_path);
    file_data_info_t *data_info = &file_data_info;
    char             *name      = data_info->name;
    uint16_t          name_size = sizeof(data_info->name);
    uint32_t          len       = 0;
    char             *buffer    = data_info->buffer;
    qstring_to_char_array(file_info.fileName(), name, name_size);

    len = file_info.size();
    if (len < 512 + 4)
    {
        return data_info;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        return data_info;
    }

    file.read(buffer, len);
    file.close();

    // 文件长度不是4的倍数的，补零补够4的倍数
    if (len % 4 > 0)
    {
        len = len + (4 - (len % 4));
    }
    data_info->len    = len;
    uint32_t file_crc = crc32_cal((uint8_t *)&buffer[512], (len - 512) / 4);
    if (file_crc != 0)
    {
        QMessageBox msgBox(QMessageBox::Warning, "error", "固件crc校验出错", QMessageBox::Button::Ok, 0, 0);
        msgBox.exec();
    }
    data_info->app_info = get_app_info_str((app_info_t *)buffer);
    return data_info;
}

void boot_port_start(uint8_t *obj_addr, uint8_t obj_num, file_data_info_t *data_info)
{
    app_info_t   app_info;
    cmd_update_t cmd_update;
    cmd_update.update_cmd = BOOT_UPDATE_CMD;
    cmd_update.obj_num    = obj_num;
    for (int i = 0; i < cmd_update.obj_num; i++)
    {
        cmd_update.obj_addr[i] = obj_addr[i];
    }
    memcpy(&app_info, data_info->buffer, sizeof(app_info));
    boot_master_update_satrt(&cmd_update, &app_info);
    BOOT_PORT_DBG("boot update start");
}

uint32_t get_file_data(uint32_t offset, uint8_t *data, uint16_t len)
{
    uint16_t ret_len     = 0;
    uint32_t file_len    = file_data_info.len;
    char    *file_buffer = file_data_info.buffer;
    if (offset >= file_len)
    {
        ret_len = 0;
    }
    else if (file_len - offset > len)
    {
        ret_len = len;
    }
    else
    {
        ret_len = file_len - offset;
    }
    memcpy(data, file_buffer + offset, ret_len);
    return ret_len;
}

void j1939_update_cmd_recv_cb(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t len)
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

void boot_port_init(void)
{
    j1939_boot_tp_rx_register(0x20, 0x40, j1939_update_cmd_get_data, j1939_update_cmd_recv_cb, NULL);
    j1939_boot_pgn_register(BOOT_MSG_ID_UPDATE_CMD_RESPONE, 0, boot_cmd_respond_cb);
    j1939_boot_pgn_register(BOOT_MSG_ID_FILE_INFO_RESPONE, 0, boot_file_info_respond_cb);
    j1939_boot_pgn_register(BOOT_MSG_ID_FILE_DATA_RESPONE, 0, boot_file_data_respond_cb);
    j1939_boot_pgn_register(BOOT_MSG_ID_CHECK_CRC_RESPONE, 0, boot_check_crc_respond_cb);
}
