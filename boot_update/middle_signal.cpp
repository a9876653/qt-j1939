#include "middle_signal.h"

MiddleSignal::MiddleSignal()
{
}
MiddleSignal::~MiddleSignal()
{
}

void MiddleSignal::dbg_info(const char *msg, ...)
{
    char buff[512];
    memset(buff, 0, sizeof(buff));
    va_list args;
    va_start(args, msg);
    int len = vsnprintf(buff, sizeof(buff), msg, args);
    if (len > 0)
    {
        emit this->sig_dbg_info(QString(buff));
    }
}

void MiddleSignal::app_info_update(app_info_str_t &str)
{
    emit this->sig_app_info_update(str);
}

void MiddleSignal::update_failed()
{
    emit this->sig_update_failed();
}
