#include "CServer.h"
#include "AsioIOServicePool.h"
#include "HttpConnection.h"

#include <iostream>

CServer::CServer(boost::asio::io_context &ioc, unsigned short &port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
{
}

void CServer::start()
{
    auto self = shared_from_this();
    auto &ioc = AsioIOServicePool::getInstance()->getIOService();
    std::shared_ptr<HttpConnection> newConn =
        std::make_shared<HttpConnection>(ioc);
    acceptor_.async_accept(newConn->getSocket(),
                           [self, newConn](beast::error_code ec)
                           {
                               try
                               {
                                   // 出错放弃当前连接,但是监听其他连接
                                   if (ec)
                                   {
                                       self->start();
                                       return;
                                   }
                                   // 创建一个新连接，并且创建HttpConnection类管理该连接
                                   newConn->start();

                                   // 继续监听
                                   self->start();
                               }
                               catch (const std::exception &e)
                               {
                                   std::cout << "exception is " << e.what()
                                             << std::endl;
                                   self->start();
                               }
                           });
}