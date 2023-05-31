#pragma once

#define J1939_USER_CONFIG
#include "system.h"
#include "stdlib.h"

#define J1939_MALLOC   malloc
#define J1939_FREE     free
#define J1939_GET_MS() sys_get_ms()

enum j1939_pgn_e
{
    // 多帧传输
    J1939_BAM   = 0x00FEECu,
    J1939_TP_CM = 0x00EC00u,
    J1939_TP_DT = 0x00EB00u,
    // 请求应答
    J1939_AC  = 0x00EE00u,
    J1939_RAC = 0x00EA00u,
    // MODBUS数据库读取
    PGN_READ_HOLDING_REGISTER    = (0x0300),
    PGN_READ_INPUT_REGISTER      = (0x0400),
    PGN_WRITE_REGISTER           = (0x0600),
    PGN_WRITE_MULTIPLE_REGISTERS = (0x1000),
    PGN_ERROR                    = (0x8000),
    // 事件请求应答
    PGN_READ_EVENT_CNT    = 0x3000, // 事件数读取请求
    PGN_RESPOND_EVENT_CNT = 0x3100, // 事件数读取应答
    PGN_READ_EVENT        = 0x3200, // 事件读取请求
    PGN_RESPOND_EVENT     = 0x3300, // 事件读取应答
    PGN_FORMAT_EVENT      = 0x3400, // 事件格式化
};
