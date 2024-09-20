#pragma once

#include <memory>
#include <mutex>

namespace BASE
{

template <typename T>
class SingletonPtr
{
public:
    static std::shared_ptr<T> getInstancePtr()
    {
        std::call_once(once_, []{
            instance_.reset(new T);
        });

        return instance_;
    }

private:


    SingletonPtr() = default;
    ~SingletonPtr() = default;

private:
    static std::once_flag once_;
    static std::shared_ptr<T> instance_;
};

template <typename T>
std::once_flag SingletonPtr<T>::once_;

template <typename T>
std::shared_ptr<T> SingletonPtr<T>::instance_ = nullptr;



template <typename T>
class Singleton
{
public:
    static T* getInstance()
    {
        std::call_once(once_, []{
            instance_ = new T;
            atexit(destroy);
        });
    
        return instance_;
    }

private:
    static void destroy()
    {
        if (instance_)
        {
            delete instance_;
            instance_ = nullptr;
        }
        
    }
    Singleton() = default;
    ~Singleton() = default;

private:
    static std::once_flag once_;
    static T*  instance_;
};

template <typename T>
std::once_flag Singleton<T>::once_;

template <typename T>
T* Singleton<T>::instance_ = nullptr;



}