#include "CServer.h"

CServer::CServer(boost::asio::io_context &ioc, unsigned short &port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)), socket_(ioc)
{
}