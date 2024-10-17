#pragma once

#include <semaphore.h>
#include <pthread.h>

#include "NoCopyable.h"

namespace BASE
{

class Semaphore : NoCopyable
{

public:
    Semaphore(int num = 0);
    ~Semaphore();

    void wait();
    void notify();
    int getValue();
private:
    sem_t sem_; //信号量
};


template <typename T>
class ScopedLock
{
public:
    ScopedLock(T& mutex) : mutex_(mutex)
    {
        mutex_.lock();
    }

    ~ScopedLock()
    {
        mutex_.unlock();
    }
private:
    T& mutex_;
};

class Mutex
{

};

template <typename T>
class ReadScopedLock
{
public:
    ReadScopedLock(T& mutex) : mutex_(mutex)
    {
        mutex_.readLock();
    }

    ~ReadScopedLock()
    {
        mutex_.unlock();
    }
private:
    T& mutex_;
};

template <typename T>
class WriteScopedLock
{
public:
    WriteScopedLock(T& mutex) : mutex_(mutex)
    {
        mutex_.writeLock();
    }

    ~WriteScopedLock()
    {
        mutex_.unlock();
    }
private:
    T& mutex_;
};


class RWmutex : NoCopyable
{
public:
    typedef WriteScopedLock<RWmutex>    wLock;
    typedef ReadScopedLock<RWmutex>     rLock;
public:
    RWmutex()
    {
        pthread_rwlock_init(&lock_, nullptr);
    }
    ~RWmutex()
    {
        pthread_rwlock_destroy(&lock_);
    }

    void readLock()
    {
        pthread_rwlock_rdlock(&lock_);
    }

    void writeLock()
    {
        pthread_rwlock_wrlock(&lock_);
    }

    void unlock()
    {
        pthread_rwlock_unlock(&lock_);
    }
private:
    pthread_rwlock_t lock_;
};




}//end namespace