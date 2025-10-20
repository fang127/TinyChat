#pragma once

#include <Singleton.h>

#include <atomic>
#include <boost/asio.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

// 基于asio的TCP连接池
class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work;
    using WorkPtr = std::shared_ptr<Work>;

    AsioIOServicePool(const AsioIOServicePool &) = delete;
    AsioIOServicePool &operator=(const AsioIOServicePool &) = delete;

    ~AsioIOServicePool();

    // 轮询获得ioc
    boost::asio::io_context &getIOService();
    void stop();

private:
    AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());

    std::size_t sizePool_;
    std::vector<IOService> ioService_;
    std::vector<WorkPtr> work_;
    std::vector<std::thread> thread_;
};