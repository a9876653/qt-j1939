#include "file_transfer_master_port.h"
#include <stdio.h>

#define FILE_PORT_DBG(x...) this->dbg_info(x)

#define J1939_DEF_PRIORITY  3
#define J1939_SEND_TIME_OUT 200

FileTransferMaster::FileTransferMaster()
{
    using namespace std::placeholders;
    m_file_transfer_if.send              = std::bind(&FileTransferMaster::send, this, _1, _2, _3);
    m_file_transfer_if.update_failed_cb  = std::bind(&FileTransferMaster::update_failed_cb, this, _1);
    m_file_transfer_if.get_file_data     = std::bind(&FileTransferMaster::get_file_data, this, _1, _2, _3);
    m_file_transfer_if.transfer_state_cb = std::bind(&FileTransferMaster::transfer_state_cb, this, _1, _2, _3);
    file_transfer_master_init(&m_file_transfer, &m_file_transfer_if);
    connect(J1939Ins, &CommJ1939::sig_recv_pgn_handle, this, &FileTransferMaster::slot_recv_pgn_handle, Qt::QueuedConnection);
}

FileTransferMaster::~FileTransferMaster()
{
}

void FileTransferMaster::transfer_task()
{
    file_transfer_master_task(&m_file_transfer);
}

void FileTransferMaster::slot_recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> array)
{
    if (pgn == FILE_MSG_ID_START_RESPONE || pgn == FILE_MSG_ID_FILE_INFO_RESPONE || pgn == FILE_MSG_ID_FILE_DATA_RESPONE
        || pgn == FILE_MSG_ID_CHECK_CRC_RESPONE)
    {
        file_master_recv_handle(&m_file_transfer, src, pgn, array.data(), array.length());
    }
}

int FileTransferMaster::send(uint32_t msg_id, uint8_t *data, uint16_t len)
{
    QVector<uint8_t> array(len);
    memcpy(&array[0], data, len);
    J1939Ins->sig_msg_send(msg_id, J1939_DEF_PRIORITY, m_dest_addr, array, J1939_SEND_TIME_OUT);
    return len;
}

void FileTransferMaster::update_failed_cb(uint8_t result)
{
    // MiddleSignalIns->update_failed();
    emit this->sig_update_failed(result);
}

uint32_t FileTransferMaster::get_file_data(uint32_t offset, uint8_t *data, uint32_t len)
{
    uint16_t ret_len     = 0;
    uint32_t file_len    = m_file_data_info.len;
    char *   file_buffer = m_file_data_info.buffer;
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

void FileTransferMaster::transfer_state_cb(uint8_t src, int result, uint32_t offset)
{
    FILE_PORT_DBG("src %d file transfer state %d, offset %d", src, result, offset);
    emit this->sig_update_state(src, result, offset);
}

static void qstring_to_char_array(QString str, char *buffer, uint16_t len)
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

static app_info_str_t get_app_info_str(app_info_t *info)
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

file_data_info_t *FileTransferMaster::set_file_data_info(QString file_path)
{
    QFile             file(file_path);
    QFileInfo         file_info(file_path);
    file_data_info_t *data_info = &m_file_data_info;
    char *            name      = data_info->name;
    uint16_t          name_size = sizeof(data_info->name);
    uint32_t          len       = 0;
    char *            buffer    = data_info->buffer;
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

int FileTransferMaster::start_transfer(uint8_t *obj_addr, uint8_t obj_num, file_data_info_t *data_info)
{
    file_info_t    info;
    file_process_t cmd;
    cmd.process = BOOT_UPDATE_CMD;
    cmd.obj_num = obj_num;
    for (int i = 0; i < cmd.obj_num; i++)
    {
        cmd.obj_addr[i] = obj_addr[i];
    }
    info.len = data_info->len;
    file_master_update_start(&m_file_transfer, &cmd, &info);
    FILE_PORT_DBG("file update start");
    return 0;
}

float FileTransferMaster::get_update_process()
{
    return m_file_transfer.update_process;
}

void FileTransferMaster::app_info_request()
{
}

void FileTransferMaster::send_handshake_cmd(uint8_t *obj_addr, uint8_t obj_num)
{
    file_process_t cmd;
    cmd.obj_num = obj_num;
    for (int i = 0; i < obj_num; i++)
    {
        cmd.obj_addr[i] = obj_addr[i];
    }
    file_master_send_update_cmd(&m_file_transfer, &cmd);
}

void FileTransferMaster::verify_request()
{
    file_master_check_crc_request(&m_file_transfer);
}

void FileTransferMaster::dbg_info(const char *msg, ...)
{
    char buff[512];
    memset(buff, 0, sizeof(buff));
    va_list args;
    va_start(args, msg);
    int len = vsnprintf(buff, sizeof(buff), msg, args);
    if (len > 0)
    {
        emit this->sig_dbg_info(QString(buff));
    }
}
