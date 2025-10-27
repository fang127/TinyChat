#include "MySQLPool.h"
#include "Defer.h"

MySQLPool::MySQLPool(const std::string &url,
                     const std::string &user,
                     const std::string &passwd,
                     const std::string &schema,
                     size_t poolSize)
    : url_(url), user_(user), passwd_(passwd), schema_(schema),
      poolSize_(poolSize), bStop_(false)
{
    try
    {
        // 初始化连接池
        for (std::size_t i = 0; i < poolSize_; ++i)
        {
            // 创建连接
            sql::mysql::MySQL_Driver *driver =
                sql::mysql::get_mysql_driver_instance();
            sql::Connection *conn = driver->connect(url_, user_, passwd_);
            // 选择数据库
            conn->setSchema(schema_);
            // 获取当前时间戳
            auto currentTime =
                std::chrono::system_clock::now().time_since_epoch();
            // 转换为秒
            unsigned long long timeStamp =
                std::chrono::duration_cast<std::chrono::seconds>(currentTime)
                    .count();
            // 加入到连接池
            pool_.push(std::make_unique<SQLConnection>(conn, timeStamp));
        }

        // 创建检查线程
        checkThread_ = std::thread(
            [this]()
            {
                while (!bStop_)
                {
                    // 当池子还存在则执行保活机制
                    checkConnection();
                    std::this_thread::sleep_for(std::chrono::seconds(600));
                }
            });

        // 后台执行
        checkThread_.detach();
    }
    catch (const sql::SQLException &e)
    {
        std::cout << "mysql pool init failed, error is " << e.what() << "\n";
    }
}

MySQLPool::~MySQLPool()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (!pool_.empty())
    {
        pool_.pop();
    }
}

void MySQLPool::checkConnection()
{
    std::lock_guard<std::mutex> lock(mutex_);
    int poolSize = pool_.size();
    // 获取当前时间
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    // 转为秒
    auto timeStamp =
        std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();

    // 检查所有连接
    for (int i = 0; i < poolSize; ++i)
    {
        auto conn = std::move(pool_.front());
        pool_.pop();
        // golang特性，Defer函数对象，在析构函数中执行push的操作，确保每一个连接用完了后归还连接池
        Defer defer([this, &conn]() { pool_.push(std::move(conn)); });

        // 10分钟
        if (timeStamp - conn->lastOperTime_ < 600)
        {
            continue;
        }

        try
        {
            // 发送简单请求保活
            std::unique_ptr<sql::Statement> state(
                conn->connect_->createStatement());
            state->executeQuery("SELECT 1");
            conn->lastOperTime_ = timeStamp;
            std::cout << "execute time alive query, cur is " << timeStamp
                      << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error keeping connection alive: " << e.what() << "\n";
            // 重新创建连接并替换旧的连接
            sql::mysql::MySQL_Driver *driver =
                sql::mysql::get_mysql_driver_instance();
            auto *newConn = driver->connect(url_, user_, passwd_);
            newConn->setSchema(schema_);
            conn->connect_.reset(newConn);
            conn->lastOperTime_ = timeStamp;
        }
    }
}

std::unique_ptr<SQLConnection> MySQLPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    // 只有池子不为空并且没有关闭才返回
    cond_.wait(lock,
               [this]() -> bool
               {
                   if (bStop_)
                   {
                       return true;
                   }

                   return !pool_.empty();
               });

    if (bStop_)
    {
        return nullptr;
    }

    std::unique_ptr<SQLConnection> conn(std::move(pool_.front()));
    pool_.pop();
    return conn;
}

// 返回连接
void MySQLPool::returnConnection(std::unique_ptr<SQLConnection> conn)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (bStop_)
    {
        return;
    }
    pool_.push(std::move(conn));
    // 唤醒阻塞等待的线程
    cond_.notify_one();
}

// 关闭连接池
void MySQLPool::close()
{
    bStop_ = true;
    // 唤醒所有阻塞的线程不用等待了
    cond_.notify_all();
}