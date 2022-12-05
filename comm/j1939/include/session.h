/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __SESSION_H__
#define __SESSION_H__

#include "hasht.h"

typedef int (*session_get_data_fun)(uint32_t pgn, uint8_t src, uint8_t **data, uint16_t *len);
typedef void (*session_err_fun)(uint32_t pgn, uint8_t src);
typedef void (*session_recv_fun)(uint32_t pgn, uint8_t src, uint8_t *data, uint16_t len);

typedef struct
{
    int8_t   id;
    uint8_t  cts_num_packets;     // 请求包数
    uint8_t  cts_next_packet;     // 下一包的索引
    uint16_t eom_ack_size;        // 整包大小
    uint8_t  eom_ack_num_packets; // 包数回复
    uint8_t  tp_num_packets;      // 当前发送的包数
    uint16_t tp_tot_size;         // 总共传输的字节

    uint32_t pgn;       // pgn码
    uint8_t  src;       // 源地址
    uint8_t  dst;       // 目标地址
    uint8_t *data;      // 数据缓冲区，非拷贝
    uint16_t data_size; // 数据缓冲区大小
    uint8_t  step;      // 发送/接收步骤
    uint32_t timeout;   // 超时计数器，毫秒
    session_get_data_fun
                     get_data; // 获取数据缓冲区的回调，CAN接收里执行，如CAN在中断接收，需注意阻塞；数据非拷贝，注意传递的数据指针引用
    session_recv_fun rec_finish; //　接收完成回调
    session_err_fun  err_handle; //　发送/接收出错回调

} j1939_session_t;

typedef struct
{
    j1939_session_t *session_dict;  // 会话堆指针
    hasht_t          sessions;      // 会话的哈希表
    uint16_t         sessions_size; // 堆大小
} j1939_sessions_t;

void             j1939_session_init(j1939_sessions_t *handle, uint16_t sessions_size);
int              j1939_session_close(j1939_sessions_t *handle, const uint8_t src, const uint8_t dest);
int              j1939_session_ins_close(j1939_sessions_t *handle, j1939_session_t *sess);
j1939_session_t *j1939_session_open(j1939_sessions_t *handle, const uint8_t src, const uint8_t dest);
j1939_session_t *j1939_session_search(j1939_sessions_t *handle, const uint16_t id);
j1939_session_t *j1939_session_search_addr(j1939_sessions_t *handle, const uint16_t src, const uint16_t dst);

#endif /* __SESSION_H__ */
