#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "StatusServiceImpl.h"
#include <boost/asio.hpp>
#include <thread>

void runServer()
{
    // 获取配置
    auto &cfg = ConfigMgr::getInstance();
    std::string serverAdd =
        cfg["StatusServer"]["Host"] + ":" + cfg["StatusServer"]["Port"];

    StatusServiceImpl service;

    grpc::ServerBuilder builder;
    // 监听端口
    builder.AddListeningPort(serverAdd, grpc::InsecureServerCredentials());
    // 注册服务
    builder.RegisterService(&service);

    // 启动服务器
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << serverAdd << std::endl;

    // 创建asio捕获sigint信号，优雅关闭服务器
    boost::asio::io_context ioc;
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&server, &ioc](const boost::system::error_code error, int signalNumber)
        {
            if (!error)
            {
                std::cout << "Shutting down server..." << std::endl;
                // 优雅关闭gRPC服务器（停止接受新请求，等待现有请求处理完成）
                server->Shutdown();
                ioc.stop();
            }
        });

    // 后台线程监听该信号，避免阻塞主线程的grpc服务
    std::thread([&ioc]() { ioc.run(); }).detach();

    // 等grpc服务器阻塞等待客户端请求，直到信号到达关闭
    server->Wait();
}

int main()
{
    try
    {
        runServer();
        RedisMgr::getInstance()->close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        RedisMgr::getInstance()->close();
        return EXIT_FAILURE;
    }

    return 0;
}