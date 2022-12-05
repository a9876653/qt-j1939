#pragma once

#define J1939_USER_CONFIG
#include "system.h"
#include "stdlib.h"

#define J1939_MALLOC   malloc
#define J1939_FREE     free
#define J1939_GET_MS() sys_get_ms()
