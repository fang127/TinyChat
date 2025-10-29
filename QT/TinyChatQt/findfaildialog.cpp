#include "findfaildialog.h"
#include "ui_findfaildialog.h"

#include <QDebug>

FindFailDialog::FindFailDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindFailDialog)
{
    ui->setupUi(this);
    setWindowTitle("添加");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("FindFailDialog");
    ui->failSureBtn->setState("normal", "hover", "press");
    this->setModal(true);
}

FindFailDialog::~FindFailDialog()
{
    qDebug() << "FindFailDialog destruct";
    delete ui;
}

void FindFailDialog::on_failSureBtn_clicked()
{
    this->hide();
}

