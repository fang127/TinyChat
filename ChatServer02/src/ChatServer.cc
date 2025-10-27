#include "ChatServer.h"
#include "ConfigMgr.h"
#include "UserMgr.h"

#include <chrono>
#include <iostream>

ChatServer::ChatServer(boost::asio::io_context &ioc, const short &port)
    : ioc_(ioc), port_(port),
      accept_(ioc,
              boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      timer_(ioc_, std::chrono::seconds(60))
{
    std::cout << "ChatServer start success, listen on port: " << port_
              << std::endl;

    startAccept();
}

ChatServer::~ChatServer()
{
    std::cout << "ChatServer destruct listen on port: " << port_ << std::endl;
}

// 开始监听
void ChatServer::startAccept()
{
    auto &ioc = AsioIOServicePool::getInstance()->getIOService();
    // 构建一个Session
    std::shared_ptr<CSession> newSession =
        std::make_shared<CSession>(ioc, this);
    // 将Session与接收到的socket绑定
    accept_.async_accept(newSession->getSocket(),
                         std::bind(&ChatServer::handleAccept, this, newSession,
                                   std::placeholders::_1));
}

// 处理监听到的连接
void ChatServer::handleAccept(std::shared_ptr<CSession> newSession,
                              const boost::system::error_code &error)
{
    if (!error)
    {
        newSession->start();
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_.insert({newSession->getSessionId(), newSession});
    }
    else
    {
        std::cout << "Session accept failed, error val: " << error.value()
                  << " message: " << error.message() << std::endl;
    }

    // 接着监听
    startAccept();
}

// 清除Session
void ChatServer::clearSession(const std::string &sessionId)
{
    if (sessions_.find(sessionId) != sessions_.end())
    {
        auto uid = sessions_[sessionId]->getUserId();
        // 移除用户和session关联
        UserMgr::getInstance()->removeUserSession(uid);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_.erase(sessionId);
    }
}