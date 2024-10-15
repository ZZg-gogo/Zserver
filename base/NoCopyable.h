#pragma once


class NoCopyable    //禁止对象的拷贝和赋值
{

public:
    NoCopyable() = default;
    ~NoCopyable() = default;

    NoCopyable(const NoCopyable&) = delete;
    NoCopyable(const NoCopyable&&) = delete;
    NoCopyable& operator=(const NoCopyable&) = delete;
};


