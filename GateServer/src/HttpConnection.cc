#include "HttpConnection.h"
#include "LogicSystem.h"
#include <iostream>

HttpConnection::HttpConnection(tcp::socket socket) : socket_(std::move(socket))
{
}

void HttpConnection::start()
{
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, request_,
                     [self](beast::error_code ec, std::size_t bytesTransferred)
                     {
                         try
                         {
                             if (ec)
                             {
                                 std::cout << "http read err is "
                                           << ec.message() << std::endl;
                                 return;
                             }

                             // 忽略掉已经发生的请求
                             boost::ignore_unused(bytesTransferred);
                             self->handleReq();
                             // 启动超时监测
                             self->checkDeadLine();
                         }
                         catch (const std::exception &e)
                         {
                             std::cerr << "exception is " << e.what()
                                       << std::endl;
                         }
                     });
}

void HttpConnection::handleReq()
{
    // 设置版本
    response_.version(request_.version());
    // 设置短链接
    response_.keep_alive(false);
    // 处理get请求
    if (request_.method() == http::verb::get)
    {
        std::string target = std::string(request_.target());
        bool success =
            LogicSystem::getInstance()->handleGet(target, shared_from_this());
        if (!success)
        {
            // 设置状态，404
            response_.result(http::status::not_found);
            // 设置字段
            response_.set(http::field::content_type, "text/plain");
            // 写入响应体
            beast::ostream(response_.body()) << "url not fount\r\n";
            writeResponse();
            return;
        }

        // 设置状态，ok
        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        writeResponse();
        return;
    }
}

void HttpConnection::checkDeadLine()
{
    auto self = shared_from_this();
    deadline_.async_wait(
        [self](beast::error_code ec)
        {
            if (!ec)
            {
                self->socket_.close(ec);
            }
        });
}

void HttpConnection::writeResponse()
{
    auto self = shared_from_this();
    response_.content_length(response_.body().size());
    http::async_write(socket_, response_,
                      [self](beast::error_code ec, std::size_t bytesTransferred)
                      {
                          // 关闭服务器的读端
                          self->socket_.shutdown(tcp::socket::shutdown_send,
                                                 ec);
                          // 取消定时器
                          self->deadline_.cancel();
                      });
}