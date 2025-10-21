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
    socket_.connectToHost(host_, port_);
}

// 因为客户端发送数据可能在任何线程，为了保证线程安全，
// 我们在要发送数据时发送TcpMgr的sig_send_data信号，然后实现接受这个信号的槽函数
void TcpMgr::slotSendData(ReqId reqId, QString data)
{
    uint16_t id = reqId;

    // 字符转为字符数组
    QByteArray dataBytes = data.toUtf8();

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
    block.append(dataBytes);

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
                buffer_.remove(0, static_cast<int>(sizeof(quint16) * 2));

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

            UserMgr::getInstance_()->setUid(jsonObj["uid"].toInt());
            UserMgr::getInstance_()->setName(jsonObj["name"].toString());
            UserMgr::getInstance_()->setToken(jsonObj["token"].toString());
            // 发生信号，切换到用户界面
            emit sigSwitchChatDiaLog();
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
