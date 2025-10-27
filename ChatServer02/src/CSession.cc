#include "CSession.h"
#include "ChatServer.h"
#include "LogicSystem.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <exception>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

CSession::CSession(boost::asio::io_context &ioc, ChatServer *server)
    : socket_(ioc), server_(server), close_(false)
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    sessionId_ = to_string(uuid);
    recvHeadNode_ = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
    lastRecvTime_ = std::time(nullptr);
}

CSession::~CSession() { std::cout << "CSession destruct" << std::endl; }

boost::asio::ip::tcp::socket &CSession::getSocket() { return socket_; }

int CSession::getUserId() { return userId_; }

void CSession::setUserId(const int &userId) { userId_ = userId; }

std::string CSession::getSessionId() { return sessionId_; }

std::shared_ptr<CSession> CSession::getSharedSelf()
{
    return shared_from_this();
}

void CSession::start() { asyncReadHead(HEAD_TOTAL_LEN); }

// 读取消息头
void CSession::asyncReadHead(int len)
{
    auto self = shared_from_this();
    // 先读取头部
    boost::asio::async_read(
        socket_, boost::asio::buffer(data_, len),
        [self, this, len](const boost::system::error_code &ec,
                          std::size_t bytesWasTransfer)
        {
            try
            {
                if (ec)
                {
                    std::cout << "handle read failed, error is " << ec.message()
                              << std::endl;
                    close();
                    server_->clearSession(sessionId_);
                    return;
                }

                recvHeadNode_->clear();
                memcpy(recvHeadNode_->data_, data_, bytesWasTransfer);

                // 获取头部id和消息体长度
                short msgId = 0;
                memcpy(&msgId, recvHeadNode_->data_, HEAD_ID_LEN);
                // 转为主机字节序
                msgId = boost::asio::detail::socket_ops::network_to_host_short(
                    msgId);
                short msglen = 0;
                memcpy(&msglen, recvHeadNode_->data_ + HEAD_ID_LEN,
                       HEAD_DATA_LEN);
                msglen = boost::asio::detail::socket_ops::network_to_host_short(
                    msglen);
                std::cout << "receive message:{\nid = " << msgId
                          << "\nlength = " << msglen << "\n}" << std::endl;
                // 组成消息
                recvMsgNode_ = std::make_shared<RecvNode>(msglen, msgId);
                // 清空缓冲区，为读取消息体做准备
                memset(data_, 0, MAX_LENGTH);
                asyncReadBody(msglen);
            }
            catch (std::exception &e)
            {
                std::cerr << "Exception is " << e.what();
            }
        });
}

// 读取消息体
void CSession::asyncReadBody(int totalLen)
{
    auto self = shared_from_this();
    boost::asio::async_read(
        socket_, boost::asio::buffer(data_, totalLen),
        [self, this, totalLen](const boost::system::error_code &ec,
                               std::size_t bytesWasTransfer)
        {
            try
            {
                if (ec)
                {
                    std::cout << "handle read failed, error is " << ec.message()
                              << std::endl;
                    close();
                    server_->clearSession(sessionId_);
                    return;
                }

                memcpy(recvMsgNode_->data_, data_, bytesWasTransfer);
                std::cout << "receive data is:{\n"
                          << recvMsgNode_->data_ << "\n}" << std::endl;
                // 将消息投递到队列中，生产者-消费者模式，让LogicSystem取消息并且发生给另一客户
                // add code ...
                LogicSystem::getInstance()->postMsgToQue(
                    std::make_shared<LogicNode>(shared_from_this(),
                                                recvMsgNode_));
                // 清空缓冲区，为下一条消息做准备
                memset(data_, 0, MAX_LENGTH);
                // 继续监听接收事件
                asyncReadHead(HEAD_TOTAL_LEN);
            }
            catch (std::exception &e)
            {
                std::cerr << "Exception is " << e.what();
            }
        });
}

void CSession::send(char *msg, short maxlen, short msgId) {}

void CSession::send(const std::string &msg, short msgId)
{
    std::cout << msg << std::endl;
    std::lock_guard<std::mutex> lock(sendLock_);
    int sendSize = sendQue_.size();
    if (sendSize > MAX_SENDQUE)
    {
        std::cout << "session: " << sessionId_ << " send que fulled, size is "
                  << MAX_SENDQUE << std::endl;
        return;
    }
    bool wasEmpty = sendQue_.empty();
    sendQue_.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgId));
    // 不为空说明在之前队列有消息，此时可能正在处理消息
    if (!wasEmpty)
    {
        return;
    }
    auto &msgNode = sendQue_.front();
    // 打印消息体（跳过头部）
    const char *body = msgNode->data_ + HEAD_TOTAL_LEN;
    const auto bodyLen = msgNode->totallen_ - HEAD_TOTAL_LEN;
    std::cout << __func__ << "\nsend payload(len=" << bodyLen
              << "): " << std::string(body, bodyLen) << std::endl;
    boost::asio::async_write(
        socket_, boost::asio::buffer(msgNode->data_, msgNode->totallen_),
        std::bind(&CSession::handleWrite, this, std::placeholders::_1,
                  getSharedSelf()));
}

void CSession::handleWrite(const boost::system::error_code &error,
                           std::shared_ptr<CSession> sharedSelf)
{
    try
    {
        std::cout << __func__ << std::endl;
        auto self = shared_from_this();
        if (!error)
        {
            std::lock_guard<std::mutex> lock(sendLock_);
            // 弹出已发送的消息
            sendQue_.pop();
            if (!sendQue_.empty())
            {
                auto msgNode = sendQue_.front();
                boost::asio::async_write(
                    socket_,
                    boost::asio::buffer(msgNode->data_, msgNode->totallen_),
                    std::bind(&CSession::handleWrite, this,
                              std::placeholders::_1, getSharedSelf()));
            }
        }
        else
        {
            std::cout << "handle write failed, error is " << error.message()
                      << std::endl;
            close();
            server_->clearSession(sessionId_);
            return;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception code : " << e.what();
    }
}

void CSession::close()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    socket_.close();
    close_ = true;
}

LogicNode::LogicNode(std::shared_ptr<CSession> session,
                     std::shared_ptr<RecvNode> recvNode)
    : session_(session), recvNode_(recvNode)
{
}