#pragma once

#include "Singleton.h"

#include <memory>
#include <mutex>
#include <unordered_map>

class CSession;

class UserMgr : public Singleton<UserMgr>
{
    friend class Singleton<UserMgr>;

public:
    ~UserMgr();
    std::shared_ptr<CSession> getSession(int uid);
    void setUserSession(int uid, std::shared_ptr<CSession> session);
    void removeUserSession(int uid);

private:
    UserMgr() = default;
    std::mutex sessionMutex;
    std::unordered_map<int, std::shared_ptr<CSession>>
        uidToSession; // key为用户uid，value为对应的CSession
};