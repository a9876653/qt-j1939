#ifndef M_OBJ_SIGNAL_H
#define M_OBJ_SIGNAL_H

#include <QObject>
#include "singleton.h"
#include "boot_port.h"

class MiddleSignal : public QObject
{
    Q_OBJECT
public:
    MiddleSignal(){};
    ~MiddleSignal()
    {
    }

    void dbg_info(const char *msg, ...)
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

    void app_info_update(app_info_str_t &str)
    {
        emit this->sig_app_info_update(str);
    }

    void update_failed()
    {
        emit this->sig_update_failed();
    }

signals:
    void sig_dbg_info(QString str);
    void sig_app_info_update(app_info_str_t &str);
    void sig_update_failed();

private:
};

#define MiddleSignalIns Singleton<MiddleSignal>::getInstance()
#endif // M_DBG_MODULES_H
