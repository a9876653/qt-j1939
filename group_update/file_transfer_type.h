#pragma once

#include "boot_header.h"
#include <stdint.h>
#include <string.h>

enum file_transfer_state_e
{
    FILE_TRANSFER_IDLE  = 0,
    FILE_TRANSFER_START = 0,
    FILE_TRANSFER_INFO,
    FILE_TRANSFER_DATA,
    FILE_TRANSFER_VERIFY,
};

enum file_respone_e
{
    FILE_RESPONE_OK = 1,
    FILE_RESPONE_BOARD_ERR,
    FILE_RESPONE_REPEAT,
    FILE_RESPONE_OFFSET_ERR,
    FILE_RESPONE_CHECK_CRC_ERR,
};

#define FILE_MSG_ID_START     (3020 | 0xF000) // 0xF000：广播帧
#define FILE_MSG_ID_FILE_INFO (3022 | 0xF000)
#define FILE_MSG_ID_FILE_DATA (3024 | 0xF000)
#define FILE_MSG_ID_CHECK_CRC (3026 | 0xF000)

#define FILE_MSG_ID_START_RESPONE     (FILE_MSG_ID_START + 1)
#define FILE_MSG_ID_FILE_INFO_RESPONE (FILE_MSG_ID_FILE_INFO + 1)
#define FILE_MSG_ID_FILE_DATA_RESPONE (FILE_MSG_ID_FILE_DATA + 1)
#define FILE_MSG_ID_CHECK_CRC_RESPONE (FILE_MSG_ID_CHECK_CRC + 1)

#define FILE_DATA_RECV_MAX_SIZE 256
#define FILE_OBJ_ADDR_MAX_NUM   32
#define FILE_ADDR_INVAILD       0xFF

// typedef struct
//{
//    uint8_t  respone;
//    uint32_t offset;
//} respone_file_data_t;

// typedef struct
//{
//    uint32_t offset;
//    uint16_t data_len;
//    uint8_t  data[FILE_DATA_RECV_MAX_SIZE];
//} file_data_t;

typedef struct
{
    uint8_t process;
    uint8_t obj_num;
    uint8_t obj_addr[FILE_OBJ_ADDR_MAX_NUM];
} file_process_t;

typedef struct file_transfer_pro_s file_transfer_pro_t;

typedef struct
{
    uint32_t    len;
    const char *name;
} file_info_t;
