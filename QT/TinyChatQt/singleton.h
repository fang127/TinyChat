#ifndef SINGLETON_H
#define SINGLETON_H

#include <global.h>

template <typename T>
class Singleton
{
protected:
    Singleton() = default;
    Singleton(const Singleton<T> &) = delete;
    Singleton &operator=(const Singleton<T> &) = delete;
    static std::shared_ptr<T> instance_; // 唯一实例
public:
    static std::shared_ptr<T> getInstance_()
    {
        // once_flag配合call_once实现单例,只有在第一次调用该函数才会创建实例
        static std::once_flag sFlag;
        std::call_once(sFlag,[&](){
           instance_ = std::shared_ptr<T>(new T);
        });

        return instance_;
    }

    void printAddress()
    {
        std::cout << instance_.get() << std::endl;
    }

    ~Singleton()
    {
        std::cout << "this is singleton destruct" << std::endl;
    }
};

template <typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;

#endif // SINGLETON_H
