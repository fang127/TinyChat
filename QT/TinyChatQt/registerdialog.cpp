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
            QUrl(gateUrlPrefix + "/get_varifycode"), jsonObj,
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
