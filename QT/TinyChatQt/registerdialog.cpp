#include "registerdialog.h"
#include "global.h"
#include "httpmgr.h"
#include "ui_registerdialog.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    // 设置密码输入隐藏的格式
    ui->passEdit->setEchoMode(QLineEdit::Password);
    ui->confirmEdit->setEchoMode(QLineEdit::Password);
    // 设置errTip初始状态
    ui->errTip->setProperty("state", "normal");
    repolish(ui->errTip);

    connect(HttpMgr::getInstance_().get(), &HttpMgr::sigRegModFinish, this,
            &RegisterDialog::slotRegModFinish);

    // 注册回调
    initHttpHandlers();
}

RegisterDialog::~RegisterDialog() { delete ui; }

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
            ReqId::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);
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
    handlers_.insert(ReqId::ID_GET_VARIFY_CODE,
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

    //注册注册用户回包逻辑
    handlers_.insert(ReqId::ID_REG_USER,
                    [this](QJsonObject jsonObj)
                    {
                    int error = jsonObj["error"].toInt();
                    if(error != ErrorCodes::SUCCESS){
                    showTip(tr("参数错误"),false);
                    return;
                    }
                    auto email = jsonObj["email"].toString();
                    showTip(tr("用户注册成功"), true);
                    qDebug()<< "email is " << email ;
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

void RegisterDialog::on_sureBtn_clicked()
{
    if(ui->userEdit->text() == "")
    {
        showTip(tr("用户名不能为空"), false);
        return;
    }

    if(ui->emailEdit->text() == "")
    {
        showTip(tr("邮箱不能为空"), false);
        return;
    }

    if(ui->passEdit->text() == "")
    {
        showTip(tr("密码不能为空"), false);
        return;
    }

    if(ui->confirmEdit->text() == "")
    {
        showTip(tr("确认密码不能为空"), false);
        return;
    }

    if(ui->confirmEdit->text() != ui->passEdit->text())
    {
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }

    if(ui->verifyEdit->text() == ""){
        showTip(tr("验证码不能为空"), false);
        return;
    }

    // set http body text
    QJsonObject jsonObj;
    jsonObj["user"] = ui->userEdit->text();
    jsonObj["email"] = ui->emailEdit->text();
    jsonObj["passwd"] = ui->passEdit->text();
    jsonObj["confirm"] = ui->confirmEdit->text();
    jsonObj["verifycode"] = ui->verifyEdit->text();
    HttpMgr::getInstance_()->postHttpReq(QUrl(gateUrlPrefix + "/user_register"),jsonObj,ReqId::ID_REG_USER,Modules::REGISTERMOD);
}

