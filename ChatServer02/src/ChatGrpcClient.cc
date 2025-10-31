#include "ChatGrpcClient.h"
#include "ConfigMgr.h"
#include "Const.h"
#include "Defer.h"
#include "RedisMgr.h"

#include <sstream>
#include <vector>

ChatConnPool::ChatConnPool(std::size_t size,
                           const std::string &host,
                           const std::string &port)
    : size_(size), host_(host), port_(port), stop_(false)
{
    for (std::size_t i = 0; i < size_; ++i)
    {
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
            host_ + ":" + port_, grpc::InsecureChannelCredentials());
        connections_.push(message::ChatService::NewStub(channel));
    }
}

ChatConnPool::~ChatConnPool()
{
    std::lock_guard<std::mutex> lock(mutex_);
    close();
    while (!connections_.empty())
    {
        connections_.pop();
    }
}

void ChatConnPool::close()
{
    stop_ = true;
    cond_.notify_all();
}

std::unique_ptr<message::ChatService::Stub> ChatConnPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    cond_.wait(lock,
               [this]()
               {
                   if (stop_)
                   {
                       return true;
                   }

                   return !connections_.empty();
               });

    if (stop_)
    {
        return nullptr;
    }

    auto conn = std::move(connections_.front());
    connections_.pop();
    return conn;
}

void ChatConnPool::returnConnection(
    std::unique_ptr<message::ChatService::Stub> conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (stop_)
    {
        return;
    }

    connections_.push(std::move(conn));
    cond_.notify_one();
}

message::AddFriendRsp
ChatGrpcClient::notifyAddFriend(std::string serverName,
                                const message::AddFriendReq &request)
{
    message::AddFriendRsp response;

    Defer defer(
        [&response, &request]()
        {
            response.set_error(ErrorCodes::Success);
            response.set_applyuid(request.applyuid());
            response.set_touid(request.touid());
        });

    // 从连接池拿到一个与对端的grpc连接
    std::cout << "ChatGrpcClient: notifyAddFriend to serverName='" << serverName
              << "'" << std::endl;
    auto it = pool_.find(serverName);
    if (it == pool_.end())
    {
        std::cout << "ChatGrpcClient: pool for '" << serverName << "' not found"
                  << std::endl;
        return response;
    }
    // 发送grpc
    auto &pool = it->second;
    grpc::ClientContext context;
    auto stub = pool->getConnection();
    grpc::Status status = stub->NotifyAddFriend(&context, request, &response);
    // 归还资源
    Defer deferConn([this, &stub, &pool]()
                    { pool->returnConnection(std::move(stub)); });

    if (!status.ok())
    {
        std::cout << "ChatGrpcClient: NotifyAddFriend RPC to '" << serverName
                  << "' failed: " << status.error_message()
                  << " (code=" << status.error_code() << ")" << std::endl;
        response.set_error(ErrorCodes::RPCFailed);
        return response;
    }

    return response;
}

message::AuthFriendRsp
ChatGrpcClient::notifyAuthFriend(std::string serverName,
                                 const message::AuthFriendReq &req)
{
    message::AuthFriendRsp rsp;
    rsp.set_error(ErrorCodes::Success);

    Defer defer(
        [&rsp, &req]()
        {
            rsp.set_fromuid(req.fromuid());
            rsp.set_touid(req.touid());
        });

    auto it = pool_.find(serverName);
    if (it == pool_.end())
    {
        return rsp;
    }

    auto &pool = it->second;
    grpc::ClientContext context;
    auto stub = pool->getConnection();
    grpc::Status status = stub->NotifyAuthFriend(&context, req, &rsp);
    Defer deferConn([&stub, this, &pool]()
                    { pool->returnConnection(std::move(stub)); });

    if (!status.ok())
    {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    return rsp;
}

bool ChatGrpcClient::getBaseInfo(std::string base_key,
                                 int uid,
                                 std::shared_ptr<UserInfo> &userinfo)
{
    return true;
}

message::TextChatMsgRsp
ChatGrpcClient::notifyTextChatMsg(std::string server_ip,
                                  const message::TextChatMsgReq &req,
                                  const Json::Value &value)
{
    message::TextChatMsgRsp rsp;
    rsp.set_error(ErrorCodes::Success);

    Defer defer(
        [&rsp, &req]()
        {
            rsp.set_fromuid(req.fromuid());
            rsp.set_touid(req.touid());
            for (const auto &textData : req.textmsgs())
            {
                message::TextChatData *newMsg = rsp.add_textmsgs();
                newMsg->set_msgid(textData.msgid());
                newMsg->set_msgcontent(textData.msgcontent());
            }
        });

    auto it = pool_.find(server_ip);
    if (it == pool_.end())
    {
        return rsp;
    }

    auto &pool = it->second;
    grpc::ClientContext context;
    auto stub = pool->getConnection();
    grpc::Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
    Defer deferConn([&stub, this, &pool]()
                    { pool->returnConnection(std::move(stub)); });

    if (!status.ok())
    {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    return rsp;
}

ChatGrpcClient::ChatGrpcClient()
{
    // 获取配置信息pool
    auto &cfg = ConfigMgr::getInstance();
    auto serverInfoList = cfg["PeerServer"]["Servers"];

    std::stringstream str(serverInfoList);
    std::vector<std::string> words;
    std::string word;
    while (std::getline(str, word, ','))
    {
        words.push_back(word);
    }

    // 构建pool_
    for (auto &word : words)
    {
        if (cfg[word]["Name"].empty())
        {
            continue;
        }

        std::string name = cfg[word]["Name"];
        std::string host = cfg[word]["Host"];
        std::string port = cfg[word]["RPCPort"];
        std::cout << "ChatGrpcClient: creating pool for '" << name << "' -> "
                  << host << ":" << port << std::endl;
        pool_[name] = std::make_unique<ChatConnPool>(5, host, port);
    }
}
