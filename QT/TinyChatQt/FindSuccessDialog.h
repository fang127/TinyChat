#ifndef FINDSUCCESSDIALOG_H
#define FINDSUCCESSDIALOG_H

#include <QDialog>
#include <memory>
#include "UserData.h"

namespace Ui {
class FindSuccessDialog;
}

class FindSuccessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindSuccessDialog(QWidget *parent = nullptr);
    ~FindSuccessDialog();
    void setSearchInfo(std::shared_ptr<SearchInfo> si);
private:
    Ui::FindSuccessDialog *ui;
    std::shared_ptr<SearchInfo> si_;
    QWidget *parent_;

private slots:
    void on_addFriendBtn_clicked();
};

#endif // FINDSUCCESSDIALOG_H
