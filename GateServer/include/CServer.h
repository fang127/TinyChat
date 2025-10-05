#pragma once

#include "const.h"

class CServer : public std::enable_shared_from_this<CServer>
{
public:
    CServer(boost::asio::io_context &ioc, unsigned short &port);

    void start();

private:
    tcp::acceptor acceptor_;
    net::io_context &ioc_;
    tcp::socket socket_;
};