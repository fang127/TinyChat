#include "AsioIOServicePool.h"
#include "ChatServer.h"
#include "ConfigMgr.h"
#include "LogicSystem.h"
#include <csignal>
#include <mutex>
#include <thread>

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
    try
    {
        auto &cfg = ConfigMgr::getInstance();
        auto pool = AsioIOServicePool::getInstance();
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&io_context, pool](auto, auto)
            {
                io_context.stop();
                pool->stop();
            });
        auto port_str = cfg["SelfServer"]["Port"];
        ChatServer s(io_context, std::stoi(port_str.c_str()));
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}