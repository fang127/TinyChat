#include "RedisConnPool.h"
#include <iostream>

RedisConnPool::RedisConnPool(std::size_t size,
                             const std::string &ip,
                             const int &port,
                             const std::string &passwd)
    : sizePool_(size), ip_(ip), port_(port), stop_(false)
{
    for (int i = 0; i < sizePool_; ++i)
    {
        auto *conn = redisConnect(ip.c_str(), port);
        if (conn == nullptr || conn->err != 0)
        {
            if (conn != nullptr)
            {
                redisFree(conn);
            }
            continue;
        }
        auto reply =
            (redisReply *)redisCommand(conn, "AUTH %s", passwd.c_str());
        if (reply->type == REDIS_REPLY_ERROR)
        {
            std::cout << "redis认证错误" << std::endl;
            freeReplyObject(reply);
            redisFree(conn);
            continue;
        }
        freeReplyObject(reply);
        std::cout << "redis认证成功" << std::endl;

        connections_.push(conn);
    }
}

RedisConnPool::~RedisConnPool()
{
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty())
    {
        redisFree(connections_.front());
        connections_.pop();
    }
}

void RedisConnPool::close()
{
    stop_ = true;
    cond_.notify_all();
}

redisContext *RedisConnPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    cond_.wait(lock,
               [this]()
               {
                   if (stop_)
                   {
                       return true;
                   }
                   return !connections_.empty();
               });

    if (stop_)
    {
        return nullptr;
    }

    auto *context = connections_.front();
    connections_.pop();
    return context;
}

void RedisConnPool::returnConnection(redisContext *conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (stop_)
    {
        redisFree(conn);
        return;
    }
    connections_.push(conn);
    cond_.notify_one();
}