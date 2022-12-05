#pragma once

#include "j1939_config.h"
#include "hasht.h"
#include "session.h"

#define ADDRESS_GLOBAL      0xFFu
#define ADDRESS_NOT_CLAIMED 0xFEu
#define ADDRESS_NULL        0xEFu

#define J1939_PRIORITY_HIGH    0x0u
#define J1939_PRIORITY_DEFAULT 0x6u
#define J1939_PRIORITY_LOW     0x7u

typedef enum
{
    J1939_OK              = 1,   // 正常
    J1939_EARGS           = 100, // 参数出错
    J1939_ETIMEOUT        = 101, // 超时
    J1939_EBUSY           = 102, // 忙碌
    J1939_EINCOMPLETE     = 103, // 未完成
    J1939_EWRONG_DATA_LEN = 104, // 数据长度出错
    J1939_ENO_RESOURCE    = 105, // 资源不足
    J1939_EIO             = 106, // IO出错
} __j1939_ret_e;

typedef int j1939_ret_e;

/** @brief J1939 PGN according to SAE J1939/21 */
typedef uint32_t j1939_pgn_t;

typedef int (*j1939_can_write_fun)(uint32_t id, uint8_t *data, uint8_t len);

typedef struct
{
    j1939_pgn_t pgn;      // pgn号
    uint8_t     priority; // 优先级
    uint8_t     src;      // 源地址
    uint8_t     dst;      // 目标地址
    uint8_t    *data;     // 注：data内存不作拷贝，只做引用（CAN接收缓冲字节），请勿改变里面的值
    uint32_t    len;      // 数据长度，不大于8
} j1939_message_t;

typedef struct
{
    j1939_can_write_fun write; // can发送接口函数
} j1939_can_if_t;

typedef struct
{
    j1939_can_if_t   interface; // 硬件操作接口
    hasht_t          pgn_pool;  // pgn注册回调堆
    j1939_sessions_t sessions;  // 多帧传输回话堆
    j1939_message_t  temp;      // 临时变量
    uint8_t          node_addr; // 节点地址
} j1939_t;

/**
 * @description: pgn回调函数
 * @param {j1939_t} *handle 实例句柄
 * @param {j1939_message_t} *msg    接收消息
 * @return j1939_ret_e，具体看故障码
 */
typedef j1939_ret_e (*pgn_callback_t)(j1939_t *handle, j1939_message_t *msg);

/**
 * @description: can接收处理函数，收到can数据后调用该函数
 * @param {j1939_t} *handle 实例句柄
 * @param {uint32_t} id     can ID
 * @param {uint8_t} *data   can 接收数据
 * @param {uint8_t} len     数据长度，不大于8
 * @return j1939_ret_e，具体看故障码
 */
j1939_ret_e j1939_receive_handle(j1939_t *handle, uint32_t id, uint8_t *data, uint8_t len);

/**
 * @description: 单帧传输发送
 * @param {j1939_t} *handle 实例句柄
 * @param {uint32_t} pgn    pgn
 * @param {uint8_t} priority    优先级
 * @param {uint8_t} dst         目标地址，一般为ADDRESS_GLOBAL 0xFF
 * @param {uint8_t} *data       发送缓冲区，可使用局部变量
 * @param {uint16_t} len        发送数据大小
 * @param {uint32_t} timeout    发送超时时间
 * @return j1939_ret_e，具体看故障码
 */
j1939_ret_e
j1939_send_msg(j1939_t *handle, uint32_t pgn, uint8_t priority, uint8_t dst, uint8_t *data, uint16_t len, uint32_t timeout);

/**
 * @description: j1939初始化函数
 * @param {j1939_t} *handle 实例句柄
 * @param {uint8_t} node_addr   节点地址=源地址
 * @param {uint16_t} pool_size  pgn注册堆的大小，允许容纳多少个pgn注册（tp传输协议固定占用6个，实际pool_size内部自动+6）
 * @param {uint16_t} session_size   多帧传输会话注册堆大小，允许容纳多少个多帧传输会话
 * @param {j1939_can_write_fun} write   can发送数据接口函数
 * @return {*}
 */
void j1939_init(j1939_t *handle, uint8_t node_addr, uint16_t pool_size, uint16_t session_size, j1939_can_write_fun write);

/**
 * @description: tp传输协议轮询任务
 * @param {j1939_t} *handle 实例句柄
 * @return {*}
 */
void j1939_tp_poll(j1939_t *handle);

/** 多帧传输协议发送（点对点）
 * @description:
 * @param {j1939_t      } *handle 实例句柄
 * @param {j1939_pgn_t   } pgn    pgn
 * @param {uint8_t } priority     优先级
 * @param {uint8_t } src          源地址，一般为本机节点地址
 * @param {uint8_t } dst          目标地址
 * @param {uint8_t      } *data   传输数据指针，注：不可用局部变量
 * @param {uint16_t} len          传输数据长度
 * @return j1939_ret_e，具体看故障码
 */
j1939_ret_e j1939_tp(j1939_t       *handle,
                     j1939_pgn_t    pgn,
                     const uint8_t  priority,
                     const uint8_t  src,
                     const uint8_t  dst,
                     uint8_t       *data,
                     const uint16_t len);

/**
 * @description: 注册Tp传输协议回调
 * @param {j1939_t            } *handle 实例句柄
 * @param {uint8_t             } src    源地址
 * @param {uint8_t             } dst    目标地址
 * @param {session_get_data_fun} get_data   获取数据缓冲区回调，返回j1939_ret_e，注：缓冲区指针不能指向局部变量
 * @param {session_recv_fun    } rec_finish 接收完成回调
 * @param {session_err_fun     } err_handle 接收出错回调
 * @return j1939_ret_e，具体看故障码
 */
j1939_ret_e j1939_tp_rx_register(j1939_t             *handle,
                                 uint8_t              src,
                                 uint8_t              dst,
                                 session_get_data_fun get_data,
                                 session_recv_fun     rec_finish,
                                 session_err_fun      err_handle);

/**
 * @description: 多帧广播传输发送
 * @param {j1939_t} *handle 实例句柄
 * @param {uint32_t} pgn    pgn
 * @param {uint8_t} priority    优先级
 * @param {uint8_t} dst         目标地址，一般为ADDRESS_GLOBAL 0xFF
 * @param {uint8_t} *data       发送缓冲区，可使用局部变量
 * @param {uint16_t} len        发送数据大小
 * @return j1939_ret_e，具体看故障码
 */
j1939_ret_e j1939_tp_bam(j1939_t *handle, uint32_t pgn, uint8_t priority, uint8_t dst, uint8_t *data, uint16_t len);

/**
 * @description: pgn接收回调注册
 * @param {j1939_t} *handle 实例句柄
 * @param {uint32_t} pgn    pgn码
 * @param {uint8_t} code    识别码，非Tp协议填0
 * @param {pgn_callback_t} cb   注册回调
 * @return >0:注册的key值，<0:出错
 */
int j1939_pgn_register(j1939_t *handle, const uint32_t pgn, uint8_t code, pgn_callback_t cb);

/***********注销的函数慎用***************/
/**
 * @description: pgn接收回调注销
 * @param {j1939_t} *handle 实例句柄
 * @param {uint32_t} pgn    pgn码
 * @param {uint8_t} code    识别码，非Tp协议填0
 * @return >0:注册的key值，<0:出错
 */
int j1939_pgn_deregister(j1939_t *handle, const uint32_t pgn, uint8_t code);

/**
 * @description: 注销所有的pgn接收回调
 * @param {j1939_t} *handle 实例句柄
 * @return {*}
 */
void j1939_pgn_deregister_all(j1939_t *handle);
