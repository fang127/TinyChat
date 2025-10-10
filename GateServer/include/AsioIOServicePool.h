#pragma once

#include "Singleton.h"

#include <boost/asio.hpp>
#include <vector>

class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
    friend Singleton<AsioIOServicePool>;

public:
    using IOService = boost::asio::io_context;
    // work避免ioc没有绑定监听事件时自动退出，设置work当没有监听的连接时，不会退出，会一直轮询
    using Work = boost::asio::io_context::work;
    using WorkPtr = std::unique_ptr<Work>;

    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool &) = delete;
    AsioIOServicePool &operator=(const AsioIOServicePool &) = delete;

    // 使用 round-robin 的方式返回一个 io_service
    boost::asio::io_context &getIOService();
    void stop();

private:
    AsioIOServicePool(
        std::size_t size = 2 /*std::thread::hardware_concurrency()*/);

    std::vector<IOService> ioServices_;
    std::vector<WorkPtr> works_;
    std::vector<std::thread> threads_;
    std::size_t nextIOService_;
};