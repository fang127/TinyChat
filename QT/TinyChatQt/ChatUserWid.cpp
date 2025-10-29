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
    userInfos_ = userInfo;
    // 加载图片
    QPixmap pixmap(userInfos_->_icon);

    // 设置图片自动缩放
    ui->iconLB->setPixmap(pixmap.scaled(ui->iconLB->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->iconLB->setScaledContents(true);

    ui->userNameLB->setText(userInfos_->_name);
    ui->userChatLB->setText(userInfos_->_last_msg);
}
