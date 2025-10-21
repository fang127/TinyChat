#ifndef USERMGR_H
#define USERMGR_H

#include <singleton.h>

#include <QObject>
#include <QString>
#include <memory>
class UserMgr : public QObject, public Singleton<UserMgr>
{
    Q_OBJECT

    friend class Singleton<UserMgr>;

public:
    ~UserMgr() = default;

    void setName(const QString &name);

    void setUid(int uid);

    void setToken(const QString &token);

private:
    UserMgr() = default;

    QString name_;  // 用户名
    QString token_; // token
    int uid_;       // uid
};

#endif // USERMGR_H
