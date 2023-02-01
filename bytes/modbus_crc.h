#pragma once
#include "stdint.h"

uint16_t modbus_crc16(uint8_t *buff, uint16_t len);