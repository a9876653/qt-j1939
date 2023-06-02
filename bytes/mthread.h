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
    MThread(thread_task_t task = nullptr) : task(task)
    {
    }
    ~MThread()
    {
        stop();
    }

    void stop()
    {
        cycle_flag = false;
        quit();
        wait();
    }

private:
    void run() override
    {
        while (cycle_flag)
        {
            if (task != nullptr)
            {
                task();
            }
        }
    }

    bool          cycle_flag = true;
    thread_task_t task       = nullptr;
};

#endif // MTHREAD_H
