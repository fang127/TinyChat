#include "MySQLDao.h"
#include "ConfigMgr.h"
#include "Defer.h"

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
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
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
            return res;
        }
        // end
        return -1;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what() << "\n";
        std::cerr << "(MySQL error code: " << e.getErrorCode() << " )\n";
        std::cerr << "(MySQL state: " << e.getSQLStateCStr() << " )\n";
        return -1;
    }
}

bool MySQLDao::checkEmail(const std::string &name, const std::string &email)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {
        if (conn == nullptr)
        {
            return false;
        }

        // 准备查询语句
        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "SELECT email FROM user WHERE name = ?"));
        // 绑定参数
        prepareState->setString(1, name);
        // 执行查询
        std::unique_ptr<sql::ResultSet> result(prepareState->executeQuery());

        // 遍历结果
        while (result->next())
        {
            std::cout << "check email: " << result->getString("email")
                      << std::endl;
            if (email != result->getString("email"))
            {
                return false;
            }
            return true;
        }
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

bool MySQLDao::updatePasswd(const std::string &name,
                            const std::string &newPassWd)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {

        if (conn == nullptr)
        {
            return false;
        }

        // 准备查询语句
        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "UPDATE user SET passwd = ? WHERE name = ?"));
        // 绑定参数
        prepareState->setString(1, newPassWd);
        prepareState->setString(2, name);

        // 执行
        int updateCount = prepareState->executeUpdate();
        std::cout << "Updated counts of rows: " << updateCount << std::endl;
        return true;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

bool MySQLDao::checkPasswd(const std::string &email,
                           const std::string &passwd,
                           UserInfo &userInfo)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {
        if (conn == nullptr)
        {
            return false;
        }

        std::unique_ptr<sql::PreparedStatement> prepareStatement(
            conn->connect_->prepareStatement(
                "SELECT * FROM user WHERE email = ?"));

        prepareStatement->setString(1, email);

        std::unique_ptr<sql::ResultSet> result(
            prepareStatement->executeQuery());
        std::string originPasswd = "";
        while (result->next())
        {
            originPasswd = result->getString("passwd");
            std::cout << "Passwd: " << originPasswd << std::endl;
            break;
        }

        if (passwd != originPasswd)
        {
            return false;
        }

        userInfo.name_ = result->getString("name");
        userInfo.email_ = result->getString("email");
        userInfo.uid_ = result->getInt("uid");
        userInfo.passwd_ = originPasswd;
        return true;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

bool MySQLDao::addFriendApply(const int &uid, const int &toUid)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });

    try
    {
        if (conn == nullptr)
        {
            return false;
        }

        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "INSERT INTO friend_apply (from_uid, to_uid) VALUES (?, ?)"
                "ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = "
                "to_uid"));
        prepareState->setInt(1, uid);
        prepareState->setInt(2, toUid);

        int rowAffected = prepareState->executeUpdate();
        if (rowAffected < 0)
        {
            return false;
        }

        return true;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }

    return true;
}

std::shared_ptr<UserInfo> MySQLDao::getUser(int uid)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {
        if (conn == nullptr)
        {
            return nullptr;
        }
        // 准备执行器
        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "SELECT * FROM user WHERE uid = ?"));
        prepareState->setInt(1, uid);
        // 执行
        std::unique_ptr<sql::ResultSet> result(prepareState->executeQuery());
        std::shared_ptr<UserInfo> userPtr = nullptr;
        // 遍历结果集
        while (result->next())
        {
            userPtr.reset(new UserInfo);
            userPtr->passwd_ = result->getString("passwd");
            userPtr->email_ = result->getString("email");
            userPtr->name_ = result->getString("name");
            userPtr->nick_ = result->getString("nick");
            userPtr->desc_ = result->getString("desc");
            userPtr->sex_ = result->getInt("sex");
            userPtr->icon_ = result->getString("icon");
            userPtr->uid_ = uid;
            break;
        }
        return userPtr;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return nullptr;
    }
}

std::shared_ptr<UserInfo> MySQLDao::getUser(const std::string &name)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {
        if (conn == nullptr)
        {
            return nullptr;
        }
        // 准备执行器
        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "SELECT * FROM user WHERE name = ?"));
        prepareState->setString(1, name);
        // 执行
        std::unique_ptr<sql::ResultSet> result(prepareState->executeQuery());
        std::shared_ptr<UserInfo> userPtr = nullptr;
        // 遍历结果集
        while (result->next())
        {
            userPtr.reset(new UserInfo);
            userPtr->passwd_ = result->getString("passwd");
            userPtr->email_ = result->getString("email");
            userPtr->name_ = result->getString("name");
            userPtr->nick_ = result->getString("nick");
            userPtr->desc_ = result->getString("desc");
            userPtr->sex_ = result->getInt("sex");
            userPtr->icon_ = result->getString("icon");
            userPtr->uid_ = result->getInt("uid");
            break;
        }

        return userPtr;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return nullptr;
    }
}

bool MySQLDao::getApplyList(int uid,
                            std::vector<std::shared_ptr<ApplyInfo>> &list,
                            int begin,
                            int limit)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {
        if (conn == nullptr)
        {
            return false;
        }
        // 准备执行器
        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "SELECT apply.from_uid, apply.status, user.name, "
                "user.nick, user.sex from friend_apply as apply join user on "
                "apply.from_uid = user.uid where apply.to_uid = ? "
                "and apply.id > ? order by apply.id ASC LIMIT ?"));
        prepareState->setInt(1, uid);   // 将uid替换为你要查询的uid
        prepareState->setInt(2, begin); // 起始id
        prepareState->setInt(3, limit); // 偏移量
        // 执行查询
        std::unique_ptr<sql::ResultSet> res(prepareState->executeQuery());
        // 遍历结果集
        while (res->next())
        {
            auto name = res->getString("name");
            auto uid = res->getInt("from_uid");
            auto status = res->getInt("status");
            auto nick = res->getString("nick");
            auto sex = res->getInt("sex");
            auto apply_ptr = std::make_shared<ApplyInfo>(uid, name, "", "",
                                                         nick, sex, status);
            list.push_back(apply_ptr);
        }
        return true;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

bool MySQLDao::authFriendApply(const int &fromUid, const int &toUid)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {
        if (conn == nullptr)
        {
            return false;
        }
        // 准备执行器
        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "UPDATE friend_apply SET status = 1 "
                "WHERE from_uid = ? AND to_uid = ?"));
        prepareState->setInt(1, toUid);
        prepareState->setInt(2, fromUid);
        // 执行更新
        int rowAffected = prepareState->executeUpdate();
        if (rowAffected < 0)
        {
            return false;
        }
        return true;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

bool MySQLDao::addFriend(const int &fromUid,
                         const int &toUid,
                         const std::string &backName)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {
        if (conn == nullptr)
        {
            return false;
        }
        // 开始事务
        conn->connect_->setAutoCommit(false);
        // 准备第一个SQL语句, 插入认证方好友数据
        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "INSERT IGNORE INTO friend(self_id, friend_id, back) "
                "VALUES (?, ?, ?) "));
        prepareState->setInt(1, fromUid);
        prepareState->setInt(2, toUid);
        prepareState->setString(3, backName);

        // 执行更新
        int rowAffected = prepareState->executeUpdate();
        if (rowAffected < 0)
        {
            conn->connect_->rollback();
            return false;
        }

        // 准备第二个SQL语句，插入申请方好友数据
        std::unique_ptr<sql::PreparedStatement> prepareState2(
            conn->connect_->prepareStatement(
                "INSERT IGNORE INTO friend(self_id, friend_id, back) "
                "VALUES (?, ?, ?) "));
        // 反过来的申请时from，验证时to
        prepareState2->setInt(1, toUid); // from id
        prepareState2->setInt(2, fromUid);
        prepareState2->setString(3, "");
        // 执行更新
        int rowAffected2 = prepareState2->executeUpdate();
        if (rowAffected2 < 0)
        {
            conn->connect_->rollback();
            return false;
        }

        // 提交事务
        conn->connect_->commit();
        std::cout << "addfriend insert friends success" << std::endl;

        return true;
    }
    catch (const sql::SQLException &e)
    {
        // 如果发生错误，回滚事务
        if (conn)
        {
            conn->connect_->rollback();
        }
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

bool MySQLDao::getFriendList(int uid,
                             std::vector<std::shared_ptr<UserInfo>> &friendList)
{
    auto conn = pool_->getConnection();
    Defer defer([this, &conn]() { pool_->returnConnection(std::move(conn)); });
    try
    {
        if (conn == nullptr)
        {
            return false;
        }
        // 准备执行器
        std::unique_ptr<sql::PreparedStatement> prepareState(
            conn->connect_->prepareStatement(
                "select * from friend where self_id = ? "));
        prepareState->setInt(1, uid);
        // 执行查询
        std::unique_ptr<sql::ResultSet> res(prepareState->executeQuery());
        // 遍历结果集
        while (res->next())
        {
            auto friendID = res->getInt("friend_id");
            auto back = res->getString("back");
            // 再一次查询friend_id对应的信息
            auto userInfo = getUser(friendID);
            if (userInfo == nullptr)
            {
                continue;
            }

            userInfo->back_ = userInfo->name_;
            friendList.push_back(userInfo);
        }
        return true;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "SQLException: " << e.what()
                  << " (MySQL error code: " << e.getErrorCode()
                  << " , SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}
