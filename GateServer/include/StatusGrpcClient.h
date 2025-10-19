#pragma once

#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <atomic>
#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include <memory>
#include <mutex>
#include <queue>
// grpc客户端连接池
class StatusConnPool
{
public:
    StatusConnPool(std::size_t size,
                   const std::string &host,
                   const std::string &port);
    ~StatusConnPool();

    void close();

    std::unique_ptr<message::StatusService::Stub> getConnection();

    void returnConnection(std::unique_ptr<message::StatusService::Stub>);

private:
    std::size_t sizePool_;
    std::atomic<bool> stop_;
    std::condition_variable cond_;
    std::queue<std::unique_ptr<message::StatusService::Stub>> connectionPool_;
    std::string host_;
    std::string port_;
    std::mutex mutex_;
};

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;

public:
    ~StatusGrpcClient() = default;

    // 获取聊天服务器
    message::GetChatServerRsp getChatServer(int uid);

    // 登录到服务器
    message::LoginRsp login(int uid, std::string token);

private:
    StatusGrpcClient();

    std::unique_ptr<StatusConnPool> pool_;
};