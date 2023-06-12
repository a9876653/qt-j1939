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

    uint64_t ms = dateTime.toMSecsSinceEpoch();
    return ms;
}

timeMs_t sys_time_get_elapsed(timeMs_t start)
{
    return sys_get_ms() - start;
}

bool sys_time_is_elapsed(timeMs_t start, timeMs_t duration_ms)
{
    return (sys_time_get_elapsed(start) >= duration_ms);
}

QString timestamp_ms_to_qstring(uint64_t timestamp_ms)
{
    if (timestamp_ms == 0)
    {
        return QString("None");
    }
    struct tm    time;
    uint16_t     time_ms  = timestamp_ms % 1000;
    const time_t time_sec = timestamp_ms / 1000;
    gmtime_r(&time_sec, &time);
    time.tm_year = time.tm_year + 1900;
    time.tm_mon  = time.tm_mon + 1;

    return QString("%1/%2/%3-%4:%5:%6:%7")
        .arg(time.tm_year)
        .arg(time.tm_mon)
        .arg(time.tm_mday)
        .arg(time.tm_hour)
        .arg(time.tm_min)
        .arg(time.tm_sec)
        .arg(time_ms);
}

QString get_current_time_ms_qstring()
{
    struct tm    time;
    QDateTime    dateTime     = QDateTime::currentDateTime();
    uint64_t     timestamp_ms = dateTime.toMSecsSinceEpoch();
    uint16_t     time_ms      = timestamp_ms % 1000;
    const time_t time_sec     = timestamp_ms / 1000;
    gmtime_r(&time_sec, &time);
    time.tm_year = time.tm_year + 1900;
    time.tm_mon  = time.tm_mon + 1;

    return QString("%1/%2/%3-%4:%5:%6:%7")
        .arg(time.tm_year)
        .arg(time.tm_mon)
        .arg(time.tm_mday)
        .arg(time.tm_hour)
        .arg(time.tm_min)
        .arg(time.tm_sec)
        .arg(time_ms);
}
