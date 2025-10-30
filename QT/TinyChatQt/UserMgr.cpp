#include "UserMgr.h"

void UserMgr::setName(const QString &name) { userInfo_->_name = name; }

void UserMgr::setUid(int uid) { userInfo_->_uid = uid; }

void UserMgr::setToken(const QString &token) { token_ = token; }

int UserMgr::getuid()
{
    return userInfo_->_uid;
}

QString UserMgr::getName()
{
    return userInfo_->_name;
}

std::vector<std::shared_ptr<ApplyInfo> > UserMgr::getApplyList()
{
    return applyList_;
}

bool UserMgr::alreadyApply(int uid)
{
    for(auto &apply : applyList_)
    {
        if(apply->_uid == uid)
        {
            return true;
        }
    }

    return false;
}

void UserMgr::addApplyList(std::shared_ptr<ApplyInfo> apply)
{
    applyList_.push_back(apply);
}

void UserMgr::setUserInfo(std::shared_ptr<UserInfo> userInfo)
{
    userInfo_ = userInfo;
}

void UserMgr::appendApplyList(QJsonArray array)
{
    qDebug() << "appendApplyList";
    for(const QJsonValue &value : array)
    {
        qDebug() << "appendApplyList......";
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto sex = value["sex"].toInt();
        auto uid = value["uid"].toInt();
        auto status = value["status"].toInt();
        auto info = std::make_shared<ApplyInfo>(uid, name, desc, icon, nick, sex, status);
        applyList_.push_back(info);
    }
}

void UserMgr::appendFriendList(QJsonArray array)
{
    qDebug() << "appendFriendList";
    for(const QJsonValue &value : array)
    {
        qDebug() << "appendFriendList......";
        auto name = value["name"].toString();
        auto desc = value["desc"].toString();
        auto icon = value["icon"].toString();
        auto nick = value["nick"].toString();
        auto sex = value["sex"].toInt();
        auto uid = value["uid"].toInt();
        auto back = value["back"].toString();
        auto info = std::make_shared<FriendInfo>(uid, name, nick,icon, sex,desc, back);
        friendList_.push_back(info);
        friendMap_.insert(uid,info);
    }
}

bool UserMgr::checkFriendByUid(int uid)
{
    auto it = friendMap_.find(uid);
    if(it == friendMap_.end())
    {
        return false;
    }
    return true;
}

void UserMgr::addFriend(std::shared_ptr<AuthRsp> authRsp)
{
    auto friendInfo = std::make_shared<FriendInfo>(authRsp);
    friendMap_[friendInfo->_uid] = friendInfo;
}

void UserMgr::addFriend(std::shared_ptr<AuthInfo> authInfo)
{
    auto friendInfo = std::make_shared<FriendInfo>(authInfo);
    friendMap_[friendInfo->_uid] = friendInfo;
}

std::shared_ptr<FriendInfo> UserMgr::getFriendByUid(int uid)
{
    auto it = friendMap_.find(uid);
    if(it == friendMap_.end())
    {
        return nullptr;
    }
    return *it;
}
// 加载好友
std::vector<std::shared_ptr<FriendInfo> > UserMgr::getChatListPerPage()
{
    std::vector<std::shared_ptr<FriendInfo>> friendList;
    int begin = chatLoaded_;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= friendList_.size())
    {
        return friendList;
    }

    if (end > friendList_.size())
    {
        friendList = std::vector<std::shared_ptr<FriendInfo>>(friendList_.begin() + begin, friendList_.end());
        return friendList;
    }


    friendList = std::vector<std::shared_ptr<FriendInfo>>(friendList_.begin() + begin, friendList_.begin()+ end);
    return friendList;
}
// 判断是否加载完全
bool UserMgr::isLoadChatFin()
{
    if (chatLoaded_ >= friendList_.size())
    {
        return true;
    }

    return false;
}

void UserMgr::updateChatLoadedCount()
{
    int begin = chatLoaded_;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= friendList_.size())
    {
        return ;
    }

    if (end > friendList_.size())
    {
        chatLoaded_ = friendList_.size();
        return ;
    }

    chatLoaded_ = end;
}

std::vector<std::shared_ptr<FriendInfo> > UserMgr::getConListPerPage()
{
    std::vector<std::shared_ptr<FriendInfo>> friendList;
    int begin = contactLoaded_;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= friendList_.size())
    {
        return friendList;
    }

    if (end > friendList_.size())
    {
        friendList = std::vector<std::shared_ptr<FriendInfo>>(friendList_.begin() + begin, friendList_.end());
        return friendList;
    }


    friendList = std::vector<std::shared_ptr<FriendInfo>>(friendList_.begin() + begin, friendList_.begin() + end);
    return friendList;
}

void UserMgr::updateContactLoadedCount()
{
    int begin = contactLoaded_;
    int end = begin + CHAT_COUNT_PER_PAGE;

    if (begin >= friendList_.size())
    {
        return;
    }

    if (end > friendList_.size())
    {
        contactLoaded_ = friendList_.size();
        return;
    }

    contactLoaded_ = end;
}

bool UserMgr::isLoadConFin()
{
    if (contactLoaded_ >= friendList_.size())
    {
        return true;
    }

    return false;
}

UserMgr::UserMgr() : userInfo_(nullptr), chatLoaded_(0), contactLoaded_(0)
{
}
