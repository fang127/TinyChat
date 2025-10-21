#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(std::size_t size)
    : sizePool_(size), ioService_(size), curUsedIoc_(0)
{
    for (std::size_t i = 0; i < sizePool_; ++i)
    {
        work_[i] = std::unique_ptr<Work>(new Work(ioService_[i]));
    }

    for (int i = 0; i < ioService_.size(); ++i)
    {
        threads_.emplace_back([this, i]() { ioService_[i].run(); });
    }
}

AsioIOServicePool::~AsioIOServicePool() { stop(); }

boost::asio::io_context &AsioIOServicePool::getIOService()
{
    // 使用 fetch_add 实现原子性的“先获取再加一”
    auto index = curUsedIoc_.fetch_add(1) % ioService_.size();
    return ioService_[index];
}

void AsioIOServicePool::stop()
{
    // 停止ioc
    for (auto &work : work_)
    {
        work->get_io_context().stop();
        work.reset();
    }

    // 主线程等待后台线程
    for (auto &t : threads_)
    {
        t.join();
    }
}
