#include "UserMgr.h"

void UserMgr::setName(const QString &name) { name_ = name; }

void UserMgr::setUid(int uid) { uid_ = uid; }

void UserMgr::setToken(const QString &token) { token_ = token; }

int UserMgr::getuid()
{
    return uid_;
}

QString UserMgr::getName()
{
    return name_;
}

std::vector<std::shared_ptr<ApplyInfo> > UserMgr::getApplyList()
{
    return applyList_;
}
