/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __J1939_H__
#define __J1939_H__


#include "j1939_user_if.h"

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

#define DLC_MAX        8u /*<! CANbus max DLC value */
#define DEFRAG_DLC_MAX (DLC_MAX - 1u)
#define MIN(x, y)      ((x) < (y) ? (x) : (y))

#define J1939_NO_WAIT 0

#define J1939_MAX_DATA_LEN 1785 /*<! Maximum data stream length */

/** @brief indicates that the parameter is "not available" */
#define J1930_NOT_AVAILABLE_8  0xFFu
#define J1930_NA_8             J1930_NOT_AVAILABLE_8
#define J1930_NOT_AVAILABLE_16 0xFF00u
#define J1930_NA_16            J1930_NOT_AVAILABLE_16
#define J1930_NA_16_MSB        (J1930_NOT_AVAILABLE_16 >> 8)
#define J1930_NA_16_LSB        (J1930_NOT_AVAILABLE_16 & 0xFF)

/** @brief indicates that the parameter is "not valid" or "in error" */
#define J1930_NOT_VALID_8  0xFEu
#define J1930_NV_8         J1930_NOT_VALID_8
#define J1930_NOT_VALID_16 0xFE00u
#define J1930_NV_16        J1930_NOT_VALID_16

/** @brief raw parameter values must not exceed the following values */
#define J1930_MAX_8  0xFAu
#define J1930_MAX_16 0xFAFFu

#define J1939_INDUSTRY_GROUP_GLOBAL       0u
#define J1939_INDUSTRY_GROUP_ON_HIGHWAY   1u
#define J1939_INDUSTRY_GROUP_AGRICULTURAL 2u
#define J1939_INDUSTRY_GROUP_CONSTRUCTION 3u
#define J1939_INDUSTRY_GROUP_MARINE       4u
#define J1939_INDUSTRY_GROUP_INDUSTRIAL   5u

#define J1939_NO_ADDRESS_CAPABLE 0u
#define J1939_ADDRESS_CAPABLE    1u

#define SEND_PERIOD 50 /* <! Send period [msec] */

#define J1939_TP_PGN_REG_SIZE 6 // TP传输注册的PGN数

/** @brief Timeouts ([msec]) according to SAE J1939/21 */
enum j1939_timeouts
{
    /* Response Time */
    Tr = 200,
    /* Holding Time */
    Th = 500,
    T1 = 750,
    T2 = 1250,
    T3 = 1250,
    T4 = 1050,
    /* timeout for multi packet broadcast messages 50..200ms */
    Tb = 50,
};

/** @brief Initialize Parameter Group Number using 3 bytes */
#define J1939_INIT_PGN(_dp, _format, _specific)                                \
    {                                                                          \
        (((_dp)&0x01u) << 17) | (((_format)&0xffu) << 8) | ((_specific)&0xffu) \
    }

bool static inline j1939_valid_priority(const uint8_t p)
{
    return p <= J1939_PRIORITY_LOW;
}

static inline uint32_t j1939_pgn_make_key(uint32_t pgn, uint8_t code)
{
    return pgn | (code << 24);
}

uint32_t j1939_pgn2id(const j1939_pgn_t pgn, const uint8_t priority, const uint8_t src);

/**
 * @description: J1393消息发送
 * @param {j1939_can_if_t} *handle  实例句柄
 * @param {j1939_message_t} *msg    发送消息指针，可传递局部变量
 * @param {uint32_t} timeout        发送超时时间（ms），发送队列满后进行等待
 * @return {*}
 */
j1939_ret_e j1939_send(j1939_can_if_t *handle, j1939_message_t *msg, uint32_t timeout);

/**
 * @description: pgn内存堆初始化
 * @param {j1939_t} *handle     实例句柄
 * @param {uint16_t} pool_size  堆大小（可容纳多少个接收回调）
 * @return {*}
 */
void j1939_pgn_pool_init(j1939_t *handle, uint16_t pool_size);

/**
 * @description: j1939 TP传输协议初始化，主要添加各种TP传输回调
 * @param {j1939_t} *handle     实例句柄
 * @return {*}
 */
void j1939_tp_init(j1939_t *handle);

#endif /* __J1939_H__ */