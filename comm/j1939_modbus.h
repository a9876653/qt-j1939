#pragma once

#include "modbus_crc.h"

typedef enum
{
    MB_EX_NONE                 = 0x00,
    MB_EX_ILLEGAL_FUNCTION     = 0x01,
    MB_EX_ILLEGAL_DATA_ADDRESS = 0x02,
    MB_EX_ILLEGAL_DATA_VALUE   = 0x03,
    MB_EX_SLAVE_DEVICE_FAILURE = 0x04,
    MB_EX_ACKNOWLEDGE          = 0x05,
    MB_EX_SLAVE_BUSY           = 0x06,
    MB_EX_MEMORY_PARITY_ERROR  = 0x08,
    MB_EX_GATEWAY_PATH_FAILED  = 0x0A,
    MB_EX_GATEWAY_TGT_FAILED   = 0x0B
} eMBException;

#define DB_FUNC_READ_HOLDING_REGISTER    (0x0300)
#define DB_FUNC_READ_INPUT_REGISTER      (0x0400)
#define DB_FUNC_WRITE_REGISTER           (0x0600)
#define DB_FUNC_WRITE_MULTIPLE_REGISTERS (0x1000)
#define DB_FUNC_ERROR                    (0x8000)

#define MB_DATA_BUFF_MAX               255
#define MB_WRITE_MUL_REG_BUFF_MAX      248
#define MB_WRITE_RESPOND_BUFF_SIZE     4
#define MB_WRITE_MUL_RESPOND_BUFF_SIZE 4
#define MB_REG_LEN_MAX                 125

typedef struct
{
    uint16_t reg_addr;
    uint16_t reg_len;
} request_read_reg_t;

typedef struct
{
    uint16_t reg_addr;
    uint8_t  data_len;
    uint8_t  data[MB_DATA_BUFF_MAX];
} respond_read_reg_t;

typedef struct
{
    uint16_t reg_addr;
    uint16_t reg_value;
} request_write_reg_t;

typedef struct
{
    uint16_t reg_addr;
    uint16_t reg_len;
    uint8_t  data[MB_WRITE_MUL_REG_BUFF_MAX];
} request_write_mul_reg_t;

typedef struct
{
    uint16_t reg_addr;
    uint16_t reg_len;
} respond_write_mul_reg_t;
