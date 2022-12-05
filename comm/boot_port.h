#pragma once
#include "bytes/crc32.h"
#include "QMessageBox"
#include "QString"

typedef struct
{
    QString appName;
    QString boardId;
    QString startAddr;
    QString size;
    QString boardVer;
    QString firmwareVer;
    QString buildTime;
    QString gitCommit;
    QString start_address;
    QString crc;
} app_info_str_t;

typedef struct
{
    char           name[128];
    char           buffer[1024 * 1024];
    uint           len;
    app_info_str_t app_info;
} file_data_info_t;

void boot_port_init(void);

void boot_port_poll(void);

file_data_info_t *set_file_data_info(QString file_path);

void boot_port_start(uint8_t *obj_addr, uint8_t obj_num, file_data_info_t *data_info);
