#pragma once

#define J1939_MALLOC   malloc
#define J1939_FREE     free
#define J1939_GET_MS() sys_get_ms()
#define sys_delay_ms(n)
#define sys_get_ms() 10
