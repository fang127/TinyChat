#include "ChatUserWid.h"
#include "ui_ChatUserWid.h"
#include "global.h"

ChatUserWid::ChatUserWid(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    setItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

QSize ChatUserWid::sizeHint() const
{
    return QSize(250,70);
}

void ChatUserWid::setInfo(std::shared_ptr<UserInfo> userInfo)
{
    userInfo_ = userInfo;
    // 加载图片
    QPixmap pixmap(userInfo_->_icon);

    // 设置图片自动缩放
    ui->iconLB->setPixmap(pixmap.scaled(ui->iconLB->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->iconLB->setScaledContents(true);

    ui->userNameLB->setText(userInfo_->_name);
    ui->userChatLB->setText(userInfo_->_last_msg);
}

std::shared_ptr<UserInfo> ChatUserWid::getUserInfo()
{
    return userInfo_;
}
