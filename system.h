#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <QString>

typedef int32_t  timeDelta_t;
typedef uint64_t timeMs_t;
typedef uint64_t timeUs_t;

static inline timeDelta_t cmp_time_us(timeUs_t a, timeUs_t b)
{
    return (timeDelta_t)(a - b);
}

#define AP_NSEC_PER_SEC  1000000000ULL
#define AP_NSEC_PER_USEC 1000ULL
#define AP_USEC_PER_SEC  1000000ULL
#define AP_USEC_PER_MSEC 1000ULL
#define AP_MSEC_PER_SEC  1000ULL
#define AP_SEC_PER_WEEK  (7ULL * 86400ULL)
#define AP_MSEC_PER_WEEK (AP_SEC_PER_WEEK * AP_MSEC_PER_SEC)

void     sys_delay_us(timeUs_t nUs);
void     sys_delay_ms(timeMs_t nMs);
timeUs_t sys_get_us(void);
timeMs_t sys_get_ms(void);

QString  timestamp_ms_to_qstring(uint64_t timestamp_ms);
QString  get_current_time_ms_qstring();
bool     sys_time_is_elapsed(timeMs_t start, timeMs_t duration_ms);
timeMs_t sys_time_get_elapsed(timeMs_t start);
