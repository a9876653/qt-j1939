#pragma once

#include <QObject>
#include "comm_j1939.h"
#include "singleton.h"
#include "j1939_event_type.h"

class J1939Event : public QObject
{
    Q_OBJECT
public:
    J1939Event(int src);
    ~J1939Event();

    void request_event_cnt();
    void request_event(int index);
    void request_event_format();

signals:
    void sig_recv_event_cnt(int cnt);
    void sig_recv_event(read_event_respond_t respond);

public:
    int src = 1;
};

J1939Event *get_j1939_event_ins(int src);

void j1939_event_init();
void j1939_recv_read_event(int src, read_event_respond_t *respond);
