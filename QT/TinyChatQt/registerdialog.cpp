#include "registerdialog.h"
#include "global.h"
#include "httpmgr.h"
#include "ui_registerdialog.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog), countdown_(5)
{
    ui->setupUi(this);
    // 设置密码输入隐藏的格式
    ui->passEdit->setEchoMode(QLineEdit::Password);
    ui->confirmEdit->setEchoMode(QLineEdit::Password);
    // 设置errTip初始状态
    // ui->errTip->setProperty("state", "normal");
    // repolish(ui->errTip);

    connect(HttpMgr::getInstance_().get(), &HttpMgr::sigRegModFinish, this,
            &RegisterDialog::slotRegModFinish);
    //    connect(ui->returnBtn,&QPushButton::clicked,this,&RegisterDialog::on_returnBtn_clicked);
    //    connect(ui->cancelBtn,
    //    &QPushButton::clicked,this,&RegisterDialog::on_cancelBtn_clicked);

    // 注册回调
    initHttpHandlers();

    // 设置检测用户的输入信息是否正确
    ui->errTip->clear();

    connect(ui->userEdit, &QLineEdit::editingFinished, this,
            [this]() { checkUserValid(); });

    connect(ui->emailEdit, &QLineEdit::editingFinished, this,
            [this]() { checkEmailValid(); });

    connect(ui->passEdit, &QLineEdit::editingFinished, this,
            [this]() { checkPassValid(); });

    connect(ui->confirmEdit, &QLineEdit::editingFinished, this,
            [this]() { checkConfirmValid(); });

    connect(ui->verifyEdit, &QLineEdit::editingFinished, this,
            [this]() { checkVerifyValid(); });

    // load pictrue of eye for passwd and confirmpass
    // 设置浮动显示手形状
    ui->passVisible->setCursor(Qt::PointingHandCursor);
    ui->confirmVisible->setCursor(Qt::PointingHandCursor);

    ui->passVisible->setState("unvisible", "unvisible_hover", "", "visible",
                              "visible_hover", "");

    ui->confirmVisible->setState("unvisible", "unvisible_hover", "", "visible",
                                 "visible_hover", "");
    // 连接点击事件
    connect(ui->passVisible, &ClickedLabel::clicked, this,
            [this]()
            {
                auto state = ui->passVisible->getCurState();
                if (state == ClickLbState::Normal)
                {
                    ui->passEdit->setEchoMode(QLineEdit::Password);
                }
                else
                {
                    ui->passEdit->setEchoMode(QLineEdit::Normal);
                }
                qDebug() << "Label was clicked!";
            });

    connect(ui->confirmVisible, &ClickedLabel::clicked, this,
            [this]()
            {
                auto state = ui->confirmVisible->getCurState();
                if (state == ClickLbState::Normal)
                {
                    ui->confirmEdit->setEchoMode(QLineEdit::Password);
                }
                else
                {
                    ui->confirmEdit->setEchoMode(QLineEdit::Normal);
                }
                qDebug() << "Label was clicked!";
            });

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
                auto str = QString("注册成功，%1 s后返回登录").arg(countdown_);
                ui->tipLabel01->setText(str);
            });
}

RegisterDialog::~RegisterDialog()
{
    qDebug() << "destruct RegisterDiglog";
    delete ui;
}

void RegisterDialog::on_getCode_clicked()
{
    // 获取用户输入的文本
    auto email = ui->emailEdit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch();
    if (match)
    {
        // 如果匹配发送http验证码
        QJsonObject jsonObj;
        jsonObj["email"] = email;
        HttpMgr::getInstance_()->postHttpReq(
            QUrl(gateUrlPrefix + "/get_verifycode"), jsonObj,
            ReqId::ID_GET_VERIFY_CODE, Modules::REGISTERMOD);
    }
    else
    {
        showTip(tr("邮箱地址错误"), false);
    }
}

void RegisterDialog::slotRegModFinish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS)
    {
        showTip(tr("网络请求错误"), false);
        return;
    }

    // 解析json-字符串,res转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull())
    {
        showTip(tr("json解析失败"), false);
        return;
    }
    // json解析错误
    if (!jsonDoc.isObject())
    {
        showTip(("json解析失败"), false);
        return;
    }

    // 成功的话转为json对象
    QJsonObject jsonObj = jsonDoc.object();

    // 回调函数处理转换成功的json
    handlers_[id](jsonObj);

    return;
}

void RegisterDialog::initHttpHandlers()
{
    // 注册获取验证码回包的逻辑
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
                         showTip(tr("验证码已经发送到邮箱，注意查收"), true);
                         qDebug() << "email is " << email;
                     });

    // 注册注册用户回包逻辑
    handlers_.insert(ReqId::ID_REG_USER,
                     [this](const QJsonObject &jsonObj)
                     {
                         int error = jsonObj["error"].toInt();
                         if (error != ErrorCodes::SUCCESS)
                         {
                             showTip(tr("参数错误"), false);
                             return;
                         }
                         auto email = jsonObj["email"].toString();
                         showTip(tr("用户注册成功"), true);
                         qDebug()
                             << "user uid is " << jsonObj["uid"].toString();
                         qDebug() << "email is " << email;
                         // 页面切换
                         changeTipPage();
                     });
}

void RegisterDialog::showTip(QString str, bool bOk)
{
    if (!bOk)
    {
        ui->errTip->setProperty("state", "err");
    }
    else
    {
        ui->errTip->setProperty("state", "normal");
    }
    ui->errTip->setText(str);

    repolish(ui->errTip);
}

void RegisterDialog::addTipErr(TipErr te, QString tips)
{
    tipErrs[te] = tips;
    showTip(tips, false);
}

void RegisterDialog::delTipErr(TipErr te)
{
    tipErrs.remove(te);
    if (tipErrs.empty())
    {
        ui->errTip->clear();
        return;
    }

    showTip(tipErrs.first(), false);
}

bool RegisterDialog::checkPassValid()
{
    auto pass = ui->passEdit->text();
    auto confirm = ui->confirmEdit->text();

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

    if (pass != confirm)
    {
        // 提示密码不匹配
        addTipErr(TipErr::TIP_PWD_CONFIRM, tr("密码和确认密码不匹配"));
        return false;
    }
    else
    {
        delTipErr(TipErr::TIP_PWD_CONFIRM);
    }
    return true;
}

bool RegisterDialog::checkEmailValid()
{
    // 验证邮箱的地址正则表达式
    auto email = ui->emailEdit->text();
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

bool RegisterDialog::checkVerifyValid()
{
    auto pass = ui->verifyEdit->text();
    if (pass.isEmpty())
    {
        addTipErr(TipErr::TIP_VERIFY_ERR, tr("验证码不能为空"));
        return false;
    }

    delTipErr(TipErr::TIP_VERIFY_ERR);
    return true;
}

bool RegisterDialog::checkUserValid()
{
    if (ui->userEdit->text() == "")
    {
        addTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }

    delTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool RegisterDialog::checkConfirmValid()
{
    auto pass = ui->passEdit->text();
    auto confirm = ui->confirmEdit->text();

    if (confirm.length() < 6 || confirm.length() > 15)
    {
        // 提示长度不准确
        addTipErr(TipErr::TIP_CONFIRM_ERR, tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$
    // 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(confirm).hasMatch();
    if (!match)
    {
        // 提示字符非法
        addTipErr(TipErr::TIP_CONFIRM_ERR, tr("不能包含非法字符"));
        return false;
    }

    delTipErr(TipErr::TIP_CONFIRM_ERR);

    if (pass != confirm)
    {
        // 提示密码不匹配
        addTipErr(TipErr::TIP_PWD_CONFIRM, tr("确认密码和密码不匹配"));
        return false;
    }
    else
    {
        delTipErr(TipErr::TIP_PWD_CONFIRM);
    }
    return true;
}

void RegisterDialog::on_sureBtn_clicked()
{
    if (ui->userEdit->text() == "")
    {
        showTip(tr("用户名不能为空"), false);
        return;
    }

    if (ui->emailEdit->text() == "")
    {
        showTip(tr("邮箱不能为空"), false);
        return;
    }

    if (ui->passEdit->text() == "")
    {
        showTip(tr("密码不能为空"), false);
        return;
    }

    if (ui->confirmEdit->text() == "")
    {
        showTip(tr("确认密码不能为空"), false);
        return;
    }

    if (ui->confirmEdit->text() != ui->passEdit->text())
    {
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }

    if (ui->verifyEdit->text() == "")
    {
        showTip(tr("验证码不能为空"), false);
        return;
    }

    // set http body text
    QJsonObject jsonObj;
    jsonObj["user"] = ui->userEdit->text();
    jsonObj["email"] = ui->emailEdit->text();
    jsonObj["passwd"] = xorString(ui->passEdit->text());
    jsonObj["confirm"] = xorString(ui->confirmEdit->text());
    jsonObj["verifycode"] = ui->verifyEdit->text();
    HttpMgr::getInstance_()->postHttpReq(QUrl(gateUrlPrefix + "/user_register"),
                                         jsonObj, ReqId::ID_REG_USER,
                                         Modules::REGISTERMOD);
}

// 页面切换逻辑
void RegisterDialog::changeTipPage()
{
    countdownTimer_->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    // 启动定时器，设置间隔为1000毫秒（1秒）
    countdownTimer_->start(1000);
}

// 返回按钮的槽函数中停止定时器并发送切换登录的信号
void RegisterDialog::on_returnBtn_clicked()
{
    countdownTimer_->stop();
    qDebug() << "on_returnBtn_clicked";
    emit sigSwitchLogin();
}

// 取消注册也发送切换登录信号
void RegisterDialog::on_cancelBtn_clicked()
{
    countdownTimer_->stop();
    emit sigSwitchLogin();
}
