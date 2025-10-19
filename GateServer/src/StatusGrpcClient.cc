#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "Defer.h"

StatusConnPool::StatusConnPool(std::size_t size,
                               const std::string &host,
                               const std::string &port)
    : sizePool_(size), host_(host), port_(port), stop_(false)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
            host + ":" + port, grpc::InsecureChannelCredentials());
        connectionPool_.push(message::StatusService::NewStub(channel));
    }
}

StatusConnPool::~StatusConnPool()
{
    std::lock_guard<std::mutex> lock(mutex_);
    close();
    while (!connectionPool_.empty())
    {
        connectionPool_.pop();
    }
}

void StatusConnPool::close()
{
    stop_ = true;
    cond_.notify_all();
}

std::unique_ptr<message::StatusService::Stub> StatusConnPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    cond_.wait(lock,
               [this]()
               {
                   if (stop_)
                   {
                       return true;
                   }

                   return !connectionPool_.empty();
               });

    if (stop_)
    {
        return nullptr;
    }

    auto conn = std::move(connectionPool_.front());
    connectionPool_.pop();

    return conn;
}

void StatusConnPool::returnConnection(
    std::unique_ptr<message::StatusService::Stub> conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (stop_)
    {
        return;
    }

    connectionPool_.push(std::move(conn));
    cond_.notify_one();
}

StatusGrpcClient::StatusGrpcClient()
{
    auto &cfg = ConfigMgr::getInstance();
    std::string host = cfg["StatusServer"]["Host"];
    std::string port = cfg["StatusServer"]["Port"];
    pool_ = std::make_unique<StatusConnPool>(5, host, port);
}

// 获取聊天服务器
message::GetChatServerRsp StatusGrpcClient::getChatServer(int uid)
{
    grpc::ClientContext context;
    message::GetChatServerRsp reply;
    message::GetChatServerReq request;
    request.set_uid(uid);
    auto stub = pool_->getConnection();

    Defer defer([this, &stub]() { pool_->returnConnection(std::move(stub)); });

    grpc::Status status = stub->GetChatServer(&context, request, &reply);

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

// 登录到服务器
message::LoginRsp StatusGrpcClient::login(int uid, std::string token)
{
    grpc::ClientContext context;
    message::LoginRsp reply;
    message::LoginReq request;
    request.set_uid(uid);
    request.set_token(token);

    auto stub = pool_->getConnection();

    Defer defer([this, &stub]() { pool_->returnConnection(std::move(stub)); });

    grpc::Status status = stub->Login(&context, request, &reply);

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