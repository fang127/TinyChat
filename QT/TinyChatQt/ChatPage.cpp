#include "ChatPage.h"
#include "ChatItemBase.h"
#include "PictureBubble.h"
#include "TextBubble.h"
#include "ui_ChatPage.h"

#include <QEvent>
#include <QPainter>
#include <QStyleOption>

ChatPage::ChatPage(QWidget *parent) : QWidget(parent), ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    // 设置按钮样式
    ui->receiveBtn->setState("normal", "hover", "press");
    ui->sendBtn->setState("normal", "hover", "press");
    // 设置图标样式
    ui->emoLB->setState("normal", "hover", "press", "normal", "hover", "press");
    ui->fileLB->setState("normal", "hover", "press", "normal", "hover",
                         "press");
}

ChatPage::~ChatPage() { delete ui; }

void ChatPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_sendBtn_clicked()
{
    auto pTextEdit = ui->chatEdit;
    ChatRole role = ChatRole::Self;
    QString userName = QStringLiteral("me");
    QString userIcon = ":/res/head_1.jpg";

    const QVector<MsgInfo> &msgList = pTextEdit->getMsgList();
    for (int i = 0; i < msgList.size(); ++i)
    {
        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;
        if (type == "text")
        {
            pBubble = new TextBubble(role, msgList[i].content);
        }
        else if (type == "image")
        {
            pBubble = new PictureBubble(QPixmap(msgList[i].content), role);
        }
        else if (type == "file")
        {
        }
        if (pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->chatDatalist->appendChatItem(pChatItem);
        }
    }
}
