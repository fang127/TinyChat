#include "LogicSystem.h"
#include <HttpConnection.h>
#include <iostream>

void LogicSystem::reqGet(std::string url, HttpHandler handler)
{
    getHandlers_.insert(make_pair(url, handler));
}

void LogicSystem::reqPost(std::string url, HttpHandler handler)
{
    postHandlers_.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
    reqGet("/get_test",
           [](std::shared_ptr<HttpConnection> connection)
           {
               beast::ostream(connection->response_.body())
                   << "receive get_test req" << "\n";
               int i = 0;
               for (auto &elem : connection->getParams_)
               {
                   i++;
                   beast::ostream(connection->response_.body())
                       << "param " << i << " key is " << elem.first;
                   beast::ostream(connection->response_.body())
                       << " , " << i << "value is " << elem.second << std::endl;
               }
           });

    reqPost("/get_varifycode",
            [](std::shared_ptr<HttpConnection> connection)
            {
                auto bodyStr = boost::beast::buffers_to_string(
                    connection->request_.body().data());
                std::cout << "receive body is " << bodyStr << std::endl;
                connection->response_.set(http::field::content_type,
                                          "text/json");
                Json::Value root;
                Json::Reader reader;
                Json::Value srcRoot;
                bool parseSuccess = reader.parse(bodyStr, srcRoot);
                if (!parseSuccess)
                {
                    std::cout << "failed to parse JSON data!" << std::endl;
                    root["error"] = ErrorCodes::ErrorJson;
                    std::string jsonStr = root.toStyledString();
                    beast::ostream(connection->response_.body()) << jsonStr;
                    return true;
                }

                if (!srcRoot.isMember("email"))
                {
                    std::cout << "failed to parse JSON data!" << std::endl;
                    root["error"] = ErrorCodes::ErrorJson;
                    std::string jsonStr = root.toStyledString();
                    beast::ostream(connection->response_.body()) << jsonStr;
                    return true;
                }

                auto email = srcRoot["email"].asString();
                std::cout << "email is " << email << std::endl;
                root["error"] = 0;
                root["email"] = srcRoot["email"];
                std::string jsonStr = root.toStyledString();
                beast::ostream(connection->response_.body()) << jsonStr;
                return true;
            });
}

bool LogicSystem::handleGet(std::string path,
                            std::shared_ptr<HttpConnection> con)
{
    if (getHandlers_.find(path) == getHandlers_.end())
    {
        return false;
    }

    getHandlers_[path](con);
    return true;
}

bool LogicSystem::handlePost(std::string path,
                             std::shared_ptr<HttpConnection> con)
{
    if (postHandlers_.find(path) == postHandlers_.end())
    {
        return false;
    }

    postHandlers_[path](con);
    return true;
}