#ifndef PAGEPARAMS_H
#define PAGEPARAMS_H

#include <QWidget>
#include "paramshandle.h"

namespace Ui
{
class PageParams;
}

class PageParams : public QWidget
{
    Q_OBJECT

public:
    explicit PageParams();
    ~PageParams();
    ParamsHandle params;

private slots:
    void on_tableWidget_cellClicked(int row, int column);

    void on_requestPushButton_clicked();

    void slot_request_get_timeout();

    void slot_request_set_timeout();

    void on_setButton_clicked();

private:
    Ui::PageParams *ui;

    int request_get_index = 0;
    int request_set_index = 0;

    QTimer request_get_timer;
    QTimer request_set_timer;
};

#endif // PAGEPARAMS_H
