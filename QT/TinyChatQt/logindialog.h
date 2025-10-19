#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "global.h"

#include <QDialog>

namespace Ui
{
class LoginDialog;
}

struct ServerInfo
{
    int uid_;
    QString host_;
    QString port_;
    QString token_;
};

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    void initHead();
    void initHttpHandlers();
    void showTip(const QString &str, bool bOk);
    bool checkEmailValid();
    bool checkPasswdValid();
    bool enableBtn(bool state);
    void addTipErr(TipErr te, const QString &tips);
    void delTipErr(TipErr te);

    Ui::LoginDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject &)>> heandlers_;
    QMap<TipErr, QString> tipErrs_;
    int uid_;
    QString token_;

signals:
    void sigSwitchRegister();
    void sigSwitchReset();
    void sig_connect_tcp(ServerInfo si);
private slots:
    void slotForgetPwd();
    void on_loginBtn_clicked();
    void slotLoginModFinish(ReqId id, QString res, ErrorCodes err);
};

#endif // LOGINDIALOG_H
