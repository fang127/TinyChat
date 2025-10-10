#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

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
    // 首先获取互斥锁，确保在销毁过程中没有其他线程在操作连接池。
    std::lock_guard<std::mutex> lock(mutex_);
    // 设置停止标志 bStop_ 并唤醒所有可能在等待连接的线程。
    close();
    // 循环清空连接队列，销毁所有剩余的存根对象
    while (!connections_.empty())
    {
        connections_.pop();
    }
}

void RPConnPool::close()
{
    // 设置一个布尔标志，告诉所有方法连接池正在关闭，不应再提供或接收连接。
    bStop_ = true;
    cond_.notify_all();
}

std::unique_ptr<VerifyService::Stub> RPConnPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);
    // 如果连接队列 connections_ 为空 并且 连接池没有被关闭
    // (!bStop_)，当前线程会在这里阻塞，并临时释放锁，允许其他线程（比如归还连接的线程）操作队列。
    cond_.wait(lock,
               [this]()
               {
                   if (bStop_)
                       return true;
                   return !connections_.empty();
               });
    // 如果是因为连接池关闭而被唤醒的（bStop_ 为
    // true），则返回空指针，表示无法提供连接。
    if (bStop_)
    {
        return nullptr;
    }
    // 从队列中取出一个连接（存根对象），并将其从队列中移除。
    auto context_ = std::move(connections_.front());
    connections_.pop();
    return context_;
}

void RPConnPool::returnConnection(std::unique_ptr<VerifyService::Stub> context)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 如果连接池已经关闭，直接返回，丢弃该连接。
    if (bStop_)
        return;
    // 将归还的连接放回队列中。
    connections_.push(std::move(context));
    cond_.notify_one();
}

GetVerifyRsp VerifyGrpcClient::getVerifyCode(std::string email)
{
    ClientContext context;
    GetVerifyRsp reply;
    GetVerifyReq request;
    request.set_email(email);
    // 从连接池获取一个 gRPC 存根对象
    // 是一个阻塞调用，直到有可用的连接为止
    auto stub = pool_->getConnection();
    // 执行 gRPC 调用，也是阻塞调用，直到服务器响应或发生错误
    Status status = stub->GetVerifyCode(&context, request, &reply);

    // 根据 gRPC 调用的结果处理响应
    if (status.ok())
    {
        pool_->returnConnection(std::move(stub));
        return reply;
    }
    else
    {
        pool_->returnConnection(std::move(stub));
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}

VerifyGrpcClient::VerifyGrpcClient()
{
    auto &gCfgMgr = ConfigMgr::getInstance();
    std::string host = gCfgMgr["VerifyServer"]["Host"];
    std::string port = gCfgMgr["VerifyServer"]["Port"];
    pool_.reset(new RPConnPool(5, host, port));
}