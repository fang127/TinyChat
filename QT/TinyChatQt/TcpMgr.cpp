#include "TcpMgr.h"
#include "UserMgr.h"

#include <QJsonDocument>
#include <QObject>

void TcpMgr::closeConnection() { socket_.close(); }

void TcpMgr::slotTcpConnect(ServerInfo serverInfo)
{
    qDebug() << "receive tcp connect signal";
    qDebug() << "Connecting to server...";
    host_ = serverInfo.host_;
    port_ = static_cast<uint16_t>(serverInfo.port_.toInt());
    qDebug() << "port: " << port_ << "host: " << host_;
    socket_.connectToHost(host_, port_);
}

// 因为客户端发送数据可能在任何线程，为了保证线程安全，
// 我们在要发送数据时发送TcpMgr的sig_send_data信号，然后实现接受这个信号的槽函数
void TcpMgr::slotSendData(ReqId reqId, QByteArray data)
{
    uint16_t id = reqId;

    // 计算长度
    quint16 len = static_cast<quint16>(data.size());
    // 存储要发送的数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入id和长度
    out << id << len;

    // 添加字符串数据
    block.append(data);

    socket_.write(block);
    qDebug() << "tcp message send byte data is: " << block;
}

TcpMgr::TcpMgr()
    : host_(""), port_(0), bRecvPending_(false), messageId_(0), messageLen_(0)
{
    // 连接成功信号
    QObject::connect(&socket_, &QTcpSocket::connected,
                     [&]()
                     {
                         qDebug() << "Connected to server!";
                         // 连接成功后发送消息
                         emit sigConSuccess(true);
                     });

    // 准备读取消息的信号
    QObject::connect(
        &socket_, &QTcpSocket::readyRead,
        [&]()
        {
            // 读取所有消息追加到buffer_
            buffer_.append(socket_.readAll());

            QDataStream stream(&buffer_, QIODevice::ReadOnly);
            stream.setVersion(QDataStream::Qt_5_0);

            forever
            {
                // 解析头部
                if (!bRecvPending_)
                {
                    // 当前缓冲区长度不满足头部时，不读取，等有完整的消息再读，解决粘包
                    if (buffer_.size() < static_cast<int>(sizeof(quint16) * 2))
                        return;
                }

                stream >> messageId_ >> messageLen_;

                // 将buffer_中的前4字节移除
                // 从缓冲区移除已处理的数据
                buffer_ = buffer_.mid(sizeof(quint16) * 2);

                qDebug() << "Message ID: " << messageId_
                         << ", Length: " << messageLen_;

                // 判断buffer_剩余长度是否大于messageLen_，大于说明是个完整的数据，小于则等待
                if (buffer_.size() < messageLen_)
                {
                    // 粘包发生
                    bRecvPending_ = true;
                    return;
                }

                bRecvPending_ = false;

                // 读取消息
                QByteArray messageBody = buffer_.mid(0, messageLen_);
                qDebug() << "receive body message is: " << messageBody;

                buffer_ = buffer_.mid(messageLen_);
                handleMsg(ReqId(messageId_), messageLen_, messageBody);
            }
        });

    // 处理错误
    QObject::connect(
        &socket_,
        QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
        [&](QAbstractSocket::SocketError socketError)
        {
            Q_UNUSED(socketError)
            qDebug() << "Error:" << socket_.errorString();
        });

    // 处理连接断开
    QObject::connect(&socket_, &QTcpSocket::disconnected,
                     [&]()
                     {
                         qDebug() << "Disconnected from server.";
                         // 发生断开信号给界面
                         emit sigConnectionClosed();
                     });
    // 发生数据的信号
    QObject::connect(this, &TcpMgr::sigSendData, this, &TcpMgr::slotSendData);

    // 注册回包函数
    initHandlers();
}

// 注册回调
void TcpMgr::initHandlers()
{
    // 收到登录回包的回调
    handlers_.insert(
        ID_CHAT_LOGIN_RSP,
        [this](const ReqId &id, const int &len, const QByteArray &data)
        {
            Q_UNUSED(len);
            qDebug() << "handle id is " << id << " data is " << data;
            // 将数据转为QJson
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

            // 检查是否转换成功
            if (jsonDoc.isNull())
            {
                std::cout << "Failed to create QJsonDocument.";
                return;
            }

            QJsonObject jsonObj = jsonDoc.object();
            qDebug() << "data jsonObj is " << jsonObj;

            if (!jsonObj.contains("error"))
            {
                int err = ErrorCodes::ERR_JSON;
                qDebug() << "Login Failed, err is Json Parse Err: " << err;
                emit sigLoginFailed(err);
                return;
            }

            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS)
            {
                qDebug() << "Login Failed, err is " << err;
                emit sigLoginFailed(err);
                return;
            }

            auto uid = jsonObj["uid"].toInt();
            auto name = jsonObj["name"].toString();
            auto nick = jsonObj["nick"].toString();
            auto icon = jsonObj["icon"].toString();
            auto sex = jsonObj["sex"].toInt();

            auto userInfo = std::make_shared<UserInfo>(uid, name, nick, icon, sex);
            UserMgr::getInstance_()->setUserInfo(userInfo);
            UserMgr::getInstance_()->setToken(jsonObj["token"].toString());

            // 添加申请列表
            if(jsonObj.contains("apply list"))
            {
                UserMgr::getInstance_()->appendApplyList(jsonObj["apply list"].toArray());
            }

            // 添加好友列表
            if(jsonObj.contains("friend list"))
            {
                UserMgr::getInstance_()->appendFriendList(jsonObj["friend list"].toArray());
            }

            // 发生信号，切换到用户界面
            emit sigSwitchChatDiaLog();
        });
    // 查询好友的回包
    handlers_.insert(
        ID_SEARCH_USER_RSP,
        [this](const ReqId &id, const int &len, const QByteArray &data)
        {
            Q_UNUSED(len);
            qDebug() << "handle id is " << id << " data is " << data;
            // 将数据转为QJson
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

            // 检查是否转换成功
            if (jsonDoc.isNull())
            {
                std::cout << "Failed to create QJsonDocument.";
                return;
            }

            QJsonObject jsonObj = jsonDoc.object();
            qDebug() << "data jsonObj is " << jsonObj;

            if (!jsonObj.contains("error"))
            {
                int err = ErrorCodes::ERR_JSON;
                qDebug() << "Search User RSP Failed, err is Json Parse Err: " << err;
                emit sigUserSearch(nullptr);
                return;
            }

            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS)
            {
                qDebug() << "Search User RSP Failed, err is " << err;
                emit sigUserSearch(nullptr);
                return;
            }

            auto searchInfo = std::make_shared<SearchInfo>(
                            jsonObj["uid"].toInt(),
                            jsonObj["name"].toString(),
                            jsonObj["nick"].toString(),
                            jsonObj["desc"].toString(),
                            jsonObj["sex"].toInt(),
                            jsonObj["icon"].toString());

            // 发生信号，通知搜索结果
            emit sigUserSearch(searchInfo);
        });

    // 添加好友回包
    handlers_.insert(
        ID_ADD_FRIEND_RSP,
        [this](const ReqId &id, const int &len, const QByteArray &data)
        {
            Q_UNUSED(len);
            qDebug() << "handle id is " << id << " data is " << data;
            // 将数据转为QJson
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

            // 检查是否转换成功
            if (jsonDoc.isNull())
            {
                std::cout << "Failed to create QJsonDocument.";
                return;
            }

            QJsonObject jsonObj = jsonDoc.object();
            qDebug() << "data jsonObj is " << jsonObj;

            if (!jsonObj.contains("error"))
            {
                int err = ErrorCodes::ERR_JSON;
                qDebug() << "Add Friend REQ Failed, err is Json Parse Err: " << err;
                return;
            }

            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS)
            {
                qDebug() << "Add Friend REQ Failed, err is " << err;
                return;
            }

            qDebug() << "Add Friend REQ success";
        });

    handlers_.insert(
        ID_NOTIFY_ADD_FRIEND_REQ,
        [this](const ReqId &id, const int &len, const QByteArray &data)
        {
            Q_UNUSED(len);
            qDebug() << "handle id is " << id << " data is " << data;
            // 将数据转为QJson
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

            // 检查是否转换成功
            if (jsonDoc.isNull())
            {
                std::cout << "Failed to create QJsonDocument.";
                return;
            }

            QJsonObject jsonObj = jsonDoc.object();
            qDebug() << "data jsonObj is " << jsonObj;

            if (!jsonObj.contains("error"))
            {
                int err = ErrorCodes::ERR_JSON;
                qDebug() << "Notify Friend REQ Failed, err is Json Parse Err: " << err;
                return;
            }

            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS)
            {
                qDebug() << "Notify Friend REQ Failed, err is " << err;
                return;
            }

            int fromUid = jsonObj["applyUid"].toInt();
            QString name = jsonObj["name"].toString();
            QString desc = jsonObj["desc"].toString();
            QString icon = jsonObj["icon"].toString();
            QString nick = jsonObj["nick"].toString();
            int sex = jsonObj["sex"].toInt();

            auto applyInfo = std::make_shared<AddFriendApply>(fromUid, name, desc, icon, nick, sex);

            // auto applyInfo = std::make_shared<AddFriendApply>(fromUid, name, "desc", "icon", "nick", 1);
            emit sigFriendApply(applyInfo);

            qDebug() << "Notify Friend REQ success";
        });

    handlers_.insert(
        ID_NOTIFY_AUTH_FRIEND_REQ,
        [this](const ReqId &id, const int &len, const QByteArray &data)
        {
            Q_UNUSED(len);
            qDebug() << "handle id is " << id << " data is " << data;
            // 将数据转为QJson
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

            // 检查是否转换成功
            if (jsonDoc.isNull())
            {
                std::cout << "Failed to create QJsonDocument.";
                return;
            }

            QJsonObject jsonObj = jsonDoc.object();
            qDebug() << "data jsonObj is " << jsonObj;

            if (!jsonObj.contains("error"))
            {
                int err = ErrorCodes::ERR_JSON;
                qDebug() << "ID_NOTIFY_AUTH_FRIEND_REQ Failed, err is Json Parse Err: " << err;
                return;
            }

            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS)
            {
                qDebug() << "ID_NOTIFY_AUTH_FRIEND_REQ Failed, err is " << err;
                return;
            }

            int fromUid = jsonObj["fromUid"].toInt();
            QString name = jsonObj["name"].toString();
            QString icon = jsonObj["icon"].toString();
            QString nick = jsonObj["nick"].toString();
            int sex = jsonObj["sex"].toInt();

            auto authInfo = std::make_shared<AuthInfo>(fromUid, name, nick, icon, sex);

            // auto applyInfo = std::make_shared<AddFriendApply>(fromUid, name, "desc", "icon", "nick", 1);
            emit sigAddAuthFriend(authInfo);

            qDebug() << "ID_NOTIFY_AUTH_FRIEND_REQ success";
        });

    handlers_.insert(
        ID_AUTH_FRIEND_RSP,
        [this](const ReqId &id, const int &len, const QByteArray &data)
        {
            Q_UNUSED(len);
            qDebug() << "handle id is " << id << " data is " << data;
            // 将数据转为QJson
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

            // 检查是否转换成功
            if (jsonDoc.isNull())
            {
                std::cout << "Failed to create QJsonDocument.";
                return;
            }

            QJsonObject jsonObj = jsonDoc.object();
            qDebug() << "data jsonObj is " << jsonObj;

            if (!jsonObj.contains("error"))
            {
                int err = ErrorCodes::ERR_JSON;
                qDebug() << "ID_AUTH_FRIEND_RSP Failed, err is Json Parse Err: " << err;
                return;
            }

            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS)
            {
                qDebug() << "ID_AUTH_FRIEND_RSP Failed, err is " << err;
                return;
            }

            QString name = jsonObj["name"].toString();
            QString icon = jsonObj["icon"].toString();
            QString nick = jsonObj["nick"].toString();
            int sex = jsonObj["sex"].toInt();
            auto uid = jsonObj["uid"].toInt();
            auto rsp = std::make_shared<AuthRsp>(uid, name, nick, icon, sex);

            // auto applyInfo = std::make_shared<AddFriendApply>(fromUid, name, "desc", "icon", "nick", 1);
            emit sigAuthRsp(rsp);

            qDebug() << "ID_AUTH_FRIEND_RSP success";
        });

    handlers_.insert(ID_TEXT_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
            Q_UNUSED(len);
            qDebug() << "handle id is " << id << " data is " << data;
            // 将QByteArray转换为QJsonDocument
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

            // 检查转换是否成功
            if (jsonDoc.isNull()) {
                qDebug() << "Failed to create QJsonDocument.";
                return;
            }

            QJsonObject jsonObj = jsonDoc.object();

            if (!jsonObj.contains("error")) {
                int err = ErrorCodes::ERR_JSON;
                qDebug() << "Chat Msg Rsp Failed, err is Json Parse Err" << err;
                return;
            }

            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS) {
                qDebug() << "Chat Msg Rsp Failed, err is " << err;
                return;
            }

            qDebug() << "Receive Text Chat Rsp Success " ;
            //ui设置送达等标记 todo...
          });

        handlers_.insert(ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqId id, int len, QByteArray data) {
            Q_UNUSED(len);
            qDebug() << "handle id is " << id << " data is " << data;
            // 将QByteArray转换为QJsonDocument
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

            // 检查转换是否成功
            if (jsonDoc.isNull()) {
                qDebug() << "Failed to create QJsonDocument.";
                return;
            }

            QJsonObject jsonObj = jsonDoc.object();

            if (!jsonObj.contains("error")) {
                int err = ErrorCodes::ERR_JSON;
                qDebug() << "Notify Chat Msg Failed, err is Json Parse Err" << err;
                return;
            }

            int err = jsonObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS) {
                qDebug() << "Notify Chat Msg Failed, err is " << err;
                return;
            }

            qDebug() << "Receive Text Chat Notify Success " ;
            auto msgPtr = std::make_shared<TextChatMsg>(jsonObj["fromUid"].toInt(),
                    jsonObj["toUid"].toInt(),jsonObj["text_array"].toArray());
            emit sigTextChatMsg(msgPtr);
          });
}

// 处理消息
void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto findIter = handlers_.find(id);
    if (findIter == handlers_.end())
    {
        qDebug() << "not found id [" << id << "] to handle";
        return;
    }
    findIter.value()(id, len, data);
}
