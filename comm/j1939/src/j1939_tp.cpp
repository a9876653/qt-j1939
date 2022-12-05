#include "j1939.h"
#include "pgn.h"
#include "session.h"
#include "compat.h"
#include "string.h"

#define TP_SEND_TIMEOUT 200

#define REASON_NONE         0x00u /*<! No Errors */
#define REASON_BUSY         0x01u /*<! Node is busy */
#define REASON_NO_RESOURCE  0x02u /*<! Lacking the necessary resources */
#define REASON_TIMEOUT      0x03u /*<! A timeout occurred */
#define REASON_CTS_WHILE_DT 0x04u /*<! CTS received when during transfer */
#define REASON_INCOMPLETE   0x05u /*<! Incomplete transfer */

#define CONN_MODE_RTS     0x10u
#define CONN_MODE_CTS     0x11u
#define CONN_MODE_EOM_ACK 0x13u
#define CONN_MODE_ABORT   0xFFu

#define PGN_DECODE(d_array) \
    ((uint32_t)((d_array[7] << 16) & 0xFF0000) + (uint32_t)((d_array[6] << 8) & 0xFF00) + (uint32_t)((d_array[5]) & 0xFF))

/**J1939_TP_Step枚举
 *
 * 实现了记录长帧（多帧）传输步骤
 */
typedef enum
{
    J1939_TP_IDLE,
    J1939_TP_RX_IDLE,
    J1939_TP_RX_READ_DATA,
    J1939_TP_RX_EOM_ACK_WAIT,
    J1939_TP_RX_BAM_READ_DATA,
    J1939_TP_RX_ERROR,
    J1939_TP_RX_DONE,
    J1939_TP_TX_IDLE,
    J1939_TP_TX_CM_WAIT,
    J1939_TP_TX_DT,
    J1939_TP_TX_ERROR,
} j1939_tp_step_e; //协议的传输步骤

static void j1939_tp_rx_abort(j1939_t *handle, j1939_session_t *sess);

static bool j1939_tp_is_tx(j1939_session_t *sess)
{
    return sess->step > J1939_TP_RX_DONE ? true : false;
}

static j1939_ret_e send_abort(j1939_can_if_t *handle, uint32_t pgn, const uint8_t src, const uint8_t dst, const uint8_t reason)
{
    uint8_t data[DLC_MAX] = {
        CONN_MODE_ABORT,
        reason,
        0xFF,
        0xFF,
        0xFF,
        PGN_SPECIFIC(pgn),
        PGN_FORMAT(pgn),
        PGN_DATA_PAGE(pgn),
    };
    j1939_message_t msg;
    msg.pgn      = TP_DT;
    msg.priority = J1939_PRIORITY_LOW;
    msg.src      = src;
    msg.dst      = dst;
    msg.data     = data;
    msg.len      = DLC_MAX;
    return j1939_send(handle, &msg, J1939_NO_WAIT);
}

j1939_ret_e defrag_send(j1939_t      *handle,
                        uint16_t      size,
                        const uint8_t priority,
                        const uint8_t src,
                        const uint8_t dest,
                        uint8_t      *data,
                        uint32_t      timeout)
{
    int             ret;
    uint8_t         seqno = 1;
    uint8_t         frame[DLC_MAX];
    j1939_message_t msg;

    while (size > 0)
    {
        frame[0] = seqno;
        seqno++;

        if (size >= DEFRAG_DLC_MAX)
        {
            memcpy(&frame[1], data, DEFRAG_DLC_MAX);
            size -= DEFRAG_DLC_MAX;
            data += DEFRAG_DLC_MAX;
        }
        else
        {
            memcpy(&frame[1], data, size);
            memset(&frame[1 + size], J1930_NA_8, DEFRAG_DLC_MAX - size);
            size = 0;
        }
        msg.data     = frame;
        msg.dst      = dest;
        msg.pgn      = TP_DT;
        msg.priority = priority;
        msg.src      = src;
        msg.len      = DLC_MAX;
        ret          = j1939_send(&handle->interface, &msg, timeout);
        if (ret < 0)
        {
            return ret;
        }
        // 等待发送周期，j1939规定等待50ms，这里我们不等待了
        // uint32_t now = j1939_get_time();
        // while (!elapsed(now, SEND_PERIOD)) {
        //                yield_task();
        //}
    }
    return J1939_OK;
}

static j1939_ret_e pgn_abort(j1939_t *handle, j1939_message_t *msg)
{
    j1939_session_t *sess = j1939_session_search_addr(&handle->sessions, msg->src, msg->dst);
    if (sess == NULL)
    {
        return -J1939_ENO_RESOURCE;
    }
    if (sess->step != J1939_TP_RX_IDLE)
    {
        j1939_tp_rx_abort(handle, sess);
    }
    else
    {
        j1939_session_ins_close(&handle->sessions, sess);
    }
    return J1939_OK;
}
/** TP TX 处理部分 START **/

static j1939_ret_e
send_tp_rts(j1939_can_if_t *handle, uint32_t pgn, uint8_t priority, uint8_t src, uint8_t dst, uint16_t size, uint8_t num_packets)
{
    uint8_t data[DLC_MAX] = {
        CONN_MODE_RTS,
        size & 0xFF,
        size >> 8,
        num_packets,
        0xFF,
        PGN_SPECIFIC(pgn),
        PGN_FORMAT(pgn),
        PGN_DATA_PAGE(pgn),
    };
    j1939_message_t msg;
    msg.pgn      = TP_CM;
    msg.priority = priority;
    msg.src      = src;
    msg.dst      = dst;
    msg.data     = data;
    msg.len      = DLC_MAX;
    return j1939_send(handle, &msg, J1939_NO_WAIT);
}

static j1939_ret_e send_tp_eom_ack(j1939_can_if_t *handle,
                                   uint32_t        pgn,
                                   const uint8_t   src,
                                   const uint8_t   dst,
                                   const uint16_t  size,
                                   const uint8_t   num_packets)
{
    uint8_t data[DLC_MAX] = {
        CONN_MODE_EOM_ACK,
        size & 0x00FF,
        (size >> 8),
        num_packets,
        0xFF,
        PGN_SPECIFIC(pgn),
        PGN_FORMAT(pgn),
        PGN_DATA_PAGE(pgn),
    };
    j1939_message_t msg;
    msg.pgn      = TP_CM;
    msg.priority = J1939_PRIORITY_LOW;
    msg.src      = src;
    msg.dst      = dst;
    msg.data     = data;
    msg.len      = DLC_MAX;
    return j1939_send(handle, &msg, J1939_NO_WAIT);
}

static j1939_ret_e tp_cts_received(j1939_t *handle, j1939_message_t *msg)
{
    j1939_session_t *sess   = j1939_session_search_addr(&handle->sessions, msg->dst, msg->src);
    j1939_can_if_t  *can_if = &handle->interface;
    if (sess == NULL)
    {
        return -J1939_EARGS;
    }

    sess->cts_num_packets = msg->data[1];
    sess->cts_next_packet = msg->data[2];
    sess->timeout         = J1939_GET_MS();
    if (sess->step == J1939_TP_TX_CM_WAIT)
    {
        sess->step = J1939_TP_TX_DT;
    }
    return J1939_OK;
}

void j1939_tp_tx_abort(j1939_t *handle, j1939_session_t *sess)
{
    j1939_message_t   msg;
    j1939_sessions_t *sessions = &handle->sessions;
    j1939_session_ins_close(sessions, sess);
}

void j1939_tp_tx_handle(j1939_t *handle, j1939_session_t *sess)
{
    j1939_can_if_t *can_if = &handle->interface;
    int             ret    = 0;
    switch (sess->step)
    {
    case J1939_TP_TX_CM_WAIT:
        if (J1939_GET_MS() - sess->timeout > T3)
        {
            j1939_tp_tx_abort(handle, sess); // 等待超时
        }
        break;
    case J1939_TP_TX_DT:
        if (sess->tp_num_packets > 0) // 数据传输
        {
            uint8_t size = 0;
            sess->tp_num_packets -= sess->cts_num_packets;
            size = MIN(sess->cts_num_packets * DEFRAG_DLC_MAX, sess->eom_ack_size);
            ret  = defrag_send(handle, size, J1939_PRIORITY_LOW, sess->src, sess->dst, sess->data, TP_SEND_TIMEOUT);
            if (ret != J1939_OK)
            {
                j1939_tp_tx_abort(handle, sess);
            }
            else if (sess->tp_num_packets == 0) // 传输完成
            {
                ret = send_tp_eom_ack(can_if, sess->pgn, sess->src, sess->dst, sess->eom_ack_size, sess->eom_ack_num_packets);
                j1939_tp_tx_abort(handle, sess);
            }
            else
            {
                sess->timeout = J1939_GET_MS();
                sess->step    = J1939_TP_TX_CM_WAIT;
            }
        }
        else // 传输完成
        {
            ret = send_tp_eom_ack(can_if, sess->pgn, sess->src, sess->dst, sess->eom_ack_size, sess->eom_ack_num_packets);
            j1939_tp_tx_abort(handle, sess);
        }
        break;
    };
}

/** TP TX 处理部分 END **/

/** TP RX 处理部分 START **/
j1939_ret_e j1939_send_tp_cts(j1939_can_if_t *handle,
                              uint32_t        pgn,
                              const uint8_t   src,
                              const uint8_t   dst,
                              const uint8_t   num_packets,
                              const uint8_t   next_packet)
{
    uint8_t data[DLC_MAX] = {
        CONN_MODE_CTS,
        num_packets,
        next_packet,
        0xFF,
        0xFF,
        PGN_SPECIFIC(pgn),
        PGN_FORMAT(pgn),
        PGN_DATA_PAGE(pgn),
    };
    j1939_message_t msg;
    msg.pgn      = TP_CM;
    msg.priority = J1939_PRIORITY_LOW;
    msg.src      = src;
    msg.dst      = dst;
    msg.data     = data;
    msg.len      = DLC_MAX;
    return j1939_send(handle, &msg, J1939_NO_WAIT);
}

static void j1939_tp_rx_abort(j1939_t *handle, j1939_session_t *sess)
{
    sess->step = J1939_TP_RX_IDLE;
    if (sess->err_handle)
    {
        sess->err_handle(sess->pgn, sess->src);
    }
}

static j1939_ret_e tp_eom_ack_received(j1939_t *handle, j1939_message_t *msg)
{
    j1939_ret_e      ret    = 0;
    j1939_can_if_t  *can_if = &handle->interface;
    j1939_session_t *sess   = j1939_session_search_addr(&handle->sessions, msg->src, msg->dst);
    if (sess == NULL)
    {
        ret = -J1939_ENO_RESOURCE;
        j1939_tp_rx_abort(handle, sess);
        return ret;
    }

    if (J1939_GET_MS() - sess->timeout > T3)
    {
        ret = -J1939_ETIMEOUT;
        j1939_tp_rx_abort(handle, sess);
        return ret;
    }

    sess->timeout                = J1939_GET_MS();
    uint16_t eom_ack_size        = htobe16((msg->data[1] << 8) | msg->data[2]);
    uint8_t  eom_ack_num_packets = msg->data[3];

    if (sess->eom_ack_size != eom_ack_size || sess->eom_ack_num_packets != eom_ack_num_packets)
    {
        ret = -J1939_EINCOMPLETE;
        j1939_tp_rx_abort(handle, sess);
        return ret;
    }

    // j1939_session_close(&handle->sessions, msg->dst, msg->src);
    sess->step = J1939_TP_RX_IDLE;
    return J1939_OK;
}

static j1939_ret_e request_to_send(j1939_t *handle, j1939_message_t *msg)
{
    j1939_can_if_t  *can_if = &handle->interface;
    j1939_session_t *sess   = j1939_session_search_addr(&handle->sessions, msg->src, msg->dst);
    if (sess == NULL || sess->step != J1939_TP_RX_IDLE)
    {
        return -J1939_EARGS;
    }
    sess->pgn = PGN_DECODE(msg->data);
    sess->src = msg->src;

    sess->tp_tot_size         = htobe16((msg->data[1] << 8) | msg->data[2]);
    sess->tp_num_packets      = DIV_ROUND_UP(sess->tp_tot_size, DEFRAG_DLC_MAX);
    sess->eom_ack_num_packets = sess->tp_num_packets;
    sess->eom_ack_size        = sess->tp_tot_size;
    sess->step                = J1939_TP_RX_READ_DATA;
    sess->timeout             = J1939_GET_MS();
    if (sess->get_data)
    {
        if (sess->get_data(sess->pgn, sess->src, &sess->data, &sess->data_size) < 0)
        {
            return -J1939_EIO;
        }
    }
    return j1939_send_tp_cts(can_if, sess->pgn, msg->dst, msg->src, sess->tp_num_packets, 0);
}

static j1939_ret_e rcv_tp(j1939_t *handle, j1939_message_t *msg)
{
    j1939_can_if_t  *can_if = &handle->interface;
    j1939_session_t *sess   = j1939_session_search_addr(&handle->sessions, msg->src, msg->dst);

    if (sess == NULL)
    {
        return -J1939_EARGS;
    }

    sess->timeout = J1939_GET_MS();
    sess->tp_num_packets--;
    uint32_t offset = (msg->data[0] - 1) * DEFRAG_DLC_MAX;
    uint8_t  len    = MIN(sess->tp_tot_size - offset, msg->len - 1);
    // 接收DT数据处理
    if (sess->data_size > offset + len)
    {
        memcpy(sess->data + offset, msg->data + 1, len);
    }
    if (sess->tp_num_packets == 0)
    {
        /* Next packet expected to be EOM ACK */
        if (sess->step == J1939_TP_RX_READ_DATA) // TP RX wait eom ack
        {
            sess->step = J1939_TP_RX_EOM_ACK_WAIT;
        }
        else
        {
            sess->step = J1939_TP_RX_IDLE;
        }
        if (sess->rec_finish)
        {
            sess->rec_finish(sess->pgn, sess->src, sess->data, sess->tp_tot_size);
        }
    }
    return J1939_OK;
}

void j1939_tp_rx_handle(j1939_t *handle, j1939_session_t *sess)
{
    j1939_can_if_t *can_if = &handle->interface;
    switch (sess->step)
    {
    case J1939_TP_RX_READ_DATA:
        if (J1939_GET_MS() - sess->timeout > T3)
        {
            j1939_tp_rx_abort(handle, sess); // 等待超时
        }
        break;
    case J1939_TP_RX_EOM_ACK_WAIT:
        if (J1939_GET_MS() - sess->timeout > T3)
        {
            j1939_tp_rx_abort(handle, sess);
        }
        break;
    };
}

/** TP RX 处理部分 END **/

/** TP BAM 处理部分 START **/
#define BAM_SEND_TIMEOUT 200
#define CONN_MODE_BAM    0x20u

static j1939_ret_e bam_recv_start(j1939_t *handle, j1939_message_t *msg)
{
    j1939_can_if_t  *can_if = &handle->interface;
    j1939_session_t *sess   = j1939_session_search_addr(&handle->sessions, msg->src, msg->dst);
    if (sess == NULL || sess->step != J1939_TP_RX_IDLE)
    {
        return -J1939_EARGS;
    }
    sess->pgn                 = PGN_DECODE(msg->data);
    sess->src                 = msg->src;
    sess->tp_tot_size         = htobe16((msg->data[1] << 8) | msg->data[2]);
    sess->tp_num_packets      = DIV_ROUND_UP(sess->tp_tot_size, DEFRAG_DLC_MAX);
    sess->eom_ack_num_packets = sess->tp_num_packets;
    sess->eom_ack_size        = sess->tp_tot_size;
    sess->step                = J1939_TP_RX_BAM_READ_DATA;
    if (sess->get_data)
    {
        if (sess->get_data(sess->pgn, sess->src, &sess->data, &sess->data_size) < 0)
        {
            return -J1939_EIO;
        }
    }
}

void j1939_tp_rx_bam_handle(j1939_t *handle, j1939_session_t *sess)
{
    j1939_can_if_t *can_if = &handle->interface;
    switch (sess->step)
    {
        break;
    };
}

j1939_ret_e j1939_tp_bam(j1939_t *handle, uint32_t pgn, uint8_t priority, uint8_t dst, uint8_t *data, uint16_t len)
{
    j1939_message_t msg;
    int             ret;
    uint8_t         num_packets = DIV_ROUND_UP(len, DEFRAG_DLC_MAX);

    uint8_t bam[DLC_MAX] = {
        CONN_MODE_BAM,
        len & 0x00FF,
        len >> 8,
        num_packets,
        0xFF,
        PGN_SPECIFIC(pgn),
        PGN_FORMAT(pgn),
        PGN_DATA_PAGE(pgn),
    };

    if (len > J1939_MAX_DATA_LEN)
    {
        return -J1939_EARGS;
    }
    msg.data     = bam;
    msg.dst      = dst;
    msg.pgn      = TP_CM;
    msg.priority = priority;
    msg.src      = handle->node_addr;
    msg.len      = DLC_MAX;
    ret          = j1939_send(&handle->interface, &msg, BAM_SEND_TIMEOUT);
    if (ret < 0)
    {
        return ret;
    }
    return defrag_send(handle, len, priority, handle->node_addr, dst, data, BAM_SEND_TIMEOUT);
}

/** TP BAM 处理部分 END **/

void j1939_tp_foreach_sessions(j1939_t *handle)
{
    j1939_sessions_t *sessions = &handle->sessions;
    j1939_can_if_t   *can_if   = &handle->interface;
    int               ret      = 0;
    for (size_t i = 0; i < sessions->sessions_size; i++)
    {
        if (sessions->session_dict[i].id >= 0)
        {
            j1939_session_t *sess = &sessions->session_dict[i];
            if (j1939_tp_is_tx(sess))
            {
                j1939_tp_tx_handle(handle, sess);
            }
            else
            {
                j1939_tp_rx_handle(handle, sess);
            }
        }
    }
}

void j1939_tp_poll(j1939_t *handle)
{
    j1939_tp_foreach_sessions(handle);
}

j1939_ret_e j1939_tp(j1939_t       *handle,
                     j1939_pgn_t    pgn,
                     const uint8_t  priority,
                     const uint8_t  src,
                     const uint8_t  dst,
                     uint8_t       *data,
                     const uint16_t len)
{
    bool             initiated = false;
    int              ret;
    j1939_session_t *sess;
    uint8_t          num_packets, reason;
    uint16_t         size;
    j1939_message_t  msg;
    j1939_can_if_t  *can_if = &handle->interface;
    if (len > J1939_MAX_DATA_LEN)
    {
        return -J1939_EWRONG_DATA_LEN;
    }

    /* single frame, send directly */
    if (len <= DLC_MAX)
    {
        msg.pgn      = pgn;
        msg.priority = priority;
        msg.src      = src;
        msg.dst      = dst;
        msg.data     = data;
        msg.len      = len;
        return j1939_send(can_if, &msg, TP_SEND_TIMEOUT);
    }

    sess = j1939_session_open(&handle->sessions, src, dst);
    if (sess == NULL || sess->step != J1939_TP_IDLE)
    {
        return -J1939_ENO_RESOURCE;
    }

    num_packets               = DIV_ROUND_UP(len, DEFRAG_DLC_MAX);
    sess->eom_ack_num_packets = num_packets;
    sess->eom_ack_size        = len;
    sess->timeout             = J1939_GET_MS();
    /* Send Request To Send (RTS) */
    ret = send_tp_rts(can_if, pgn, priority, src, dst, len, num_packets);
    if (ret < 0)
    {
        j1939_session_close(&handle->sessions, src, dst);
        return ret;
    }

    sess->step           = J1939_TP_TX_CM_WAIT;
    sess->tp_num_packets = num_packets;
    sess->data           = data;
    sess->dst            = dst;
    sess->src            = src;
    return ret;
}

void j1939_tp_init(j1939_t *handle)
{
    j1939_pgn_register(handle, TP_CM, CONN_MODE_CTS, tp_cts_received);         // TP_TX
    j1939_pgn_register(handle, TP_CM, CONN_MODE_ABORT, pgn_abort);             // TP_TX
    j1939_pgn_register(handle, TP_CM, CONN_MODE_RTS, request_to_send);         // TP_RX
    j1939_pgn_register(handle, TP_CM, CONN_MODE_EOM_ACK, tp_eom_ack_received); // TP_RX
    j1939_pgn_register(handle, TP_DT, 0, rcv_tp);                              // TP_RX_DATA
    j1939_pgn_register(handle, TP_CM, CONN_MODE_BAM, bam_recv_start);          // TP_BAM_REC
}

j1939_ret_e j1939_tp_rx_register(j1939_t             *handle,
                                 uint8_t              src,
                                 uint8_t              dst,
                                 session_get_data_fun get_data,
                                 session_recv_fun     rec_finish,
                                 session_err_fun      err_handle)
{
    j1939_session_t *sess = j1939_session_open(&handle->sessions, src, dst);
    if (sess == NULL)
    {
        return -J1939_ENO_RESOURCE;
    }
    sess->get_data   = get_data;
    sess->err_handle = err_handle;
    sess->rec_finish = rec_finish;
    sess->src        = src;
    sess->dst        = dst;
    sess->step       = J1939_TP_RX_IDLE;
    return J1939_OK;
}
