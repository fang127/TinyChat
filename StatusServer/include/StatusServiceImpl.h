#pragma once

#include "message.grpc.pb.h"
#include "message.pb.h"

#include <grpcpp/grpcpp.h>
#include <string>

class ChatServer
{
    friend class StatusServiceImpl;

public:
    ChatServer();
    ~ChatServer() = default;

    ChatServer &operator=(const ChatServer &cs);

private:
    std::string host_;
    std::string port_;
    std::string name_;
    std::size_t count_;
};

class StatusServiceImpl final : public message::StatusService::Service
{
public:
    StatusServiceImpl();

    ~StatusServiceImpl() = default;

    grpc::Status GetChatServer(::grpc::ServerContext *context,
                               const message::GetChatServerReq *request,
                               message::GetChatServerRsp *response) override;

    grpc::Status Login(grpc::ServerContext *context,
                       const message::LoginReq *request,
                       message::LoginRsp *response) override;

private:
    void insertToken(int uid, std::string token);
    ChatServer getChatServer();

    std::unordered_map<std::string, ChatServer> servers_;
    std::mutex mutex_;
};