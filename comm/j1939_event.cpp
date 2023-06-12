#include "j1939_event.h"
#include <QMap>

static QMap<int, J1939Event *> j1939_event_map;
#define EVENT_TYPE_NUM 2
const uint32_t magic_num_map[] = {
    READ_ERR_EVENT_MAGIC,
    READ_INFO_EVENT_MAGIC,
};

j1939_ret_e respond_read_event_cnt_cb(j1939_t *handle, j1939_message_t *msg)
{
    (void)handle;
    read_event_cnt_respond_t *ptr = (read_event_cnt_respond_t *)msg->data;

    J1939Event *event = get_j1939_event_ins(msg->src);
    event->sig_recv_event_cnt(ptr->cnt);
    qDebug("respond_read_event_cnt %d", ptr->cnt);
    return J1939_OK;
}

void j1939_recv_read_event(int src, read_event_respond_t *respond)
{
    J1939Event *event = get_j1939_event_ins(src);
    event->sig_recv_event(*respond);
}

J1939Event *get_j1939_event_ins(int src)
{
    if (j1939_event_map.contains(src))
    {
        return j1939_event_map.value(src);
    }
    J1939Event *event = new J1939Event(src);
    j1939_event_map.insert(src, event);
    return event;
}

void j1939_event_init()
{
    J1939Ins->pgn_register(PGN_RESPOND_EVENT_CNT, 0, respond_read_event_cnt_cb);
    // J1939Ins->pgn_register(PGN_RESPOND_EVENT, 0, respond_read_event_cb);
}

J1939Event::J1939Event(int src) : src(src)
{
}
J1939Event::~J1939Event()
{
}

void J1939Event::request_event_cnt(int type)
{
    read_event_cnt_request_t request;
    if (type < EVENT_TYPE_NUM)
    {
        request.megic_num = magic_num_map[type];
        QVector<uint8_t> array(sizeof(read_event_cnt_request_t));
        memcpy(&array[0], &request, sizeof(read_event_cnt_request_t));
        J1939Ins->sig_msg_send(PGN_READ_EVENT_CNT, J1939_PRIORITY_DEFAULT, src, array, 0);
    }
}
void J1939Event::request_event(int type, int index)
{
    read_event_request_t request;
    if (type < EVENT_TYPE_NUM)
    {
        request.megic_num = magic_num_map[type];
        request.index     = index;
        QVector<uint8_t> array(sizeof(read_event_request_t));
        memcpy(&array[0], &request, sizeof(read_event_request_t));
        J1939Ins->sig_msg_send(PGN_READ_EVENT, J1939_PRIORITY_DEFAULT, src, array, 0);
    }
}

void J1939Event::request_event_format(int type)
{
    read_event_request_t request;
    if (type < EVENT_TYPE_NUM)
    {
        request.megic_num = magic_num_map[type];
        request.index     = 0;
        QVector<uint8_t> array(sizeof(read_event_request_t));
        memcpy(&array[0], &request, sizeof(read_event_request_t));
        J1939Ins->sig_msg_send(PGN_FORMAT_EVENT, J1939_PRIORITY_DEFAULT, src, array, 0);
    }
}
