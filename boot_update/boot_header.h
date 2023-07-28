#pragma once

#include "stdint.h"

#define FLASH_DEFINE_NUM 0xFFFFFFFF

#define APP_INFO ((app_info_t *)APP_INFO_ADDRESS)

#define APP_START_ADDRESS_ARRAY(index) (*((volatile uint32_t *)(APP_START_ADDRESS + index)))

#define MAGIC_NUM_UAVCAN_BOOT 0xA0A0A0A0
#define MAGIC_NUM_UART_BOOT   0x746F6F62 // "boot"

#define BOOT_UPDATE_CMD 0x56

#define OBJ_ADDR_MAX_NUM        128
#define FILE_DATA_RECV_MAX_SIZE 256

#define BOOT_MSG_ID_UPDATE_CMD            (3000 | 0xF000) // 0xF000：广播帧
#define BOOT_MSG_ID_FILE_INFO             (3002 | 0xF000)
#define BOOT_MSG_ID_FILE_DATA             (3004 | 0xF000)
#define BOOT_MSG_ID_CHECK_CRC             (3006 | 0xF000)
#define BOOT_MSG_ID_JUMP_APP              (3008 | 0xF000)
#define BOOT_MSG_ID_READ_APP_INFO         (3010 | 0xF000)
#define BOOT_MSG_ID_UPDATE_CMD_RESPONE    (BOOT_MSG_ID_UPDATE_CMD + 1)
#define BOOT_MSG_ID_FILE_INFO_RESPONE     (BOOT_MSG_ID_FILE_INFO + 1)
#define BOOT_MSG_ID_FILE_DATA_RESPONE     (BOOT_MSG_ID_FILE_DATA + 1)
#define BOOT_MSG_ID_CHECK_CRC_RESPONE     (BOOT_MSG_ID_CHECK_CRC + 1)
#define BOOT_MSG_ID_READ_APP_INFO_RESPONE (BOOT_MSG_ID_READ_APP_INFO + 1)

#define BOOT_MSG_ID_START_UPDATE             (3012 | 0xF000)
#define BOOT_MSG_ID_READ_UPDATE_INFO         (3014 | 0xF000)
#define BOOT_MSG_ID_START_UPDATE_RESPONE     (BOOT_MSG_ID_START_UPDATE + 1)
#define BOOT_MSG_ID_READ_UPDATE_INFO_RESPONE (BOOT_MSG_ID_READ_UPDATE_INFO + 1)

#define BOOT_MAGIC_NUM 0x12345678

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

/** APP 的信息字段: 头部插入 512 字节 */
typedef struct
{
    uint32_t magic;
    uint32_t board_id;
    uint32_t app_start_address;
    uint32_t app_size;
    uint32_t app_crc;
    uint32_t build_timestamp;
    uint32_t board_version;
    uint32_t firmware_version;

    char build_time[19];
    char app_git_commit[40];
    char app_name[50];
    char device_id;
} app_info_t;

enum boot_update_respone_e
{
    BOOT_UPDATE_RESPONE_OK = 1,
    BOOT_UPDATE_RESPONE_BOARD_ERR,
    BOOT_UPDATE_RESPONE_REPEAT,
    BOOT_UPDATE_RESPONE_OFFSET_ERR,
    BOOT_UPDATE_RESPONE_CHECK_CRC_ERR,
};

typedef struct
{
    uint8_t update_cmd;
    uint8_t obj_num;
    uint8_t obj_addr[OBJ_ADDR_MAX_NUM];
} cmd_update_t;

typedef struct
{
    uint8_t  respone;
    uint32_t offset;
} respone_file_data_t;

typedef struct
{
    uint32_t offset;
    uint16_t data_len;
    uint8_t  data[FILE_DATA_RECV_MAX_SIZE];
} file_data_t;
