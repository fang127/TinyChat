#include "FindSuccessDialog.h"
#include "ui_FindSuccessDialog.h"

#include <QDir>

FindSuccessDialog::FindSuccessDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::FindSuccessDialog)
{
    ui->setupUi(this);
    // 设置对话框标题
    setWindowTitle("添加");
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    QString pix_path =
        QDir::toNativeSeparators(app_path + QDir::separator() + "static" +
                                 QDir::separator() + "head_1.jpg");
    QPixmap head_pix(pix_path);
    head_pix = head_pix.scaled(ui->headLB->size(), Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    ui->headLB->setPixmap(head_pix);
    ui->addFriendBtn->setState("normal", "hover", "press");
    this->setModal(true);
}

FindSuccessDialog::~FindSuccessDialog()
{
    qDebug() << "FindSuccessDialog destruct";
    delete ui;
}

void FindSuccessDialog::setSearchInfo(std::shared_ptr<SearchInfo> si)
{
    ui->nameLB->setText(si->name_);
    si_ = si;
}

void FindSuccessDialog::on_addFriendBtn_clicked()
{
    // add code... 添加好友界面弹出
}
