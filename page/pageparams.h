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
    explicit PageParams(int src_addr);
    ~PageParams();

    void load_json(QString path);

private slots:
    void on_tableWidget_cellClicked(int row, int column);

    void on_requestPushButton_clicked();

    void on_setButton_clicked();

    void on_autoCheckBox_stateChanged(int arg1);

private:
    int src_addr = 0;

    Ui::PageParams *ui;

    bool is_auto_get = false;

    ParamsHandle *params;

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
};

#endif // PAGEPARAMS_H
