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

private:
    UserMgr() = default;

    QString name_;  // 用户名
    QString token_; // token
    int uid_;       // uid
    std::vector<std::shared_ptr<ApplyInfo>> applyList_;
};

#endif // USERMGR_H
