#pragma once

#include "Data.h"
#include "MySQLDao.h"
#include "Singleton.h"

#include <memory>

// 应用层可调用的mysql管理类
class MySQLMgr : public Singleton<MySQLMgr>
{
    friend class Singleton<MySQLMgr>;

public:
    ~MySQLMgr() = default;

    // 注册用户
    int regUser(const std::string &name,
                const std::string &email,
                const std::string &passwd);

    bool checkEmail(const std::string &name, const std::string &email);

    bool updatePasswd(const std::string &name, const std::string &newPassWd);

    bool checkPasswd(const std::string &name,
                     const std::string &passwd,
                     UserInfo &userInfo);
    bool addFriendApply(const int &uid, const int &toUid);
    // 获取用户信息
    std::shared_ptr<UserInfo> getUser(int uid);
    std::shared_ptr<UserInfo> getUser(const std::string &name);

private:
    MySQLMgr();
    std::unique_ptr<MySQLDao> dao_;
};