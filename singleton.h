#pragma once

#include <mutex>

using std::mutex;

template <typename T>
class Singleton
{
public:
    static T *getInstance()
    {
        if(instance == nullptr)
        {
            mtx.lock();
            if(instance == nullptr)
            {
                instance = new T();
            }
            mtx.unlock();
        }
        return instance;
    }

    static void destory()
    {
        if(instance != nullptr)
        {
            delete instance;
            instance = nullptr;
        }
    }

private:
    static T *instance;
    static mutex mtx;

};

template <typename T>
T *Singleton<T>::instance = nullptr;

template <typename T>
mutex Singleton<T>::mtx;
