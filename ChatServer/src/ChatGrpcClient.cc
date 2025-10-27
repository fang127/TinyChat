#include "ChatGrpcClient.h"
#include "ConfigMgr.h"
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
ChatGrpcClient::notifyAddFriend(std::string server_ip,
                                const message::AddFriendReq &req)
{
    return message::AddFriendRsp();
}

message::AuthFriendRsp
ChatGrpcClient::notifyAuthFriend(std::string server_ip,
                                 const message::AuthFriendReq &req)
{
    return message::AuthFriendRsp();
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
                                  const Json::Value &rtvalue)
{
    return message::TextChatMsgRsp();
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

        pool_[cfg[word]["Name"]] = std::make_unique<ChatConnPool>(
            5, cfg[word]["Host"], cfg[word]["Port"]);
    }
}
