#include "friendinfopage.h"
#include "ui_friendinfopage.h"
#include <QDebug>

FriendInfoPage::FriendInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendInfoPage),userInfo_(nullptr)
{
    ui->setupUi(this);
    ui->msg_chat->setState("normal","hover","press");
    ui->video_chat->setState("normal","hover","press");
    ui->voice_chat->setState("normal","hover","press");
}

FriendInfoPage::~FriendInfoPage()
{
    delete ui;
}

void FriendInfoPage::setInfo(std::shared_ptr<UserInfo> userInfo)
{
    userInfo_ = userInfo;
    // 加载图片
    QPixmap pixmap(userInfo->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->name_lb->setText(userInfo->_name);
    ui->nick_lb->setText(userInfo->_nick);
    ui->bak_lb->setText(userInfo->_nick);
}

void FriendInfoPage::on_msg_chat_clicked()
{
    qDebug() << "msg chat btn clicked";
    emit sigJumpChatItem(userInfo_);
}
