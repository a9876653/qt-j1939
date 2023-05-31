#ifndef PAGEWIDGETSCOLLECT_H
#define PAGEWIDGETSCOLLECT_H

#include <QWidget>
#include <QTimer>
#include <QMap>
#include "dataobjmap.h"

namespace Ui
{
class PageWidgetsCollect;
}

class PageWidgetsCollect : public QWidget
{
    Q_OBJECT

public:
    explicit PageWidgetsCollect(DataObjMap *data);
    ~PageWidgetsCollect();

    void json_items_handle(QJsonDocument *jdoc);
signals:
    void sig_request_read_reg(uint16_t reg_addr, uint16_t reg_len);

private:
    void auto_read_start();
    void auto_read_stop();

    void slot_auto_read_timeout();

private:
    Ui::PageWidgetsCollect *ui;
    class RegInfo
    {
    public:
        RegInfo(uint16_t reg_addr, uint16_t reg_len, bool is_auto_read = true)
            : reg_addr(reg_addr), reg_len(reg_len), is_auto_read(is_auto_read)
        {
        }
        uint16_t reg_addr;
        uint16_t reg_len;
        bool     is_auto_read = true;
    };

    DataObjMap *data;

    int update_period     = 0;
    int auto_read_index   = 0;
    int auto_read_offset  = 0;
    int is_auto_read      = true;
    int request_get_index = 0;

    QList<RegInfo>       reg_info_list;
    QMap<int, RegInfo *> reg_map;

    QTimer auto_read_timer;

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
private slots:
    void on_checkBox_stateChanged(int arg1);
    void on_updatePeriodSpinBox_valueChanged(int arg1);
};

#endif // PAGEWIDGETSCOLLECT_H
