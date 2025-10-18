#include "HttpConnection.h"
#include "LogicSystem.h"
#include <iostream>

HttpConnection::HttpConnection(boost::asio::io_context &ioc) : socket_(ioc) {}

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
                             // 主要作用抑制编译器警告，因为没有用到这个参数，编译器会发出警告
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
        preParseGetParam();
        bool success =
            LogicSystem::getInstance()->handleGet(getUrl_, shared_from_this());
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
    // 处理post
    if (request_.method() == http::verb::post)
    {
        bool success = LogicSystem::getInstance()->handlePost(
            std::string(request_.target()), shared_from_this());
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
                          // 关闭服务器的写端
                          self->socket_.shutdown(tcp::socket::shutdown_send,
                                                 ec);
                          // 取消定时器
                          self->deadline_.cancel();
                      });
}

unsigned char toHex(unsigned char x) { return x > 9 ? x + 55 : x + 48; }

unsigned char fromHex(unsigned char x)
{
    unsigned char y;
    if (x >= 'A' && x <= 'Z')
        y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        y = x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        y = x - '0';
    else
        assert(0);
    return y;
}

std::string urlEncode(const std::string &str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        // 判断是否仅有数字和字母构成
        if (isalnum((unsigned char)str[i]) || (str[i] == '-') ||
            (str[i] == '_') || (str[i] == '.') || (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ') // 为空字符
            strTemp += "+";
        else
        {
            // 其他字符需要提前加%并且高四位和低四位分别转为16进制
            strTemp += '%';
            strTemp += toHex((unsigned char)str[i] >> 4);
            strTemp += toHex((unsigned char)str[i] & 0x0F);
        }
    }
    return strTemp;
}

std::string urlDecode(const std::string &str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        // 还原+为空
        if (str[i] == '+')
            strTemp += ' ';
        // 遇到%将后面的两个字符从16进制转为char再拼接
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = fromHex((unsigned char)str[++i]);
            unsigned char low = fromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else
            strTemp += str[i];
    }
    return strTemp;
}

void HttpConnection::preParseGetParam()
{
    // 提取 URI
    auto uri = request_.target();
    // 查找查询字符串的开始位置（即 '?' 的位置）
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos)
    {
        getUrl_ = std::string(uri);
        return;
    }

    getUrl_ = std::string(uri).substr(0, query_pos);
    std::string query_string = std::string(uri).substr(query_pos + 1);
    std::string key;
    std::string value;
    size_t pos = 0;
    while ((pos = query_string.find('&')) != std::string::npos)
    {
        auto pair = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos)
        {
            key = urlDecode(
                pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码
            value = urlDecode(pair.substr(eq_pos + 1));
            getParams_[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    // 处理最后一个参数对（如果没有 & 分隔符）
    if (!query_string.empty())
    {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos)
        {
            key = urlDecode(query_string.substr(0, eq_pos));
            value = urlDecode(query_string.substr(eq_pos + 1));
            getParams_[key] = value;
        }
    }
}