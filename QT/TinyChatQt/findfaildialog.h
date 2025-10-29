#ifndef FINDFAILDIALOG_H
#define FINDFAILDIALOG_H

#include <QDialog>

namespace Ui {
class FindFailDialog;
}

class FindFailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindFailDialog(QWidget *parent = nullptr);
    ~FindFailDialog();

private:
    Ui::FindFailDialog *ui;

private slots:
    void on_failSureBtn_clicked();
};

#endif // FINDFAILDIALOG_H
