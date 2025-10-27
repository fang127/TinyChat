#pragma once

#include <atomic>
#include <condition_variable>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mutex>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <queue>
#include <thread>

class SQLConnection
{
public:
    SQLConnection(sql::Connection *con, int64_t lastTime)
        : connect_(con), lastOperTime_(lastTime)
    {
    }

    ~SQLConnection() = default;

    std::unique_ptr<sql::Connection> connect_;
    int64_t lastOperTime_; // 时间确保不会空占连接
};

class MySQLPool
{
public:
    MySQLPool(const std::string &url,
              const std::string &user,
              const std::string &passwd,
              const std::string &schema,
              std::size_t poolSize);

    ~MySQLPool();

    // 检查时间戳，发送请求保证连接存活,保活机制
    void checkConnection();

    // 获取连接池中的一个连接
    std::unique_ptr<SQLConnection> getConnection();

    // 返回连接
    void returnConnection(std::unique_ptr<SQLConnection> conn);

    // 关闭连接池
    void close();

private:
    std::string url_;      // 和数据库连接使用的url
    std::string user_;     // 用户名
    std::string passwd_;   // 密码
    std::string schema_;   // 数据库名
    std::size_t poolSize_; // 池大小
    std::queue<std::unique_ptr<SQLConnection>> pool_;
    std::mutex mutex_;             // 互斥锁
    std::condition_variable cond_; // 信号量
    // 池子状态，有用为flase，关闭为true，结合cond_通知其他线程
    std::atomic<bool> bStop_;
    // 监测线程，配合SQLConnection中的lastOperTime_使用，当时间大于某个值，则执行简单的请求，告诉mysql该连接还活着，心跳机制，避免长时间不使用，被mysql服务器主动断开
    std::thread checkThread_;
};