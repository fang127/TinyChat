#include "HttpConnection.h"

HttpConnection::HttpConnection(tcp::socket socket) : socket_(std::move(socket))
{
}

void HttpConnection::start() { auto self = shared_from_this(); }

void HttpConnection::checkDeadLine() {}

void HttpConnection::writeResponse() {}