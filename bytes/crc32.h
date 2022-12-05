#ifndef __CRC32_H
#define __CRC32_H

#include "stdint.h"

uint64_t reflect(uint64_t ref, uint8_t ch);
uint32_t direct8_table_crc(uint8_t *ptr, int len, uint32_t *table);
uint32_t direct32_table_crc(uint8_t *ptr, int len, uint32_t *table);
uint32_t crc32_cal(uint8_t *ptr, int len);
#endif
