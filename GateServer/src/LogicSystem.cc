#include "LogicSystem.h"
#include <HttpConnection.h>
#include <iostream>

void LogicSystem::reqGet(std::string url, HttpHandler handler)
{
    getHandlers_.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
    reqGet("/get_test",
           [](std::shared_ptr<HttpConnection> connection)
           {
               beast::ostream(connection->response_.body())
                   << "receive get_test req";
           });
}

bool LogicSystem::handleGet(std::string url,
                            std::shared_ptr<HttpConnection> con)
{
    if (getHandlers_.find(url) == getHandlers_.end())
    {
        return false;
    }

    getHandlers_[url](con);
    return true;
}