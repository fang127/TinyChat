#pragma once

#include <atomic>
#include <condition_variable>
#include <hiredis/hiredis.h>
#include <mutex>
#include <queue>
#include <string>

class RedisConnPool
{
public:
    RedisConnPool(std::size_t size,
                  const std::string &ip,
                  const int &port,
                  const std::string &passwd);

    ~RedisConnPool();

    void close();

    redisContext *getConnection();

    void returnConnection(redisContext *conn);

private:
    std::size_t sizePool_;
    const std::string &ip_;
    const int port_;
    std::queue<redisContext *> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> stop_;
};