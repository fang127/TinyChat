#pragma once

#include "AsioIOServicePool.h"
#include "CSession.h"

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string>

// 监听服务器端口，将连接分配给CSession
class ChatServer : public std::enable_shared_from_this<ChatServer>
{
public:
    ChatServer(boost::asio::io_context &ioc, const short &port);

    ~ChatServer();

    // 开始监听
    void startAccept();

    // 处理监听到的连接
    void handleAccept(std::shared_ptr<CSession>,
                      const boost::system::error_code &ec);

    // 清除Session
    void clearSession(const std::string &sessionId);

private:
    short port_;
    boost::asio::io_context &ioc_;
    boost::asio::ip::tcp::acceptor accept_;                     // 接收socket
    std::map<std::string, std::shared_ptr<CSession>> sessions_; // 保存客户连接
    std::mutex mutex_;
    boost::asio::steady_timer timer_; // 计时器
};