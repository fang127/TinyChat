#include "AsioIOServicePool.h"
#include "ChatServer.h"
#include "ConfigMgr.h"
#include "LogicSystem.h"
#include <csignal>
#include <mutex>
#include <thread>
using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
    try
    {
        auto &cfg = ConfigMgr::getInstance();
        LogicSystem::getInstance(); // 提前初始化 LogicSystem
        auto pool = AsioIOServicePool::getInstance();
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&](auto, auto)
            {
                io_context.stop();
                pool->stop();
                pool.reset();
            });
        auto port = cfg["SelfServer"]["Port"];
        ChatServer s(io_context, std::stoi(port.c_str()));
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << endl;
    }
}