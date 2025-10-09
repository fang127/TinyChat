#pragma once

#include "Const.h"

class HttpConnection;

typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;

class LogicSystem : public Singleton<LogicSystem>
{
public:
    ~LogicSystem() = default;
    bool handleGet(std::string url, std::shared_ptr<HttpConnection> con);
    bool handlePost(std::string url, std::shared_ptr<HttpConnection> con);

    void reqGet(std::string url, HttpHandler handler);
    void reqPost(std::string url, HttpHandler handler);

private:
    friend class Singleton<LogicSystem>;

    LogicSystem();

    std::map<std::string, HttpHandler> postHandlers_;
    std::map<std::string, HttpHandler> getHandlers_;
};