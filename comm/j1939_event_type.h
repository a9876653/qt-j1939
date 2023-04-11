#pragma once
#include "stdint.h"
typedef uint64_t event_timestamp_t;
typedef uint32_t event_id_t;

#define EVENT_SAVE_DATA_MAX_SIZE 8

typedef struct
{
    event_id_t        id;
    event_timestamp_t start_time;
    event_timestamp_t end_time;
    char              data[EVENT_SAVE_DATA_MAX_SIZE];
} err_event_t;

#define READ_EVENT_MAGIC        0x12345678
#define READ_EVENT_DES_BUF_SIZE 32
typedef struct
{
    uint32_t megic_num;
} read_event_cnt_request_t;

typedef struct
{
    uint32_t megic_num;
    uint32_t cnt;
} read_event_cnt_respond_t;

typedef struct
{
    uint32_t megic_num;
    uint32_t index;
} read_event_request_t;

#define EVENT_RESPOND_HEADER_LEN (4 + sizeof(err_event_t) + 1)

typedef struct
{
    uint32_t    index;
    err_event_t err_event;
    uint8_t     des_buf_len;
    uint8_t     des_buf[READ_EVENT_DES_BUF_SIZE];
} read_event_respond_t;
