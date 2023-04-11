#ifndef PAGEREADEVENT_H
#define PAGEREADEVENT_H

#include <QWidget>
#include "j1939_event.h"

namespace Ui
{
class PageReadEvent;
}

class PageReadEvent : public QWidget
{
    Q_OBJECT

public:
    explicit PageReadEvent(int src, QWidget *parent = nullptr);
    ~PageReadEvent();

private slots:
    void on_readEventCntBtn_clicked();

    void on_pushButton_clicked();

    void read_timeout();

    void slot_recv_read_event_cnt(int cnt);
    void slot_recv_read_event(read_event_respond_t respond);

private:
    void insert_item(int row, int column, QString s);

private:
    Ui::PageReadEvent *ui;
    int                src;
    J1939Event        *event;

    QTimer read_timer;
};

#endif // PAGEREADEVENT_H
