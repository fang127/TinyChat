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

private:
    void initHttpHandlers();
    void showTip(QString str, bool bOk);
    Ui::RegisterDialog *ui;

    QMap<ReqId, std::function<void(const QJsonObject &)>> handlers_;
};

#endif // REGISTERDIALOG_H
