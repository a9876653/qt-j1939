#pragma once

#define SOCKET_CAN_FRAME_NUM_MAX 64
#define SOCKET_CAN_CMD_CFG       1
#define SOCKET_CAN_CMD_DATA      2
#define SOCKET_CAN_CMD_ACK       3

#define SOCKET_CAN_CHANNEL_OPEN  1
#define SOCKET_CAN_CHANNEL_CLOSE 2

#pragma pack(1)

typedef struct
{
    uint8_t  ch_ctrl;
    uint32_t baudrate;
} socket_can_config_t;

typedef struct
{
    uint8_t cmd;
    int     ack;
} socket_can_ack_t;

typedef struct
{
    uint16_t             frame_num;
    CanBase::can_frame_t data[SOCKET_CAN_FRAME_NUM_MAX];
} socket_can_data_t;

typedef struct
{
    uint8_t cmd;
    uint8_t device_index;
    uint8_t channel_index;
    union
    {
        socket_can_config_t cfg;
        socket_can_ack_t    ack;
        socket_can_data_t   data;
    };
} socket_can_t;

#pragma pack()
