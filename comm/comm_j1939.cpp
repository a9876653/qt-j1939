#include "comm_j1939.h"
#include "QDateTime"
#include "pgn.h"

int j1939_can_write(uint32_t id, uint8_t *data, uint8_t len)
{
    QVector<uint8_t> array(len);
    memcpy(&array[0], data, len);
    return J1939Ins->can_write(id, array);
}

CommJ1939::CommJ1939()
{
    comm_thread = new MThreadPeriodTask(std::bind(&CommJ1939::poll, this));
    can_dev     = new CtrlCan();
    // can_dev = new ZlgCan();
    can_dev->moveToThread(comm_thread);
    this->moveToThread(comm_thread);
    init();

    connect(this, &CommJ1939::sig_msg_send, this, &CommJ1939::msg_send);
    connect(can_dev, &CanBase::sig_open_finish, this, &CommJ1939::sig_open_finish);
    comm_thread->start();
}

CommJ1939::~CommJ1939()
{
    delete can_dev;
    delete comm_thread;
}

bool CommJ1939::is_open()
{
    return can_dev->is_open();
}

bool CommJ1939::open_device(uint8_t device_index, uint8_t ch_index, uint32_t baudrate)
{
    return can_dev->open_device(device_index, ch_index, baudrate);
}

void CommJ1939::close_device()
{
    return can_dev->close_device();
}

void CommJ1939::can_recv_task()
{
    CanBase::can_farme_t frame;
    while (can_dev->recv_dequeue(frame))
    {
        j1939_receive_handle(&j1939_ins, frame.id, frame.data, frame.len);
    }
}

int CommJ1939::can_write(uint32_t id, QVector<uint8_t> array)
{
    if (can_dev->transmit(id, MSG_FLAG_EXT, array) == 1)
    {
        return array.size();
    }
    return array.size(); // 不考虑发送失败的情况
}

void CommJ1939::poll(void)
{
    can_recv_task();
    j1939_tp_poll(&j1939_ins);
    can_dev->can_task();
}

void CommJ1939::init()
{
    memset(&j1939_ins, 0, sizeof(j1939_ins));
    j1939_init(&j1939_ins, J1939_SRC_ADDR, PGN_REG_NUM, SESSION_REG_NUM, j1939_can_write);
}

int CommJ1939::msg_send(uint32_t pgn, uint8_t priority, uint8_t dst, QVector<uint8_t> array, uint32_t timeout)
{
    j1939_ret_e ret  = J1939_OK;
    int         len  = array.size();
    uint8_t    *data = (uint8_t *)&array[0];

    if (len > 8)
    {
        if (dst == ADDRESS_GLOBAL)
        {
            ret = j1939_tp_bam(&j1939_ins, pgn, priority, dst, data, len);
        }
        else
        {
            ret = j1939_tp(&j1939_ins, pgn, priority, j1939_ins.node_addr, dst, data, len);
        }
    }
    else
    {
        ret = j1939_send_msg(&j1939_ins, pgn, priority, dst, data, len, timeout);
    }
    if (ret == J1939_OK)
    {
        return len;
    }
    return 0;
}

void CommJ1939::set_src_addr(uint8_t addr)
{
    // 改变源地址的订阅哈希值
    j1939_ins.node_addr = addr;
}

void CommJ1939::set_dst_addr(uint8_t addr)
{
    dst_addr = addr;
}

uint8_t CommJ1939::get_src_addr()
{
    return j1939_ins.node_addr;
}

uint8_t CommJ1939::get_dst_addr()
{
    return dst_addr;
}

void CommJ1939::slot_msg_send(uint32_t pgn, QVector<uint8_t> array)
{
    msg_send(pgn, J1939_PRIORITY_DEFAULT, dst_addr, array, J1939_DEF_TIMEOUT);
}

void CommJ1939::recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> array)
{
    emit this->sig_recv_pgn_handle(pgn, src, array);
}

int CommJ1939::pgn_register(const uint32_t pgn, uint8_t code, pgn_callback_t cb)
{
    return j1939_pgn_register(&j1939_ins, pgn, code, cb);
}

int CommJ1939::session_cb_register(session_recv_fun recv_cb, session_err_fun err_cb)
{
    return j1939_tp_session_register_cb(&j1939_ins, recv_cb, err_cb);
}

void CommJ1939::slot_request_pgn(uint32_t pgn, uint8_t dst, uint16_t len)
{
    uint8_t data[5] = {
        PGN_SPECIFIC(pgn),
        PGN_FORMAT(pgn),
        PGN_DATA_PAGE(pgn),
        (uint8_t)(len & 0xFF),
        (uint8_t)((len & 0xFF00) >> 8),
    };
    j1939_send_msg(&j1939_ins, RAC, J1939_PRIORITY_DEFAULT, dst, data, sizeof(data), J1939_DEF_TIMEOUT);
}
