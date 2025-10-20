#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(
    std::size_t size = std::thread::hardware_concurrency())
    : sizePool_(size), ioService_(size)
{
}

AsioIOServicePool::~AsioIOServicePool() {}

boost::asio::io_context &AsioIOServicePool::getIOService() {}

void AsioIOServicePool::stop() {}
