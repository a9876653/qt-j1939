#ifndef MTHREADPERIODTASK_H
#define MTHREADPERIODTASK_H

#include <QObject>
#include <QThread>
#include <QTimer>

class MThreadPeriodTask : public QThread
{
    Q_OBJECT
public:
    typedef std::function<void(void)> thread_task_t;

public:
    MThreadPeriodTask(thread_task_t task = nullptr) : m_task(task)
    {
        m_timer = new QTimer;
        m_timer->stop();
        connect(m_timer, &QTimer::timeout, this, &MThreadPeriodTask::slot_period_task);
    }
    ~MThreadPeriodTask()
    {
        stop();
    }

    void start(int period_ms = 1)
    {
        m_is_run = true;
        m_timer->start(period_ms);
        QThread::start();
    }

    void stop()
    {
        m_is_run = false;
        m_timer->stop();
        quit();
        wait();
    }

    bool is_run()
    {
        return m_is_run;
    }

private slots:
    void slot_period_task()
    {
        if (m_task != nullptr)
        {
            m_task();
        }
    }

private:
    bool          m_is_run = false;
    thread_task_t m_task   = nullptr;
    int           m_period = 0;
    QTimer       *m_timer  = nullptr;
};

#endif // MTHREADPERIODTASK_H
