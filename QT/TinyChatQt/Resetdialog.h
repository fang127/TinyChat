#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include "global.h"

#include <QDialog>

namespace Ui
{
class Resetdialog;
}

class Resetdialog : public QDialog
{
    Q_OBJECT

public:
    explicit Resetdialog(QWidget *parent = nullptr);
    ~Resetdialog();

private:
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkVerifyValid();
    void initHttpHandlers();
    void showTip(QString str, bool status);
    void addTipErr(TipErr te, QString tips);
    void delTipErr(TipErr te);

    Ui::Resetdialog *ui;
    QMap<TipErr, QString> tipErrs_;
    QMap<ReqId, std::function<void(const QJsonObject &)>> handlers_;
    QTimer *countdownTimer_;
    int countdown_;

signals:
    void sigSwitchLogin();

private slots:
    void on_resetBack_clicked();
    void on_resetConfirm_clicked();
    void on_resetCode_clicked();
    void slotResetModFinish(ReqId id, QString res, ErrorCodes err);
};

#endif // RESETDIALOG_H
