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


class SpinLock : NoCopyable  
{
public:
    typedef ScopedLock<SpinLock> Lock;
public:
    SpinLock()
    {
        pthread_spin_init(&mutex_, 0);
    }

    ~SpinLock()
    {
        pthread_spin_destroy(&mutex_);
    }

    void lock()
    {
        pthread_spin_lock(&mutex_);
    }

    void unlock()
    {
        pthread_spin_unlock(&mutex_);
    }
private:
    pthread_spinlock_t mutex_;
};


class Mutex : NoCopyable
{
public:
    typedef ScopedLock<Mutex> Lock;
public:
    Mutex()
    {
        pthread_mutex_init(&mutex_, nullptr);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&mutex_);
    }

    void lock()
    {
        pthread_mutex_lock(&mutex_);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex_);
    }
private:
    pthread_mutex_t mutex_;
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