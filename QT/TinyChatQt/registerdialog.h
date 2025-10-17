#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "global.h"
#include <QDialog>

namespace Ui
{
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_getCode_clicked();
    void slotRegModFinish(ReqId id, QString res, ErrorCodes err);
    void on_sureBtn_clicked();
    // 返回按钮的槽函数中停止定时器并发送切换登录的信号
    void on_returnBtn_clicked();
    // 取消注册也发送切换登录信号
    void on_cancelBtn_clicked();

private:
    void initHttpHandlers();
    void showTip(QString str, bool bOk);
    // 验证输入是否正确
    bool checkUserValid();
    void addTipErr(TipErr te,QString tips);
    void delTipErr(TipErr te);
    bool checkPassValid();
    bool checkEmailValid();
    bool checkVerifyValid();
    bool checkConfirmValid();
    // 页面切换逻辑
    void changeTipPage();


    Ui::RegisterDialog *ui;

    QMap<ReqId, std::function<void(const QJsonObject &)>> handlers_;
    QMap<TipErr, QString> tipErrs; // 用户输入错误缓存

    QTimer *countdownTimer_;
    int countdown_;

signals:
    void sigSwitchLogin();
};

#endif // REGISTERDIALOG_H
