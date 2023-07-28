#include "gourp_update.h"
#include "comm_j1939.h"
GroupUpdate::GroupUpdate()
{
    connect(&m_timer, &QTimer::timeout, this, &GroupUpdate::slot_timeout);
    connect(J1939Ins, &CommJ1939::sig_recv_pgn_handle, this, &GroupUpdate::slot_recv_pgn_handle);
}

void GroupUpdate::slot_recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> data)
{
    if (pgn == BOOT_MSG_ID_READ_UPDATE_INFO_RESPONE)
    {
        memcpy(&m_update_info, &data[0], sizeof(update_info_t));
        if (m_update_info.obj_num == 0 || m_update_info.process >= 100)
        {
            stop();
        }

        QString s
            = QString("recv %3 update info obj num %1 process %2").arg(m_update_info.obj_num).arg(m_update_info.process).arg(src);
        emit this->sig_dbg_info(s);
        emit this->sig_update_state(m_update_info);
    }
    else if (pgn == BOOT_MSG_ID_START_UPDATE_RESPONE)
    {
        if (data[0] == BOOT_UPDATE_RESPONE_OK)
        {
            m_timer.start(1000);
        }
        else
        {
            QString s = QString("地址 %1 开始更新失败，固件校验失败").arg(src);
            emit this->sig_dbg_info(s);
        }
    }
}

int GroupUpdate::send(uint32_t msg_id, uint8_t *data, uint16_t len)
{
    QVector<uint8_t> array(len);
    memcpy(&array[0], data, len);
    J1939Ins->sig_msg_send(msg_id, 3, 0xFF, array, 100);
    return len;
}

void GroupUpdate::start(QVector<uint8_t> array)
{
    update_info_t info;
    info.process = 0;
    info.obj_num = array.size();
    for (int i = 0; i < info.obj_num; i++)
    {
        info.obj_addr[i] = array[i];
    }
    send(BOOT_MSG_ID_START_UPDATE, (uint8_t *)&info, info.obj_num + 2);
}

void GroupUpdate::stop()
{
    m_timer.stop();
}

void GroupUpdate::slot_timeout()
{
    request_update_info();
}

void GroupUpdate::request_update_info()
{
    uint8_t cmd = 1;

    send(BOOT_MSG_ID_READ_UPDATE_INFO, (uint8_t *)&cmd, 1);
}
