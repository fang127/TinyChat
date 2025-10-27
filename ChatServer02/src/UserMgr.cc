#include "UserMgr.h"
#include "CSession.h"

UserMgr::~UserMgr()
{
    // 单例类，可以不加锁
    uidToSession.clear();
}

std::shared_ptr<CSession> UserMgr::getSession(int uid)
{
    std::lock_guard<std::mutex> lock(sessionMutex);
    auto it = uidToSession.find(uid);
    if (it != uidToSession.end())
    {
        return nullptr;
    }

    return it->second;
}

void UserMgr::setUserSession(int uid, std::shared_ptr<CSession> session)
{
    std::lock_guard<std::mutex> lock(sessionMutex);
    uidToSession[uid] = session;
}

void UserMgr::removeUserSession(int uid)
{
    auto uidStr = std::to_string(uid);
    // 因为再次登录可能是其他服务器，所以会造成本服务器删除key，其他服务器注册key的情况
    //  有可能其他服务登录，本服删除key造成找不到key的情况
    // RedisMgr::GetInstance()->Del(USERIPPREFIX + uid_str);
    {
        std::lock_guard<std::mutex> lock(sessionMutex);
        uidToSession.erase(uid);
    }
}