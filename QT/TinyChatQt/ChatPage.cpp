#include "ChatPage.h"
#include "ChatItemBase.h"
#include "PictureBubble.h"
#include "TextBubble.h"
#include "ui_ChatPage.h"
#include "UserMgr.h"
#include "TcpMgr.h"
#include <QEvent>
#include <QPainter>
#include <QStyleOption>
#include <QJsonDocument>

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

void ChatPage::setUserInfo(std::shared_ptr<UserInfo> userInfo)
{
    userInfo_ = userInfo;
    ui->titleLabel->setText(userInfo->_name);
    ui->chatDatalist->removeAllItem();
    for(auto &msg : userInfo->_chat_msgs)
    {
        appendChatMsg(msg);
    }
}

void ChatPage::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatPage::on_sendBtn_clicked()
{
    if(userInfo_ == nullptr)
    {
        qDebug() << "friendInfo is empty";
        return;
    }

    auto userInfo = UserMgr::getInstance_()->getUserInfo();

    auto pTextEdit = ui->chatEdit;
    ChatRole role = ChatRole::Self;
    QString userName = userInfo->_name;
    QString userIcon = userInfo->_icon;

    const QVector<MsgInfo> &msgList = pTextEdit->getMsgList();
    QJsonObject textObj;
    QJsonArray textArray;
    int textSize = 0;

    for(int i = 0; i < msgList.size(); ++i)
    {
        //消息内容长度不合规就跳过
        if(msgList[i].content.length() > 1024)
        {
            continue;
        }

        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;

        if(type == "text")
        {
            //生成唯一id
            QUuid uuid = QUuid::createUuid();
            //转为字符串
            QString uuidString = uuid.toString();

            pBubble = new TextBubble(role, msgList[i].content);
            if(textSize + msgList[i].content.length() > 1024)
            {
                textObj["fromUid"] = userInfo->_uid;
                textObj["toUid"] = userInfo_->_uid;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textObj);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                //发送并清空之前累计的文本列表
                textSize = 0;
                textArray = QJsonArray();
                textObj = QJsonObject();
                //发送tcp请求给chat server
                emit TcpMgr::getInstance_()->sigSendData(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
            }

            //将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
            //_bubble_map[uuidString] = pBubble;
            textSize += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8Message);
            obj["msgID"] = uuidString;
            textArray.append(obj);
            auto txt_msg = std::make_shared<TextChatData>(uuidString, obj["content"].toString(),
                userInfo->_uid, userInfo_->_uid);
            emit sigAppendSendChatMsg(txt_msg);
        }
        else if(type == "image")
        {
             pBubble = new PictureBubble(QPixmap(msgList[i].content) , role);
        }
        else if(type == "file")
        {

        }
        //发送消息
        if(pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->chatDatalist->appendChatItem(pChatItem);
        }

    }

    qDebug() << "textArray is " << textArray ;
    //发送给服务器
    textObj["text_array"] = textArray;
    textObj["fromUid"] = userInfo->_uid;
    textObj["toUid"] = userInfo_->_uid;
    QJsonDocument doc(textObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    //发送并清空之前累计的文本列表
    textSize = 0;
    textArray = QJsonArray();
    textObj = QJsonObject();
    //发送tcp请求给chat server
    emit TcpMgr::getInstance_()->sigSendData(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
}

void ChatPage::appendChatMsg(std::shared_ptr<TextChatData> msg)
{
    auto selfInfo = UserMgr::getInstance_()->getUserInfo();
    ChatRole role;
    // 添加聊天显示
    if (msg->_from_uid == selfInfo->_uid)
    {
        role = ChatRole::Self;
        ChatItemBase* pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(selfInfo->_name);
        pChatItem->setUserIcon(QPixmap(selfInfo->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chatDatalist->appendChatItem(pChatItem);
    }
    else
    {
        role = ChatRole::Other;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::getInstance_()->getFriendById(msg->_from_uid);
        if (friend_info == nullptr)
        {
            return;
        }
        pChatItem->setUserName(friend_info->_name);
        pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->chatDatalist->appendChatItem(pChatItem);
    }
}
