#pragma once

#include <memory>
#include <string>

#include "MySQLPool.h"

// 用户信息
struct UserInfo
{
    std::string name_;
    std::string passwd_;
    int uid_;
    std::string email_;
};

// mysql实际的管理类，封装mysql的增删改查
class MySQLDao
{
public:
    MySQLDao();
    ~MySQLDao();

    // 注册用户
    int regUser(const std::string &name,
                const std::string &email,
                const std::string &passwd);

    bool checkEmail(const std::string &name, const std::string &email);

    bool updatePasswd(const std::string &name, const std::string &newPassWd);

    bool checkPasswd(const std::string &name,
                     const std::string &passwd,
                     UserInfo &userInfo);

private:
    std::unique_ptr<MySQLPool> pool_;
};