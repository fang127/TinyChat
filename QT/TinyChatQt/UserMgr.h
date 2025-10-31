#ifndef USERMGR_H
#define USERMGR_H

#include "singleton.h"
#include "UserData.h"
#include <QObject>
#include <QString>
#include <memory>
#include <vector>
class UserMgr : public QObject, public Singleton<UserMgr>
{
    Q_OBJECT

    friend class Singleton<UserMgr>;

public:
    ~UserMgr() = default;

    void setName(const QString &name);

    void setUid(int uid);

    void setToken(const QString &token);

    int getuid();

    QString getName();

    std::vector<std::shared_ptr<ApplyInfo>> getApplyList();
    bool alreadyApply(int apply);
    void addApplyList(std::shared_ptr<ApplyInfo> apply);
    void setUserInfo(std::shared_ptr<UserInfo> userInfo);
    void appendApplyList(QJsonArray array);
    void appendFriendList(QJsonArray array);
    bool checkFriendByUid(int uid);
    void addFriend(std::shared_ptr<AuthRsp> authRsp);
    void addFriend(std::shared_ptr<AuthInfo> authInfo);
    std::shared_ptr<FriendInfo> getFriendByUid(int uid);

    std::vector<std::shared_ptr<FriendInfo>> getChatListPerPage();
    bool isLoadChatFin();
    void updateChatLoadedCount();
    std::vector<std::shared_ptr<FriendInfo>> getConListPerPage();
    void updateContactLoadedCount();
    bool isLoadConFin();
    std::shared_ptr<UserInfo> getUserInfo();
    std::shared_ptr<FriendInfo> getFriendById(int uid);
    void appendFriendChatMsg(int friendID,std::vector<std::shared_ptr<TextChatData> > msgs);
private:
    UserMgr();

    QString token_; // token
    std::vector<std::shared_ptr<ApplyInfo>> applyList_; // 已有的申请列表
    std::shared_ptr<UserInfo> userInfo_; // self info
    QMap<int, std::shared_ptr<FriendInfo>> friendMap_; // was added friend
    std::vector<std::shared_ptr<FriendInfo>> friendList_; // 好友列表
    int chatLoaded_; // 加载数量
    int contactLoaded_; // 联系人数量
};

#endif // USERMGR_H
