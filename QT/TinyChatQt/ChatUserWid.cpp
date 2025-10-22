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

void ChatUserWid::setInfo(const QString &name, const QString &head, const QString &msg)
{
    name_ = name;
    head_ = head;
    msg_ = msg;
    // 加载图片
    QPixmap pixmap(head_);

    // 设置图片自动缩放
    ui->iconLB->setPixmap(pixmap.scaled(ui->iconLB->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->iconLB->setScaledContents(true);

    ui->userNameLB->setText(name_);
    ui->userChatLB->setText(msg_);
}
