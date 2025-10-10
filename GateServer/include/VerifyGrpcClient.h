#pragma once
#include "Const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class RPConnPool
{
public:
    RPConnPool(std::size_t poolsize,
               const std::string &host,
               const std::string &port);
    ~RPConnPool();

    void close();

    std::unique_ptr<VerifyService::Stub> getConnection();

    void returnConnection(std::unique_ptr<VerifyService::Stub> context);

private:
    std::atomic<bool> bStop_;
    std::size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<VerifyService::Stub>> connections_;
    std::condition_variable cond_;
    std::mutex mutex_;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;

public:
    GetVerifyRsp getVerifyCode(std::string email);

private:
    VerifyGrpcClient();

    std::unique_ptr<RPConnPool> pool_;
};