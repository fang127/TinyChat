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

void ChatUserWid::setInfo(std::shared_ptr<FriendInfo> friendInfo)
{
    userInfo_ = std::make_shared<UserInfo>(friendInfo);
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

void ChatUserWid::updateLastMsg(std::vector<std::shared_ptr<TextChatData> > msgs)
{
    QString lastMsg = "";
    for (auto& msg : msgs)
    {
        lastMsg = msg->_msg_content;
        userInfo_->_chat_msgs.push_back(msg);
    }

    userInfo_->_last_msg = lastMsg;
    ui->userChatLB->setText(userInfo_->_last_msg);
}
