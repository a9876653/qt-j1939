/* SPDX-License-Identifier: Apache-2.0 */
#include "j1939.h"
#include <string.h>

#include "session.h"

#define SESSION_UNDEF (-1)

static uint16_t j1939_session_hash(const uint8_t s, const uint8_t d)
{
    return (s << 8) | d;
}

void j1939_session_init(j1939_sessions_t *handle, uint16_t sessions_size)
{
    handle->session_dict   = (j1939_session_t *)J1939_MALLOC(sizeof(j1939_session_t) * sessions_size);
    handle->sessions.items = (hasht_entry_t *)J1939_MALLOC(sizeof(hasht_entry_t) * sessions_size);
    memset(handle->session_dict, 0, sizeof(j1939_session_t) * sessions_size);
    memset(handle->sessions.items, 0, sizeof(hasht_entry_t) * sessions_size);
    handle->sessions_size     = sessions_size;
    handle->sessions.max_size = sessions_size;
    handle->sessions.size     = 0;
    hasht_init(&handle->sessions);
    for (size_t i = 0; i < sessions_size; i++)
    {
        handle->session_dict[i].id = SESSION_UNDEF;
    }
}

static j1939_session_t *assign_session(j1939_sessions_t *handle)
{
    for (size_t i = 0; i < handle->sessions_size; i++)
    {
        if (handle->session_dict[i].id < 0)
        {
            memset(&handle->session_dict[i], 0, sizeof(j1939_session_t));
            handle->session_dict[i].id = i;
            return &handle->session_dict[i];
        }
    }
    return NULL;
}

j1939_session_t *j1939_session_open(j1939_sessions_t *handle, const uint8_t src, const uint8_t dest)
{
    j1939_session_t *sess;
    uint16_t         key = j1939_session_hash(src, dest);
    if (j1939_session_search(handle, key) == NULL)
    {
        sess = assign_session(handle);
        if (sess)
        {
            hasht_insert(&handle->sessions, key, sess);
            return sess;
        }
    }
    return NULL;
}

j1939_session_t *j1939_session_search_addr(j1939_sessions_t *handle, const uint16_t src, const uint16_t dst)
{
    uint16_t key = j1939_session_hash(src, dst);
    return j1939_session_search(handle, key);
}

j1939_session_t *j1939_session_search(j1939_sessions_t *handle, const uint16_t id)
{
    hasht_entry_t *s;
    s = hasht_search(&handle->sessions, id);
    return s != NULL ? (j1939_session_t *)s->item : NULL;
}

int j1939_session_ins_close(j1939_sessions_t *handle, j1939_session_t *sess)
{
    if (sess)
    {
        uint16_t key = j1939_session_hash(sess->src, sess->dst);
        sess->id     = SESSION_UNDEF;
        return hasht_delete(&handle->sessions, key);
    }
    return -1;
}

int j1939_session_close(j1939_sessions_t *handle, const uint8_t src, const uint8_t dest)
{
    j1939_session_t *sess;
    uint16_t         key = j1939_session_hash(src, dest);
    sess                 = j1939_session_search(handle, key);
    if (sess)
    {
        sess->id = SESSION_UNDEF;
        return hasht_delete(&handle->sessions, key);
    }
    return -1;
}
