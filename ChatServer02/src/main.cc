#include "AsioIOServicePool.h"
#include "ChatServer.h"
#include "ChatServiceImpl.h"
#include "ConfigMgr.h"
#include "LogicSystem.h"
#include "RedisMgr.h"

#include <csignal>
#include <mutex>
#include <thread>

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
    auto &cfg = ConfigMgr::getInstance();
    auto serverName = cfg["SelfServer"]["Name"];
    try
    {
        auto pool = AsioIOServicePool::getInstance();
        // 初始设置登录数为0
        RedisMgr::getInstance()->hset(LOGIN_COUNT, serverName, "0");

        // 定义一个GrpcServer
        std::string serverAdd(cfg["SelfServer"]["Host"] + ":" +
                              cfg["SelfServer"]["RPCPort"]);
        ChatServiceImpl service;
        grpc::ServerBuilder builder;
        // 监听端口和注册服务
        builder.AddListeningPort(serverAdd, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        // 构建并启动RPC服务器
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "RPC Server listening on " << serverAdd << std::endl;
        // 单独启动一个线程处理grpc服务
        std::thread grpcServerThread([&server]() { server->Wait(); });

        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&io_context, pool, &server](auto, auto)
            {
                io_context.stop();
                pool->stop();
                server->Shutdown();
            });
        auto port_str = cfg["SelfServer"]["Port"];
        ChatServer s(io_context, std::stoi(port_str.c_str()));
        io_context.run();

        RedisMgr::getInstance()->hdel(LOGIN_COUNT, serverName);
        RedisMgr::getInstance()->close();
        grpcServerThread.join();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}