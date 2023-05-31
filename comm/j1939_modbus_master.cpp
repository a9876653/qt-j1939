#include "j1939_modbus_master.h"
#include "modbus_crc.h"
#include "utils.h"
#include "QDebug"
#include "canard_dsdl.h"

#define J1939_MODBUS_MASTER_DBG(x...) qDebug(x)

j1939_ret_e respond_read_reg_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    respond_read_reg_t *ptr = (respond_read_reg_t *)msg->data;
    J1939DbIns->recv_read_reg_handle(msg->src, ptr);
    return J1939_OK;
}

j1939_ret_e respond_write_reg_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    request_write_reg_t *ptr = (request_write_reg_t *)msg->data;
    J1939DbIns->recv_write_reg_handle(msg->src, ptr);
    return J1939_OK;
}

j1939_ret_e respond_write_mul_reg_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    respond_write_mul_reg_t *ptr = (respond_write_mul_reg_t *)msg->data;
    J1939DbIns->recv_write_mul_reg_handle(msg->src, ptr);
    return J1939_OK;
}

CommJ1939Db::CommJ1939Db()
{
    db_read_queue.set_max_size(200);
    request_read_thread = new MThread(std::bind(&CommJ1939Db::request_read_thread_task, this));
    request_read_thread->start();
}

void CommJ1939Db::init()
{
    J1939Ins->pgn_register(DB_FUNC_READ_HOLDING_REGISTER, 0, respond_read_reg_cb);
    J1939Ins->pgn_register(DB_FUNC_WRITE_REGISTER, 0, respond_write_reg_cb);
    J1939Ins->pgn_register(DB_FUNC_WRITE_MULTIPLE_REGISTERS, 0, respond_write_mul_reg_cb);
}

CommDbValue *CommJ1939Db::db_reg_register(uint16_t src_addr, uint16_t reg_addr, uint16_t reg_size)
{
    CommDbValue *reg     = nullptr;
    uint32_t     map_key = (src_addr << 16) | reg_addr;
    if (db_map.contains(map_key))
    {
        reg = db_map.value(map_key);
    }
    else
    {
        reg = new CommDbValue(reg_addr, reg_size);
        db_map.insert(map_key, reg);
    }

    return reg;
}

int CommJ1939Db::msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, QByteArray array)
{
    return J1939Ins->sig_msg_send(pgn, priority, dst, array, J1939_DEF_TIMEOUT);
}

void CommJ1939Db::slot_request_dst_read_reg(uint16_t dst, uint16_t reg_addr, uint16_t reg_len)
{
    db_request_read_t request_read;
    request_read.dst      = dst;
    request_read.reg_addr = reg_addr;
    request_read.reg_len  = reg_len;
    db_read_queue.enqueue(request_read);
}

void CommJ1939Db::slot_request_dst_write_reg(uint16_t dst, uint16_t reg_addr, uint16_t reg_value)
{
    static uint8_t       request_buff[sizeof(request_write_reg_t)];
    request_write_reg_t *ptr = (request_write_reg_t *)request_buff;
    ptr->reg_addr            = reg_addr;
    ptr->reg_value           = reg_value;
    msg_send(DB_FUNC_WRITE_REGISTER,
             J1939_PRIORITY_DEFAULT,
             dst,
             QByteArray((const char *)request_buff, sizeof(request_write_reg_t)));
}

void CommJ1939Db::slot_request_dst_write_mul_reg(uint16_t dst, uint16_t reg_addr, QVector<uint16_t> array)
{
    static uint8_t           request_buff[sizeof(request_write_mul_reg_t)];
    uint16_t                 reg_len  = array.count();
    request_write_mul_reg_t *ptr      = (request_write_mul_reg_t *)request_buff;
    uint8_t                 *put_data = ptr->data;
    ptr->reg_addr                     = reg_addr;
    ptr->reg_len                      = LW_MIN(reg_len, MB_REG_LEN_MAX);
    for (uint16_t i = 0; i < reg_len; i++)
    {
        *put_data++ = array[i] >> 8;
        *put_data++ = array[i] & 0xFF;
    }
    uint16_t crc_offset        = ptr->reg_len * 2 + 4;
    uint16_t crc               = modbus_crc16((uint8_t *)ptr, crc_offset);
    request_buff[crc_offset++] = crc & 0xFF;
    request_buff[crc_offset++] = crc >> 8;
    msg_send(DB_FUNC_WRITE_MULTIPLE_REGISTERS, J1939_PRIORITY_DEFAULT, dst, QByteArray((const char *)request_buff, crc_offset));
}

void CommJ1939Db::slot_request_read_reg(uint16_t reg_addr, uint16_t reg_len)
{
    int r_index = 0;

    while (r_index < reg_len)
    {
        int r_len = qMin(64, reg_len - r_index);
        slot_request_dst_read_reg(J1939Ins->get_dst_addr(), reg_addr + r_index, r_len);
        r_index += r_len;
    }
}
void CommJ1939Db::slot_request_write_reg(uint16_t reg_addr, QVector<uint16_t> array)
{
    uint16_t  reg_len = array.size();
    uint16_t *data    = &array[0];
    if (reg_len == 1)
    {
        slot_request_dst_write_reg(J1939Ins->get_dst_addr(), reg_addr, L2B16(*data));
    }
    else
    {
        int w_index = 0;

        while (w_index < reg_len)
        {
            int               w_len = qMin(64, reg_len - w_index);
            QVector<uint16_t> vec(w_len);
            for (int i = 0; i < w_len; i++)
            {
                vec[i] = data[w_index];
            }
            slot_request_dst_write_mul_reg(J1939Ins->get_dst_addr(), reg_addr + w_index, vec);
            w_index += w_len;
        }
    }
}

void CommJ1939Db::recv_read_reg_handle(uint16_t src_addr, respond_read_reg_t *ptr)
{
    uint8_t *respond_buff = (uint8_t *)ptr;
    uint8_t  reg_len      = ptr->data_len / 2;
    uint8_t *data         = ptr->data;
    uint8_t  data_len     = ptr->data_len;
    uint16_t reg_addr     = ptr->reg_addr;
    uint16_t crc_offset   = qMin(reg_len * 2 + 3 + 2, 256);

    uint16_t crc = modbus_crc16((uint8_t *)respond_buff, crc_offset);
    if (crc != 0)
    {
        return;
    }
    for (int i = 0; i < reg_len; i++)
    {
        uint32_t map_key = (src_addr << 16) | (reg_addr + i);
        if (db_map.contains(map_key))
        {
            CommDbValue *db    = db_map.value(map_key);
            uint32_t     value = 0;
            if (db->reg_size == 1)
            {
                value = canardDSDLGetU16(data, data_len, i * 16, 16);
                value = L2B16(value);
            }
            else if (db->reg_size == 2)
            {
                uint16_t u16_v = canardDSDLGetU32(data, data_len, i * 16, 16);
                u16_v          = L2B16(u16_v);
                value          = u16_v;
                u16_v          = canardDSDLGetU32(data, data_len, i * 16 + 16, 16);
                u16_v          = L2B16(u16_v);
                value |= u16_v << 16;
                // value = L2B32(value);
                i++;
            }

            db->sig_read_finish(value);
        }
    }
    J1939_MODBUS_MASTER_DBG("read addr %d start reg %d (0x%04x) reg len %d v (0x%04x) successful",
                            src_addr,
                            reg_addr,
                            reg_addr,
                            ptr->data_len / 2,
                            *(uint16_t *)ptr->data);

    emit sig_recv_read_reg(ptr->reg_addr, QByteArray((const char *)ptr->data, ptr->data_len));
    read_wait_sem.release();
}
void CommJ1939Db::recv_write_reg_handle(uint16_t src_addr, request_write_reg_t *ptr)
{
    uint16_t reg_addr = ptr->reg_addr;
    uint32_t map_key  = (src_addr << 16) | reg_addr;
    if (db_map.contains(map_key))
    {
        CommDbValue *db = db_map.value(map_key);
        J1939_MODBUS_MASTER_DBG("write addr %d reg %d (0x%04x) value %d successful",
                                src_addr,
                                reg_addr,
                                reg_addr,
                                ptr->reg_value);
        db->sig_write_finish();
    }
    emit sig_recv_write_reg(ptr->reg_addr, ptr->reg_value);
}
void CommJ1939Db::recv_write_mul_reg_handle(uint16_t src_addr, respond_write_mul_reg_t *ptr)
{
    uint16_t reg_addr = ptr->reg_addr;
    uint8_t  reg_len  = ptr->reg_len;
    for (int i = 0; i < reg_len; i++)
    {
        uint32_t map_key = (src_addr << 16) | reg_addr;
        if (db_map.contains(map_key))
        {
            CommDbValue *db = db_map.value(map_key);
            J1939_MODBUS_MASTER_DBG("write addr %d reg 0x%04x len %d successful", src_addr, reg_addr, reg_len);
            db->sig_write_finish();
        }
    }
    emit sig_recv_write_mul_reg(ptr->reg_addr, ptr->reg_len);
}

void CommJ1939Db::request_read_thread_task()
{
    db_request_read_t   request_read = db_read_queue.dequeue();
    static uint8_t      request_buff[sizeof(request_read_reg_t)];
    request_read_reg_t *ptr = (request_read_reg_t *)request_buff;
    ptr->reg_addr           = request_read.reg_addr;
    ptr->reg_len            = request_read.reg_len;
    msg_send(DB_FUNC_READ_HOLDING_REGISTER,
             J1939_PRIORITY_DEFAULT,
             request_read.dst,
             QByteArray((const char *)request_buff, sizeof(request_read_reg_t)));
    read_wait_sem.tryAcquire(1, 100);
}
