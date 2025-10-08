#pragma once

#include "const.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    friend class LogicSystem;
    HttpConnection(tcp::socket socket);

    void start();

private:
    void checkDeadLine();
    void writeResponse();
    void handleReq();
    void preParseGetParam();

    tcp::socket socket_;
    beast::flat_buffer buffer_{8192};
    http::request<http::dynamic_body> request_;
    http::response<http::dynamic_body> response_;
    net::steady_timer deadline_{socket_.get_executor(),
                                std::chrono::seconds(60)};

    std::string getUrl_;
    std::unordered_map<std::string, std::string> getParams_;
};