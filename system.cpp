#include "system.h"
#include "QThread"
#include "QDateTime"
void sys_delay_us(timeUs_t nUs)
{
    QThread::usleep(nUs);
}

void sys_delay_ms(timeMs_t nMs)
{
    QThread::msleep(nMs);
}

timeUs_t sys_get_us(void)
{
    QDateTime dateTime = QDateTime::currentDateTime();

    uint64_t ms = dateTime.toMSecsSinceEpoch() & 0xFFFFFFFF;
    return ms * 1000;
}

timeMs_t sys_get_ms(void)
{
    QDateTime dateTime = QDateTime::currentDateTime();

    uint64_t ms = dateTime.toMSecsSinceEpoch() & 0xFFFFFFFF;
    return ms;
}

timeMs_t sys_time_get_elapsed(uint32_t start)
{
    return sys_get_ms() - start;
}

bool sys_time_is_elapsed(uint32_t start, uint32_t duration_ms)
{
    return (sys_time_get_elapsed(start) >= duration_ms);
}
