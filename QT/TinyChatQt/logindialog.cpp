#include "logindialog.h"
#include "ui_logindialog.h"

#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->regBtn, &QPushButton::clicked, this,
            &LoginDialog::sigSwitchRegister);
    ui->forgetLabel->setState("normal","hover","","selected","selected_hover","");
    connect(ui->forgetLabel, &ClickedLabel::clicked, this, &LoginDialog::slotForgetPwd);
}

LoginDialog::~LoginDialog()
{
    qDebug() << "destruct LoginDiglog";
    delete ui;
}

void LoginDialog::slotForgetPwd()
{
    qDebug() << "slow forget pwd";
    emit sigSwitchReset();
}
