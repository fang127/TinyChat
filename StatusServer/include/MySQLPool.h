#pragma once

#include "Const.h"

#include <cppconn/exception.h> // 定义了数据库操作的异常类​（如 sql::SQLException）。捕获和处理数据库操作中的错误（如连接失败、SQL语法错误、权限不足等）
#include <cppconn/prepared_statement.h> // 预编译语句执行器
#include <cppconn/resultset.h> // 查询结果集 接收SELECT语句的执行结果，提供遍历、获取字段值的方法
#include <cppconn/statement.h> // 普通SQL语句执行器
#include <mysql_connection.h>  // 数据库连接对象 管理连接，执行SQL语句
#include <mysql_driver.h> // 定义了MySQL驱动的核心接口（sql::mysql::MySQL_Driver类），是连接MySQL的“入口”
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