#ifndef M_OBJ_SIGNAL_H
#define M_OBJ_SIGNAL_H

#include <QObject>
#include "singleton.h"
#include "boot_port.h"

class MiddleSignal : public QObject
{
    Q_OBJECT
public:
    MiddleSignal();
    ~MiddleSignal();

    void dbg_info(const char *msg, ...);

    void app_info_update(app_info_str_t &str);

    void update_failed();

signals:
    void sig_dbg_info(QString str);
    void sig_app_info_update(app_info_str_t str);
    void sig_update_failed();

private:
};

#define MiddleSignalIns Singleton<MiddleSignal>::getInstance()
#endif // M_DBG_MODULES_H
