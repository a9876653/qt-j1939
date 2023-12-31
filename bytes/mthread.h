#ifndef MTHREAD_H
#define MTHREAD_H

#include <QObject>
#include <QThread>

class MThread : public QThread
{
    Q_OBJECT
public:
    typedef std::function<void(void)> thread_task_t;

public:
    MThread(thread_task_t task = nullptr, thread_task_t init = nullptr) : m_task(task), m_init(init)
    {
    }
    ~MThread()
    {
        stop();
    }

    void start()
    {
        if (!m_is_run)
        {
            m_is_run = true;
            QThread::start();
        }
    }

    void stop()
    {
        m_is_run = false;
        quit();
        wait();
    }

    bool is_run()
    {
        return m_is_run;
    }

private:
    void run() override
    {
        if (m_init)
        {
            m_init();
        }
        while (m_is_run)
        {
            if (m_task != nullptr)
            {
                m_task();
            }
        }
    }

    bool          m_is_run = false;
    thread_task_t m_task   = nullptr;
    thread_task_t m_init   = nullptr;
};

#endif // MTHREAD_H
