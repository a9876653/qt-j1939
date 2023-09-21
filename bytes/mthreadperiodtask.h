#ifndef MTHREADPERIODTASK_H
#define MTHREADPERIODTASK_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QDebug>

class MThreadPeriodTask : public QThread
{
    Q_OBJECT
public:
    typedef std::function<void(void)> thread_task_t;

public:
    MThreadPeriodTask(thread_task_t task = nullptr) : m_task(task)
    {
        this->moveToThread(this);
        // QThread::start();
        connect(this, &MThreadPeriodTask::sig_start, this, &MThreadPeriodTask::slot_start, Qt::QueuedConnection);
        connect(this, &MThreadPeriodTask::sig_stop, this, &MThreadPeriodTask::slot_stop, Qt::QueuedConnection);
    }
    ~MThreadPeriodTask()
    {
        stop();
    }

    void start(int period_ms = 1)
    {
        QThread::start();
        emit sig_start(period_ms);
    }

    void stop()
    {
        emit sig_stop();
    }

    bool is_run()
    {
        return m_is_run;
    }

signals:
    void sig_start(int period_ms = 1);
    void sig_stop();

private slots:
    void slot_period_task()
    {
        if (m_task != nullptr)
        {
            m_task();
        }
    }

    void slot_start(int period_ms = 1)
    {
        qDebug() << "MThreadPeriodTask start thread ID:" << QThread::currentThreadId();
        m_timer = new QTimer;
        connect(m_timer, &QTimer::timeout, this, &MThreadPeriodTask::slot_period_task);
        m_timer->start(period_ms);
        m_is_run = true;
    }

    void slot_stop()
    {
        if (m_timer != nullptr)
        {
            m_timer->stop();
            delete m_timer;
            m_timer = nullptr;
        }
        m_is_run = false;
        quit();
        wait();
    }

private:
    bool          m_is_run = false;
    thread_task_t m_task   = nullptr;
    int           m_period = 0;
    QTimer       *m_timer  = nullptr;
};

#endif // MTHREADPERIODTASK_H
