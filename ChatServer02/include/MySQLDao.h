#pragma once

#include <memory>
#include <string>

#include "Data.h"
#include "MySQLPool.h"

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
    bool addFriendApply(const int &uid, const int &toUid);
    // 获取用户信息
    std::shared_ptr<UserInfo> getUser(int uid);
    std::shared_ptr<UserInfo> getUser(const std::string &name);
    // 获取好友申请列表
    bool getApplyList(int uid,
                      std::vector<std::shared_ptr<ApplyInfo>> &list,
                      int begin,
                      int limit = 10);

private:
    std::unique_ptr<MySQLPool> pool_;
};