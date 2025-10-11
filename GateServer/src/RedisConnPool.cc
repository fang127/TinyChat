#include "RedisConnPool.h"

RedisConnPool::RedisConnPool(std::size_t size,
                             const std::string &ip,
                             const int &port,
                             const std::string &password)
    : size_(size), ip_(ip), port_(port), stop_(false)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        auto *conn = redisConnect(ip_.c_str(), port_);
        if (conn == nullptr || conn->err != 0)
        {
            if (conn != nullptr)
            {
                redisFree(conn);
            }
            continue;
        }

        auto reply =
            (redisReply *)redisCommand(conn, "AUTH %s", password.c_str());
        if (reply->type == REDIS_REPLY_ERROR)
        {
            std::cout << "认证错误！" << std::endl;
            freeReplyObject(reply);
            redisFree(conn);
            continue;
        }

        freeReplyObject(reply);
        std::cout << "认证成功！" << std::endl;
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
    // 设置状态为关闭
    stop_ = true;
    // 通知所有阻塞的进程
    cond_.notify_all();
}

redisContext *RedisConnPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);
    /*
    当 connections_ 为空且 stop_ 为 false 时，当前线程会释放锁并阻塞，直到被
    notify 唤醒并且谓词为 true（即要么 stop_ 被置
    true，要么队列不空）。stop_被置为true则说明该连接池要关闭了，所以不用再阻塞了，直接返回nullptr
    */
    cond_.wait(lock,
               [this]
               {
                   if (stop_)
                   {
                       return true;
                   }
                   return !connections_.empty();
               });
    // 如果停止则直接返回空指针
    if (stop_)
    {
        return nullptr;
    }
    auto *context = connections_.front();
    connections_.pop();
    return context;
}

void RedisConnPool::returnConnection(redisContext *context)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (stop_)
    {
        return;
    }
    connections_.push(context);
    cond_.notify_one();
}