#include "VerifyGrpcClient.h"

RPConnPool::RPConnPool(std::size_t poolsize,
                       const std::string &host,
                       const std::string &port)
    : poolSize_(poolsize), host_(host), port_(port), bStop_(false)
{
    for (std::size_t i = 0; i < poolSize_; ++i)
    {
        std::shared_ptr<Channel> channel = grpc::CreateChannel(
            host + ":" + port, grpc::InsecureChannelCredentials());

        connections_.push(VerifyService::NewStub(channel));
    }
}

RPConnPool::~RPConnPool()
{
    std::lock_guard<std::mutex> lock(mutex_);
    close();
    while (!connections_.empty())
    {
        connections_.pop();
    }
}

void RPConnPool::close()
{
    bStop_ = true;
    cond_.notify_all();
}

std::size_t RPConnPool::poolSize() {}

GetVerifyRsp VerifyGrpcClient::getVerifyCode(std::string email)
{
    ClientContext context;
    GetVerifyRsp reply;
    GetVerifyReq request;
    request.set_email(email);

    Status status = stub_->GetVerifyCode(&context, request, &reply);

    if (status.ok())
    {
        return reply;
    }
    else
    {
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}

VerifyGrpcClient::VerifyGrpcClient() {}