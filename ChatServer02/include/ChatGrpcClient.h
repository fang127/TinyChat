#pragma once

#include "Data.h"
#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <atomic>
#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include <jsoncpp/json/json.h>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

class ChatConnPool
{
public:
    ChatConnPool(std::size_t size,
                 const std::string &host,
                 const std::string &port);
    ~ChatConnPool();

    void close();

    std::unique_ptr<message::ChatService::Stub> getConnection();

    void returnConnection(std::unique_ptr<message::ChatService::Stub> conn);

private:
    std::size_t size_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> stop_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<message::ChatService::Stub>> connections_;
};

class ChatGrpcClient : public Singleton<ChatGrpcClient>
{
    friend class Singleton<ChatGrpcClient>;

public:
    ~ChatGrpcClient() = default;
    // 通知对方加好友
    message::AddFriendRsp notifyAddFriend(std::string server_ip,
                                          const message::AddFriendReq &req);
    // 通知对方认证好友
    message::AuthFriendRsp notifyAuthFriend(std::string server_ip,
                                            const message::AuthFriendReq &req);
    // 获取信息
    bool getBaseInfo(std::string base_key,
                     int uid,
                     std::shared_ptr<UserInfo> &userinfo);
    // 文本信息
    message::TextChatMsgRsp
    notifyTextChatMsg(std::string server_ip,
                      const message::TextChatMsgReq &req,
                      const Json::Value &rtvalue);

private:
    ChatGrpcClient();
    std::unordered_map<std::string, std::unique_ptr<ChatConnPool>> pool_;
};