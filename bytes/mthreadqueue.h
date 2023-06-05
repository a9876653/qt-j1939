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
        QMutexLocker locker(&m_mutex);
        m_max_size = max_size;
        m_queue.reserve(max_size);
    }
    // 添加元素到队列尾部
    void enqueue(const T &value)
    {
        if (!isFull())
        {
            QMutexLocker locker(&m_mutex);
            m_queue.enqueue(value);
            m_sem.release();
        }
        else
        {
            QMutexLocker locker(&m_mutex);
        }
    }

    // 从队列头部移除一个元素，并返回它
    bool dequeue(T &value)
    {
        m_sem.tryAcquire(1, -1);
        QMutexLocker locker(&m_mutex);
        if (m_queue.isEmpty())
        {
            return false;
        }
        value = m_queue.dequeue();
        return true;
    }

    // 返回队列是否为空
    bool isEmpty() const
    {
        QMutexLocker locker(&m_mutex);
        return m_queue.isEmpty();
    }

    bool isFull() const
    {
        QMutexLocker locker(&m_mutex);
        return m_queue.count() >= m_max_size;
    }

    int count()
    {
        QMutexLocker locker(&m_mutex);
        return m_queue.count();
    }

private:
    QQueue<T>      m_queue;
    mutable QMutex m_mutex;
    QSemaphore     m_sem;
    int            m_max_size;
};
