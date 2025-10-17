#ifndef GLOBAL_H
#define GLOBAL_H

#include "QStyle"
#include <QByteArray>
#include <QDir>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSettings>
#include <QWidget>
#include <QtNetwork/QNetworkReply>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

/**
 * @brief repolish 用来刷新qss
 */

extern std::function<void(QWidget *)> repolish;

extern std::function<QString(QString)> xorString;

enum ReqId
{
    ID_GET_VERIFY_CODE = 1001, // 获取验证码
    ID_REG_USER = 1002,        // 注册用户
    ID_RESET_PWD = 1003,       // 重置密码
};

enum Modules
{
    REGISTERMOD = 0,
    RESETPASSMOD = 1,
};

enum ErrorCodes
{
    SUCCESS = 0,
    ERR_JSON = 1,    // json解析失败
    ERR_NETWORK = 2, // 网络错误
};

enum TipErr
{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VERIFY_ERR = 5,
    TIP_USER_ERR = 6
};

enum ClickLbState
{
    Normal = 0,
    Selected = 1
};

extern QString gateUrlPrefix;

#endif // GLOBAL_H
