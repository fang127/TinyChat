#include "MySQLDao.h"
#include "ConfigMgr.h"

MySQLDao::MySQLDao()
{
    auto &cfg = ConfigMgr::getInstance();
    const std::string &host = cfg["Mysql"]["Host"];
    const std::string &port = cfg["Mysql"]["Port"];
    const std::string &passwd = cfg["Mysql"]["Passwd"];
    const std::string &schema = cfg["Mysql"]["Schema"];
    const std::string &user = cfg["Mysql"]["User"];
    pool_.reset(new MySQLPool(host + ":" + port, user, passwd, schema, 5));
}

MySQLDao::~MySQLDao() { pool_->close(); }

// 注册用户
int MySQLDao::regUser(const std::string &name,
                      const std::string &email,
                      const std::string &passwd)
{
    auto conn = pool_->getConnection();
    try
    {
        if (conn == nullptr)
        {
            return -1;
        }

        // 准备调用存储过程
        std::unique_ptr<sql::PreparedStatement> state(
            conn->connect_->prepareStatement("CALL regUser(?,?,?,@result)"));
        // 设置输入参数
        state->setString(1, name);
        state->setString(2, email);
        state->setString(3, passwd);

        // 由于prepareStatement不直接支持注册输出参数，所以需要使用会话变量或其他方法获取输出参数的值

        // 执行存储过程
        state->execute();
        // 如果存储过程设置了会话变量或有其他方法获取输出参数的值，可以在这里执行select查询获取他们
        /*
            ...add code
        */
        // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取:
        // start
        std::unique_ptr<sql::Statement> stateResult(
            conn->connect_->createStatement());
        std::unique_ptr<sql::ResultSet> result(
            stateResult->executeQuery("SELECT @result AS result"));
        if (result->next())
        {
            int res = result->getInt("result");
            std::cout << "Result: " << res << std::endl;
            pool_->returnConnection(std::move(conn));
            return res;
        }
        // end
        pool_->returnConnection(std::move(conn));
        return -1;
    }
    catch (sql::SQLException &e)
    {
        pool_->returnConnection(std::move(conn));
        std::cerr << "SQLException: " << e.what() << "\n";
        std::cerr << "(MySQL error code: " << e.getErrorCode() << " )\n";
        std::cerr << "(MySQL state: " << e.getSQLStateCStr() << " )\n";
        return -1;
    }
}

bool MySQLDao::checkEmail(const std::string &name, const std::string &email) {}

bool MySQLDao::updatePasswd(const std::string &name,
                            const std::string &newPassWd)
{
}

bool MySQLDao::checkPasswd(const std::string &name,
                           const std::string &passwd,
                           UserInfo &userInfo)
{
}