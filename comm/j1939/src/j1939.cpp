/* SPDX-License-Identifier: Apache-2.0 */

/*
 * J1939 Message Construction
 *
 * J1939 messages are built on top of CAN 2.0b and make specific use of
 * extended frames. Extended frames use a 29-bit identifier instead of the
 * common 11-bit identifier.
 *
 * |======================= CAN IDENTIFIER (29 bit) =======================|
 * | Priority [3] | Parameter Group Number (PGN) [18] | Source Address [8] |
 * |-----------------------------------------------------------------------|
 * | EDP [1]  | DP [1]  | PDU Format [8] | PDU Specific / Destination [8]  |
 * |=======================================================================|
 *
 * The first three bits are the priority field.
 * This field sets the message’s priority on the network and helps ensure
 * messages with higher importance are sent/received before lower priority
 * messages. Zero is the highest priority.
 *
 * Using the Extended Data Page bit (EDP) and the Data Page bit (DP),
 * four different "Data pages" for J1939 messages (Parameter Group) can be
 * selected:
 *
 * EDP | DP |  Description
 * =======================================
 *  0  | 0  |  SAE J1939 Parameter Groups
 *  0  | 1  |  NMEA2000 defined
 *  1  | 0  |  SAE J1939 reserved
 *  1  | 1  |  ISO 15765-3 defined
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "j1939.h"
#include "pgn.h"

void j1939_pgn_pool_init(j1939_t *handle, uint16_t pool_size)
{
    handle->pgn_pool.items = (hasht_entry_t *)J1939_MALLOC(pool_size * sizeof(hasht_entry_t));
    memset(handle->pgn_pool.items, 0, pool_size * sizeof(hasht_entry_t));
    handle->pgn_pool.max_size = pool_size;
    handle->pgn_pool.size     = 0;
    hasht_init(&handle->pgn_pool);
}

int j1939_pgn_register(j1939_t *handle, const uint32_t pgn, const uint8_t code, const pgn_callback_t cb)
{
    return hasht_insert(&handle->pgn_pool, j1939_pgn_make_key(pgn, code), (void *)cb);
}

int j1939_pgn_deregister(j1939_t *handle, const uint32_t pgn, const uint8_t code)
{
    return hasht_delete(&handle->pgn_pool, j1939_pgn_make_key(pgn, code));
}

void j1939_pgn_deregister_all(j1939_t *handle)
{
    hasht_clear(&handle->pgn_pool);
}

uint32_t j1939_pgn2id(const j1939_pgn_t pgn, const uint8_t priority, const uint8_t src)
{
    return (((uint32_t)priority & PRGN_PRIORITY_MASK) << 26) | ((pgn & PGN_MASK) << 8) | (uint32_t)src;
}

j1939_ret_e j1939_send(j1939_can_if_t *handle, j1939_message_t *msg, uint32_t timeout)
{
    uint32_t id;

    if (!j1939_valid_priority(msg->priority))
    {
        return -J1939_EARGS;
    }

    id = j1939_pgn2id(msg->pgn, msg->priority, msg->src);

    /* If PGN is peer-to-peer, add destination address to the ID */
    if (j1939_pdu_is_p2p(msg->pgn))
    {
        id = (id & 0xFFFF00FFu) | ((uint32_t)msg->dst << 8);
    }
    int ret = handle->write(id, msg->data, msg->len);
    if (ret <= 0 && timeout > 0)
    {
        uint32_t tick = J1939_GET_MS();
        while ((J1939_GET_MS() - tick < timeout) && ret <= 0)
        {
            ret = handle->write(id, msg->data, msg->len);
            sys_delay_ms(1);
        }
    }
    if (ret == msg->len)
    {
        return J1939_OK;
    }
    else
    {
        return -J1939_ETIMEOUT;
    }
}

j1939_ret_e
j1939_send_msg(j1939_t *handle, uint32_t pgn, uint8_t priority, uint8_t dst, uint8_t *data, uint16_t len, uint32_t timeout)
{
    j1939_message_t msg;
    if (len > DLC_MAX || data == NULL)
    {
        return -J1939_EARGS;
    }
    msg.pgn      = pgn;
    msg.priority = priority;
    msg.dst      = dst;
    msg.src      = handle->node_addr;
    msg.data     = data;
    msg.len      = len;
    return j1939_send(&handle->interface, &msg, timeout);
}

j1939_ret_e j1939_receive(j1939_message_t *msg, uint32_t id, uint8_t *data, uint8_t len)
{
    if (!msg)
    {
        return -J1939_EARGS;
    }

    msg->len      = len;
    msg->priority = (id & 0x1C000000u) >> 26;
    msg->src      = id & 0x000000FFu;
    msg->pgn      = (id & 0xFFFF00u) >> 8;

    /*
     * if PGN is peer-to-peer, remove destination from
     * PGN itself and calculate destination address
     */
    if (j1939_pdu_is_p2p(msg->pgn))
    {
        msg->pgn = id & 0xFFFF00FFu;
        msg->dst = (id >> 8) & 0x000000FFu;
    }
    else
    {
        msg->pgn = id & 0x00FFFF00u;
        msg->dst = ADDRESS_NULL;
    }
    msg->pgn  = (msg->pgn >> 8) & PGN_MASK;
    msg->data = data;
    return J1939_OK;
}

j1939_ret_e j1939_receive_handle(j1939_t *handle, uint32_t id, uint8_t *data, uint8_t len)
{
    hasht_entry_t  *entry;
    uint8_t         code;
    int             ret;
    j1939_message_t msg;

    ret = j1939_receive(&msg, id, data, len);
    if (ret == J1939_OK)
    {
        if ((msg.pgn & 0xFF00) == TP_CM)
        {
            msg.pgn = TP_CM;
            code    = msg.data[0];
        }
        else
        {
            code = 0;
        }
        entry = hasht_search(&handle->pgn_pool, j1939_pgn_make_key(msg.pgn, code));
        if (entry && entry->item)
        {
            pgn_callback_t cb = (pgn_callback_t)entry->item;
            return (*cb)(handle, &msg);
        }
    }
    return ret;
}

void j1939_init(j1939_t *handle, uint8_t node_addr, uint16_t pool_size, uint16_t session_size, j1939_can_write_fun write)
{
    handle->node_addr       = node_addr;
    handle->interface.write = write;

    j1939_pgn_pool_init(handle, pool_size + J1939_TP_PGN_REG_SIZE); // 自定义注册数+TP使用数
    j1939_session_init(&handle->sessions, session_size);
    j1939_tp_init(handle);
}
