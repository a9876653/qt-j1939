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

    void read_timeout();

    void slot_recv_read_event_cnt(int cnt);
    void slot_recv_read_event(read_event_respond_t respond);

    void on_readEventBtn_clicked();

    void on_readAllBtn_clicked();

    void on_saveBtn_clicked();

    void on_cleanBtn_clicked();

    void on_eventTypeComboBox_currentIndexChanged(int index);

private:
    void insert_item(int row, int column, QString s);

    void start_read(int start_index = 1, int stop_index = -1);
    void stop_read();

private:
    Ui::PageReadEvent *ui;
    int                src;
    J1939Event        *event;

    int read_index  = 0;
    int end_index   = 0;
    int timeout_cnt = 0;
    int read_type   = 0;

    const int read_timeout_ms = 500;

    QTimer read_timer;
};

#endif // PAGEREADEVENT_H
