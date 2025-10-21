#include "CSession.h"
#include "ChatServer.h"
#include "LogicSystem.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <exception>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

CSession::CSession(boost::asio::io_context &ioc, ChatServer *server)
    : socket_(ioc), server_(server), close_(false), userId_(0)
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    sessionId_ = boost::uuids::to_string(uuid);
    recvHeadNode_ = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
    lastRecvTime_ = std::time(nullptr);
}

CSession::~CSession() { std::cout << "CSession destruct" << std::endl; }

boost::asio::ip::tcp::socket &CSession::getSocket() { return socket_; }

std::string CSession::getUserId() { return userId_; }

void CSession::setUserId(const std::string &userId) { userId_ = userId; }

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
    asyncReadFull(
        HEAD_TOTAL_LEN,
        [self, this](const boost::system::error_code &ec,
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

                //   if (bytesWasTransfer < HEAD_TOTAL_LEN)
                //   {
                //       std::cout << "read length failed, error is "
                //                 << ec.message() << std::endl;
                //   }
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
                std::cout << "receive message:\nid = " << msgId
                          << "\nlength = " << msglen << std::endl;
                // 组成消息
                recvMsgNode_ = std::make_shared<RecvNode>(msglen, msgId);
                asyncReadBody(msglen);
            }
            catch (std::exception &e)
            {
                std::cerr << "Exception is " << e.what();
            }
        });
}

// 读取完整消息
void CSession::asyncReadFull(
    std::size_t maxlen,
    std::function<void(const boost::system::error_code &, std::size_t)> handler)
{
    memset(data_, 0, MAX_LENGTH);
    asyncReadLen(0, maxlen, handler);
}

// 读取指定长度
void CSession::asyncReadLen(
    std::size_t readlen,
    std::size_t totalLen,
    std::function<void(const boost::system::error_code &, std::size_t)> handler)
{
    auto self = shared_from_this();
    socket_.async_receive(
        boost::asio::buffer(data_ + readlen, totalLen - readlen),
        [handler, readlen, totalLen, self](const boost::system::error_code &ec,
                                           std::size_t bytesWasTransfer)
        {
            // 调用回调
            handler(ec, readlen + bytesWasTransfer);
            return;
        });
}

// 读取消息体
void CSession::asyncReadBody(int totalLen)
{
    auto self = shared_from_this();
    asyncReadFull(totalLen,
                  [self, this, totalLen](const boost::system::error_code &ec,
                                         std::size_t bytesWasTransfer)
                  {
                      try
                      {
                          if (ec)
                          {
                              std::cout << "handle read failed, error is "
                                        << ec.message() << std::endl;
                              close();
                              server_->clearSession(sessionId_);
                              return;
                          }

                          memcpy(recvMsgNode_->data_, data_, bytesWasTransfer);
                          std::cout << "receive data is:\n"
                                    << recvMsgNode_->data_ << std::endl;
                          // 将消息投递到队列中，生产者-消费者模式，让LogicSystem取消息并且发生给另一客户
                          // add code ...
                          LogicSystem::getInstance()->postMsgToQue(
                              recvMsgNode_);
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

void CSession::send(const std::string &msg, short msgId) {}

void CSession::close()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    socket_.close();
    close_ = true;
}