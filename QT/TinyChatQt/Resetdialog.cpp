#include "Resetdialog.h"
#include "httpmgr.h"
#include "ui_Resetdialog.h"

#include <QDebug>
#include <QTimer>

Resetdialog::Resetdialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::Resetdialog), countdown_(5)
{
    ui->setupUi(this);

    // 设置密码输入列
    ui->resetPassText->setEchoMode(QLineEdit::Password);
    ui->resetErrTip->setCursor(Qt::PointingHandCursor);
    ui->resetErrTip->clear();
    ui->resetPassVisible->setState("unvisible", "unvisible_hover", "",
                                   "visible", "visible_hover", "");
    // 连接点击事件
    connect(ui->resetPassVisible, &ClickedLabel::clicked, this,
            [this]()
            {
                auto state = ui->resetPassVisible->getCurState();
                if (state == ClickLbState::Normal)
                {
                    ui->resetPassText->setEchoMode(QLineEdit::Password);
                }
                else
                {
                    ui->resetPassText->setEchoMode(QLineEdit::Normal);
                }
                qDebug() << "Label was clicked!";
            });

    // 设置检测用户的输入信息是否正确
    connect(ui->resetUserText, &QLineEdit::editingFinished, this,
            [this]() { checkUserValid(); });

    connect(ui->resetEmailText, &QLineEdit::editingFinished, this,
            [this]() { checkEmailValid(); });

    connect(ui->resetPassText, &QLineEdit::editingFinished, this,
            [this]() { checkPassValid(); });

    connect(ui->resetCodeText, &QLineEdit::editingFinished, this,
            [this]() { checkVerifyValid(); });

    // 连接reset相关信号和注册处理回调
    initHttpHandlers();
    connect(HttpMgr::getInstance_().get(), &HttpMgr::sigResetModFinish, this,
            &Resetdialog::slotResetModFinish);

    // 创建定时器
    countdownTimer_ = new QTimer(this);
    // 连接信号和槽
    connect(countdownTimer_, &QTimer::timeout,
            [this]()
            {
                if (countdown_ == 0)
                {
                    countdownTimer_->stop();
                    emit sigSwitchLogin();
                    return;
                }
                --countdown_;
                auto str = QString("重置成功，%1 s后返回登录").arg(countdown_);
                ui->resetErrTip->setText(str);
            });
}

Resetdialog::~Resetdialog()
{
    qDebug() << "destruct Resetdialog";
    delete ui;
}

bool Resetdialog::checkUserValid()
{
    if (ui->resetUserText->text() == "")
    {
        addTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }

    delTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool Resetdialog::checkEmailValid()
{
    // 验证邮箱正则表达式
    auto email = ui->resetEmailText->text();
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

bool Resetdialog::checkPassValid()
{
    auto pass = ui->resetPassText->text();

    if (pass.length() < 6 || pass.length() > 15)
    {
        // 提示长度不准确
        addTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$
    // 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if (!match)
    {
        // 提示字符非法
        addTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
        return false;
        ;
    }

    delTipErr(TipErr::TIP_PWD_ERR);

    return true;
}

bool Resetdialog::checkVerifyValid()
{
    auto pass = ui->resetCodeText->text();
    if (pass.isEmpty())
    {
        addTipErr(TipErr::TIP_VERIFY_ERR, tr("验证码不能为空"));
        return false;
    }

    delTipErr(TipErr::TIP_VERIFY_ERR);
    return true;
}

void Resetdialog::initHttpHandlers()
{
    // 注册获取验证码回包逻辑
    handlers_.insert(ReqId::ID_GET_VERIFY_CODE,
                     [this](const QJsonObject &jsonObj)
                     {
                         int error = jsonObj["error"].toInt();
                         if (error != ErrorCodes::SUCCESS)
                         {
                             showTip(tr("参数错误"), false);
                             return;
                         }

                         auto email = jsonObj["email"].toString();
                         showTip(tr("验证码已发送到邮箱，请注意查收"), true);
                         qDebug() << "email is " << email;
                     });

    handlers_.insert(ReqId::ID_RESET_PWD,
                     [this](const QJsonObject &jsonObj)
                     {
                         int error = jsonObj["error"].toInt();
                         if (error != ErrorCodes::SUCCESS)
                         {
                             showTip(tr("参数错误"), false);
                             return;
                         }

                         auto email = jsonObj["email"].toString();
                         showTip("重置成功，点击返回登录", true);
                         qDebug() << "email is " << email;
                         qDebug()
                             << "user uid is " << jsonObj["uid"].toString();
                         countdownTimer_->start(1000);
                     });
}

void Resetdialog::showTip(QString str, bool status)
{
    if (status)
    {
        ui->resetErrTip->setProperty("state", "normal");
    }
    else
    {
        ui->resetErrTip->setProperty("state", "err");
    }

    ui->resetErrTip->setText(str);

    repolish(ui->resetErrTip);
}

void Resetdialog::addTipErr(TipErr te, QString tips)
{
    tipErrs_[te] = tips;
    showTip(tips, false);
}

void Resetdialog::delTipErr(TipErr te)
{
    tipErrs_.remove(te);
    if (tipErrs_.empty())
    {
        ui->resetErrTip->clear();
        return;
    }

    showTip(tipErrs_.first(), false);
}

void Resetdialog::on_resetBack_clicked()
{
    countdownTimer_->stop();
    qDebug() << "resetBack btn clicked";
    emit sigSwitchLogin();
}
void Resetdialog::on_resetConfirm_clicked()
{
    if (!checkUserValid())
    {
        return;
    }

    if (!checkEmailValid())
    {
        return;
    }

    if (!checkPassValid())
    {
        return;
    }

    if (!checkVerifyValid())
    {
        return;
    }

    // 发送http重置用户请求
    QJsonObject jsonObj;
    jsonObj["user"] = ui->resetUserText->text();
    jsonObj["email"] = ui->resetEmailText->text();
    jsonObj["passwd"] = ui->resetPassText->text();
    jsonObj["verifyCode"] = ui->resetCodeText->text();
    HttpMgr::getInstance_()->postHttpReq(QUrl(gateUrlPrefix + "/reset_pwd"),
                                         jsonObj, ReqId::ID_RESET_PWD,
                                         Modules::RESETPASSMOD);
}

void Resetdialog::on_resetCode_clicked()
{
    qDebug() << "receive verify btn clicked";
    auto email = ui->resetEmailText->text();
    if (!checkEmailValid())
    {
        return;
    }

    // 发送http请求获取验证码
    QJsonObject jsonObj;
    jsonObj["email"] = email;
    HttpMgr::getInstance_()->postHttpReq(
        QUrl(gateUrlPrefix + "/get_verifycode"), jsonObj,
        ReqId::ID_GET_VERIFY_CODE, Modules::RESETPASSMOD);
}

void Resetdialog::slotResetModFinish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS)
    {
        showTip(tr("网络请求错误"), false);
        return;
    }

    // 解析Json字符串，res需要转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());

    if (jsonDoc.isNull() || !jsonDoc.isObject())
    {
        showTip(tr("json解析错误"), false);
        return;
    }

    // 调用回调函数
    handlers_[id](jsonDoc.object());

    return;
}
