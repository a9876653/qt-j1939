#pragma once

typedef uint64_t event_timestamp_t;
typedef uint32_t event_id_t;

#define EVENT_SAVE_DATA_MAX_SIZE 8

#define ERR_EVENT_FLAG          (0x8000) // 故障事件记录码
#define ERR_EVENT_ID_DECODE(id) (id & (~ERR_EVENT_FLAG))

#define READ_ERR_EVENT_MAGIC    0x45525200 // ERR\0
#define READ_INFO_EVENT_MAGIC   0x494E464F // INFO
#define READ_EVENT_DES_BUF_SIZE 32
#define EVENT_RESPOND_DATA_LEN  59 // 64-4-1

#define SYS_EVENT_CELL_ID               (-1)
#define CELL_EVENT_ID_INDEX_OFFSET      (1000)
#define CELL_EVENT_ID_CONVER(id)        ((id) % CELL_EVENT_ID_INDEX_OFFSET)
#define CELL_EVENT_INDEX_CONVER(id)     ((id) / CELL_EVENT_ID_INDEX_OFFSET)
#define CELL_EVENT_ID_FORMAT(index, id) ((id) + (index + 1) * CELL_EVENT_ID_INDEX_OFFSET)

#define EVENT_ID_FORMAT(index, id) (((index) >= 0) ? CELL_EVENT_ID_FORMAT(index, id) : id)

#define IS_CELL_EVENT(id) ((id) > CELL_EVENT_ID_INDEX_OFFSET)

#define SYS_SAVE_DATA_MAX_SIZE   64 // 最多不可超过64字节
#define INFO_SAVE_DATA_MAX_SIZE  (SYS_SAVE_DATA_MAX_SIZE)
#define EVENT_SAVE_DATA_MAX_SIZE 8

enum
{
    SYS_EVENT_TYPE_ERR  = 0,
    SYS_EVENT_TYPE_INFO = 1,
};

#pragma pack(push) //保存对齐
#pragma pack(1)
typedef struct
{
    event_id_t        id;
    event_timestamp_t start_time;
    event_timestamp_t end_time;
    char              data[EVENT_SAVE_DATA_MAX_SIZE];
} err_event_t;

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

typedef struct
{
    uint32_t index;
    uint8_t  data[EVENT_RESPOND_DATA_LEN];
} read_event_respond_t;

typedef struct
{
    char data[INFO_SAVE_DATA_MAX_SIZE];
} info_event_t;

typedef struct
{
    event_timestamp_t start_time;
    info_event_t      info;
} info_event_respond_t;

typedef union
{
    event_id_t   id;
    err_event_t  err_event;
    info_event_t info_event;
    uint8_t      bytes[SYS_SAVE_DATA_MAX_SIZE];
} sys_event_union;

#pragma pack(pop)
