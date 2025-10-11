#pragma once

#include "Const.h"

class RedisConnPool
{
public:
    RedisConnPool(std::size_t size,
                  const std::string &ip,
                  const int &port,
                  const std::string &password);

    ~RedisConnPool();

    void close();

    redisContext *getConnection();

    void returnConnection(redisContext *context);

private:
    std::size_t size_;     // 池的大小
    const std::string ip_; // ip地址
    const int port_;       // 端口号
    std::mutex mutex_;     // 互斥锁
    bool stop_;            // 连接池是否关闭，用来通知阻塞的进程
    std::queue<redisContext *> connections_; // 连接池
    std::condition_variable cond_;           // 条件变量，线程通信
};