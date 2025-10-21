#pragma once

#include <iostream>
#include <memory>
#include <mutex>

// 奇异递归模板实现单例
template <typename T> class Singleton
{
public:
    ~Singleton() { std::cout << "this is Singleton destruct" << std::endl; }

    // 获取唯一实例
    static std::shared_ptr<T> getInstance()
    {
        static std::once_flag flag;
        std::call_once(flag, [&]() { instance_ = std::shared_ptr<T>(new T); });
        return instance_;
    }
    // 获取实例地址
    void printAddress() const { std::cout << instance_.get() << std::endl; }

protected:
    Singleton() = default;
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

    static std::shared_ptr<T> instance_;
};

template <typename T> std::shared_ptr<T> Singleton<T>::instance_ = nullptr;