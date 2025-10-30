#include "MySQLMgr.h"

MySQLMgr::MySQLMgr() : dao_(std::make_unique<MySQLDao>()) {}

// 注册用户
int MySQLMgr::regUser(const std::string &name,
                      const std::string &email,
                      const std::string &passwd)
{
    return dao_->regUser(name, email, passwd);
}

bool MySQLMgr::checkEmail(const std::string &name, const std::string &email)
{
    return dao_->checkEmail(name, email);
}

bool MySQLMgr::updatePasswd(const std::string &name,
                            const std::string &newPassWd)
{
    return dao_->updatePasswd(name, newPassWd);
}

bool MySQLMgr::checkPasswd(const std::string &name,
                           const std::string &passwd,
                           UserInfo &userInfo)
{
    return dao_->checkPasswd(name, passwd, userInfo);
}

bool MySQLMgr::addFriendApply(const int &uid, const int &toUid)
{
    return dao_->addFriendApply(uid, toUid);
}

std::shared_ptr<UserInfo> MySQLMgr::getUser(int uid)
{
    return dao_->getUser(uid);
}

std::shared_ptr<UserInfo> MySQLMgr::getUser(const std::string &name)
{
    return dao_->getUser(name);
}

bool MySQLMgr::getApplyList(int uid,
                            std::vector<std::shared_ptr<ApplyInfo>> &list,
                            int begin,
                            int limit)
{
    return dao_->getApplyList(uid, list, begin, limit);
}

bool MySQLMgr::authFriendApply(const int &fromUid, const int &toUid)
{
    return dao_->authFriendApply(fromUid, toUid);
}

bool MySQLMgr::addFriend(const int &fromUid,
                         const int &toUid,
                         const std::string &backName)
{
    return dao_->addFriend(fromUid, toUid, backName);
}

bool MySQLMgr::getFriendList(int uid,
                             std::vector<std::shared_ptr<UserInfo>> &friendList)
{
    return dao_->getFriendList(uid, friendList);
}
