#pragma once
#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QDebug>
#include <QSemaphore>

// 定义线程安全队列类
template <typename T> class ThreadSafeQueue
{
public:
    ThreadSafeQueue(int max_size = 4096)
    {
        set_max_size(max_size);
    }

    void set_max_size(int max_size)
    {
        m_mutex.lock();
        m_max_size = max_size;
        m_queue.reserve(max_size);
        m_mutex.unlock();
    }
    // 添加元素到队列尾部
    void enqueue(const T &value)
    {
        if (!isFull())
        {
            m_mutex.lock();
            m_queue.enqueue(value);
            m_sem.release();
            m_mutex.unlock();
        }
    }

    // 从队列头部移除一个元素，并返回它
    bool dequeue(T &value, int timeout = -1)
    {
        if (m_sem.tryAcquire(1, timeout))
        {
            m_mutex.lock();
            value = m_queue.dequeue();
            m_mutex.unlock();
            return true;
        }
        return false;
    }

    // 返回队列是否为空
    bool isEmpty() const
    {
        return m_queue.isEmpty();
    }

    bool isFull() const
    {
        return m_queue.count() >= m_max_size;
    }

    int count()
    {
        return m_queue.count();
    }

private:
    QQueue<T>  m_queue;
    QMutex     m_mutex;
    QSemaphore m_sem;
    int        m_max_size;
};
