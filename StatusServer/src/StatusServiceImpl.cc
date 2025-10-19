#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "Const.h"
#include "RedisMgr.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

std::string generateUniqueString()
{
    // 创建UUID对象
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    // 将UUID转换为字符串
    std::string uniqueString = to_string(uuid);

    return uniqueString;
}

ChatServer::ChatServer() : host_(""), port_(""), name_(""), count_(0) {}

ChatServer &ChatServer::operator=(const ChatServer &cs)
{
    if (&cs == this)
    {
        return *this;
    }

    host_ = cs.host_;
    port_ = cs.port_;
    name_ = cs.name_;
    count_ = cs.count_;
    return *this;
}

StatusServiceImpl::StatusServiceImpl()
{
    // 获取配置
    auto &cfg = ConfigMgr::getInstance();
    auto serverName = cfg["ChatServers"]["Name"];

    std::vector<std::string> words;

    std::stringstream ss(serverName);
    std::string word;

    while (std::getline(ss, word, ','))
    {
        words.push_back(word);
    }
    // 创建服务器
    for (auto &word : words)
    {
        if (cfg[word]["Name"].empty())
        {
            continue;
        }

        ChatServer server;
        server.host_ = cfg[word]["Host"];
        server.port_ = cfg[word]["Port"];
        server.name_ = cfg[word]["Name"];

        servers_[server.name_] = server;
    }
}

grpc::Status
StatusServiceImpl::GetChatServer(::grpc::ServerContext *context,
                                 const message::GetChatServerReq *request,
                                 message::GetChatServerRsp *response)
{
    std::string prefix("TinyChat status server has received: ");
    const auto &server = getChatServer();
    response->set_host(server.host_);
    response->set_port(server.port_);
    response->set_token(generateUniqueString());
    response->set_error(ErrorCodes::Success);
    insertToken(request->uid(), response->token());
    return grpc::Status::OK;
}

grpc::Status StatusServiceImpl::Login(grpc::ServerContext *context,
                                      const message::LoginReq *request,
                                      message::LoginRsp *response)
{
    auto uid = request->uid();
    auto token = request->token();

    std::string uidStr = std::to_string(uid);
    std::string tokenKey = USERTOKENPREFIX + uidStr;
    std::string tokenValue = "";
    if (!RedisMgr::getInstance()->get(uidStr, tokenValue))
    {
        response->set_error(ErrorCodes::UidInvalid);
        return grpc::Status::OK;
    }

    if (tokenValue != token)
    {
        response->set_error(ErrorCodes::TokenInvalid);
        return grpc::Status::OK;
    }

    response->set_error(ErrorCodes::Success);
    response->set_uid(uid);
    response->set_token(token);
    return grpc::Status::OK;
}

// 保存token到redis
void StatusServiceImpl::insertToken(int uid, std::string token)
{
    std::string uidStr = std::to_string(uid);
    std::string tokenKey = USERTOKENPREFIX + uidStr;
    RedisMgr::getInstance()->set(tokenKey, token);
}

// 获取服务器
ChatServer StatusServiceImpl::getChatServer()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 获取第一个服务器
    auto minServer = servers_.begin()->second;
    // 获取服务器连接数
    auto countStr = RedisMgr::getInstance()->hget(LOGIN_COUNT, minServer.name_);
    if (countStr.empty())
    {
        // 不存在则默认设置为最大
        minServer.count_ = INT_MAX;
    }
    else
    {
        minServer.count_ = static_cast<std::size_t>(std::stoi(countStr));
    }

    // 使用范围for循环获取服务器
    for (auto &server : servers_)
    {

        if (server.second.name_ == minServer.name_)
        {
            continue;
        }

        auto countStr =
            RedisMgr::getInstance()->hget(LOGIN_COUNT, server.second.name_);
        if (countStr.empty())
        {
            server.second.count_ = INT_MAX;
        }
        else
        {
            server.second.count_ =
                static_cast<std::size_t>(std::stoi(countStr));
        }

        if (server.second.count_ < minServer.count_)
        {
            minServer = server.second;
        }
    }

    return minServer;
}