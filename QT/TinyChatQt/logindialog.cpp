#include "logindialog.h"
#include "TcpMgr.h"
#include "httpmgr.h"
#include "ui_logindialog.h"

#include <QDebug>
// #include <QPainter>
// #include <QPainterPath>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    // 注册选项设置
    connect(ui->regBtn, &QPushButton::clicked, this,
            &LoginDialog::sigSwitchRegister);
    // 忘记密码选项设置
    ui->forgetLabel->setState("normal", "hover", "", "selected",
                              "selected_hover", "");
    ui->forgetLabel->setCursor(Qt::PointingHandCursor);
    connect(ui->forgetLabel, &ClickedLabel::clicked, this,
            &LoginDialog::slotForgetPwd);

    // 密码设置
    ui->loginPassEdit->setEchoMode(QLineEdit::Password);
    ui->loginErrTip->setCursor(Qt::PointingHandCursor);
    ui->loginErrTip->clear();
    ui->loginPassVisible->setState("unvisible", "unvisible_hover", "",
                                   "visible", "visible_hover", "");

    // 连接点击事件
    connect(ui->loginPassVisible, &ClickedLabel::clicked, this,
            [this]()
            {
                auto state = ui->loginPassVisible->getCurState();
                if (state == ClickLbState::Normal)
                {
                    ui->loginPassEdit->setEchoMode(QLineEdit::Password);
                }
                else
                {
                    ui->loginPassEdit->setEchoMode(QLineEdit::Normal);
                }
                qDebug() << "Label was clicked!";
            });

    // 注册回包回调函数
    initHttpHandlers();
    // 连接登录回包信号
    connect(HttpMgr::getInstance_().get(), &HttpMgr::sigLoginModFinish, this,
            &LoginDialog::slotLoginModFinish);
    // logo设置
    // initHead();

    // 邮箱和密码格式校验
    connect(ui->loginEmailEdit, &QLineEdit::editingFinished, this,
            [this]() { checkEmailValid(); });
    connect(ui->loginPassEdit, &QLineEdit::editingFinished, this,
            [this]() { checkEmailValid(); });

    // 连接tcp连接请求的信号和槽函数
    connect(this, &LoginDialog::sigConnectTcp, TcpMgr::getInstance_().get(),
            &TcpMgr::slotTcpConnect);
    // 连接tcp管理者发生的连接成功信号
    connect(TcpMgr::getInstance_().get(), &TcpMgr::sigConSuccess, this,
            &LoginDialog::slotTcpConnFinish);
    // 连接tcp管理者发出的登陆失败信号
    connect(TcpMgr::getInstance_().get(), &TcpMgr::sigLoginFailed, this,
            &LoginDialog::slotLoginFailed);
}

LoginDialog::~LoginDialog()
{
    qDebug() << "destruct LoginDiglog";
    delete ui;
}

// void LoginDialog::initHead()
// {
//     // 加载图片
//     QPixmap originalPixmap(":/res/login.png");
//     // 设置图片自动缩放
//     qDebug() << originalPixmap.size() << ui->loginHeadLogo->size();
//     originalPixmap =
//         originalPixmap.scaled(ui->loginHeadLogo->size(), Qt::KeepAspectRatio,
//                               Qt::SmoothTransformation);

//     // 创建一个和原始图片相同大小的QPixmap，用于绘制圆角图片
//     QPixmap roundedPixmap(originalPixmap.size());
//     roundedPixmap.fill(Qt::transparent); // 用透明色填充

//     QPainter painter(&roundedPixmap);
//     painter.setRenderHint(QPainter::Antialiasing); //
//     设置抗锯齿，使圆角更平滑
//     painter.setRenderHint(QPainter::SmoothPixmapTransform);

//     // 使用QPainterPath设置圆角
//     QPainterPath path;
//     path.addRoundedRect(0, 0, originalPixmap.width(),
//     originalPixmap.height(),
//                         10, 10); // 最后两个参数分别是x和y方向的圆角半径
//     painter.setClipPath(path);

//     // 将原始图片绘制到roundedPixmap上
//     painter.drawPixmap(0, 0, originalPixmap);

//     // 设置绘制好的圆角图片到QLabel上
//     ui->loginHeadLogo->setPixmap(roundedPixmap);
// }

void LoginDialog::initHttpHandlers()
{
    heandlers_.insert(ReqId::ID_LOGIN_USER,
                      [this](const QJsonObject &jsonObj)
                      {
                          int error = jsonObj["error"].toInt();
                          if (error != ErrorCodes::SUCCESS)
                          {
                              showTip(tr("参数错误"), false);
                              // 打开按钮
                              enableBtn(true);
                              return;
                          }

                          auto email = jsonObj["email"].toString();

                          // 发送信号通知TcpMgr发送长连接
                          // add code ...
                          // ServerInfo缓存用户信息到本地
                          ServerInfo si;
                          si.uid_ = jsonObj["uid"].toInt();
                          si.host_ = jsonObj["host"].toString();
                          si.port_ = jsonObj["port"].toString();
                          si.token_ = jsonObj["token"].toString();

                          uid_ = si.uid_;
                          token_ = si.token_;
                          qDebug()
                              << "email is " << email << " uid is " << si.uid_
                              << " host is " << si.host_ << " Port is "
                              << si.port_ << " Token is " << si.token_;
                          emit sigConnectTcp(si);
                      });
}

void LoginDialog::showTip(const QString &str, bool bOk)
{
    if (bOk)
    {
        ui->loginErrTip->setProperty("state", "normal");
    }
    else
    {
        ui->loginErrTip->setProperty("state", "err");
    }

    ui->loginErrTip->setText(str);

    repolish(ui->loginErrTip);
}

bool LoginDialog::checkEmailValid()
{
    auto email = ui->loginEmailEdit->text();
    if (email.isEmpty())
    {
        qDebug() << "Email empty";
        addTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱不能为空"));
        return false;
    }

    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if (!match)
    {
        // 提示邮箱不正确
        addTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }

    delTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool LoginDialog::checkPasswdValid()
{
    auto passwd = ui->loginPassEdit->text();
    if (passwd.length() < 6 || passwd.length() > 15)
    {
        qDebug() << "Pass length invalid";
        addTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$
    // 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(passwd).hasMatch();
    if (!match)
    {
        // 提示字符非法
        addTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符且长度为(6~15)"));
        return false;
    }

    delTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

// 登录和注册按钮状态设置
bool LoginDialog::enableBtn(bool state)
{
    ui->loginBtn->setEnabled(state);
    ui->regBtn->setEnabled(state);
    return true;
}

void LoginDialog::addTipErr(TipErr te, const QString &tips)
{
    tipErrs_[te] = tips;
    showTip(tips, false);
}

void LoginDialog::delTipErr(TipErr te)
{
    tipErrs_.remove(te);
    if (tipErrs_.empty())
    {
        ui->loginErrTip->clear();
        return;
    }
    showTip(tipErrs_.first(), false);
}

void LoginDialog::slotForgetPwd()
{
    qDebug() << "slow forget pwd";
    emit sigSwitchReset();
}

// 先法短连接给gateServer验证用户信息，然后再回包处理中发送tcp长连接给tcp服务器，建立登录状态
void LoginDialog::on_loginBtn_clicked()
{
    qDebug() << "login btn clicked";
    if (checkEmailValid() == false)
    {
        return;
    }

    if (checkPasswdValid() == false)
    {
        return;
    }

    enableBtn(false);

    // 发送http请求登录
    QJsonObject jsonObj;
    jsonObj["email"] = ui->loginEmailEdit->text();
    jsonObj["passwd"] = xorString(ui->loginPassEdit->text());
    HttpMgr::getInstance_()->postHttpReq(QUrl(gateUrlPrefix + "/user_login"),
                                         jsonObj, ReqId::ID_LOGIN_USER,
                                         Modules::LOGINMOD);
}

void LoginDialog::slotLoginModFinish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS)
    {
        showTip(tr("网络请求错误"), false);
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isObject())
    {
        showTip(tr("json解析错误"), false);
        return;
    }
    // 调用对应的逻辑,根据id回调。
    heandlers_[id](jsonDoc.object());

    return;
}

void LoginDialog::slotTcpConnFinish(bool bsuccess)
{
    if (bsuccess)
    {
        showTip(tr("聊天服务连接成功，正在登录..."), true);
        QJsonObject jsonObj;
        jsonObj["uid"] = uid_;
        jsonObj["token"] = token_;

        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);

        // 发生tcp请求给chatserver
        emit TcpMgr::getInstance_()
            -> sigSendData(ReqId::ID_CHAT_LOGIN, jsonString);
    }
    else
    {
        showTip(tr("网络异常"), false);
        enableBtn(true);
    }
}

void LoginDialog::slotLoginFailed(int err)
{
    QString result = QString("登录失败, err is %1").arg(err);
    showTip(result, false);
    enableBtn(true);
}
