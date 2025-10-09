#include "CServer.h"
#include "ConfigMgr.h"

#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
int main()
{
    auto &gCfgNgr = ConfigMgr::getInstance();
    std::string gatePortStr = gCfgMgr["GateServer"]["Port"];
    unsigned short gatePort = atoi(gatePortStr.c_str());
    try
    {
        net::io_context ioc{1};
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait(
            [&ioc](const boost::system::error_code &error, int signalNumber)
            {
                if (error)
                {
                    return;
                }
                ioc.stop();
            });

        std::make_shared<CServer>(ioc, gatePort)->start();
        std::cout << "Gate Server listen on port: " << gatePort << std::endl;

        ioc.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error : " << e.what();
        return EXIT_FAILURE;
    }
}