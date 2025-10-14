#include "LogicSystem.h"
#include "HttpConnection.h"
#include "RedisMgr.h"
#include "VerifyGrpcClient.h"

#include <iostream>

void LogicSystem::reqGet(std::string url, HttpHandler handler)
{
    getHandlers_.insert(make_pair(url, handler));
}

void LogicSystem::reqPost(std::string url, HttpHandler handler)
{
    postHandlers_.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
    reqGet("/get_test",
           [](std::shared_ptr<HttpConnection> connection)
           {
               beast::ostream(connection->response_.body())
                   << "receive get_test req" << "\n";
               int i = 0;
               for (auto &elem : connection->getParams_)
               {
                   i++;
                   beast::ostream(connection->response_.body())
                       << "param " << i << " key is " << elem.first;
                   beast::ostream(connection->response_.body())
                       << " , " << i << "value is " << elem.second << std::endl;
               }
           });

    reqPost("/get_verifycode",
            [](std::shared_ptr<HttpConnection> connection)
            {
                auto bodyStr = boost::beast::buffers_to_string(
                    connection->request_.body().data());
                std::cout << "receive body is " << bodyStr << std::endl;
                connection->response_.set(http::field::content_type,
                                          "text/json");
                Json::Value root;
                Json::Reader reader;
                Json::Value srcRoot;
                bool parseSuccess = reader.parse(bodyStr, srcRoot);
                if (!parseSuccess)
                {
                    std::cout << "failed to parse JSON data!" << std::endl;
                    root["error"] = ErrorCodes::ErrorJson;
                    std::string jsonStr = root.toStyledString();
                    beast::ostream(connection->response_.body()) << jsonStr;
                    return true;
                }

                if (!srcRoot.isMember("email"))
                {
                    std::cout << "failed to parse JSON data!" << std::endl;
                    root["error"] = ErrorCodes::ErrorJson;
                    std::string jsonStr = root.toStyledString();
                    beast::ostream(connection->response_.body()) << jsonStr;
                    return true;
                }

                auto email = srcRoot["email"].asString();
                GetVerifyRsp rsp =
                    VerifyGrpcClient::getInstance()->getVerifyCode(email);
                std::cout << "email is " << email << std::endl;
                root["error"] = rsp.error();
                root["email"] = srcRoot["email"];
                std::string jsonStr = root.toStyledString();
                beast::ostream(connection->response_.body()) << jsonStr;
                return true;
            });

    reqPost(
        "/user_register",
        [](std::shared_ptr<HttpConnection> connection)
        {
            // 打印报文主体
            auto bodyStr = boost::beast::buffers_to_string(
                connection->request_.body().data());
            std::cout << "receive body is " << bodyStr << std::endl;
            // 设置响应报文
            connection->response_.set(http::field::content_type, "text/json");
            Json::Value root;
            Json::Reader reader;
            Json::Value srcRoot;
            // 解析报文主体到srcRoot
            bool parseSuccess = reader.parse(bodyStr, srcRoot);
            if (!parseSuccess)
            {
                std::cout << "failed to parse JSON data!" << std::endl;
                root["error"] = ErrorCodes::ErrorJson;
                std::string jsonStr = root.toStyledString();
                beast::ostream(connection->response_.body()) << jsonStr;
                return true;
            }

            // 判断密码是否和确认密码一致
            auto email = srcRoot["email"].asString();
            auto name = srcRoot["user"].asString();
            auto passwd = srcRoot["passwd"].asString();
            auto confirm = srcRoot["confirm"].asString();

            if (passwd != confirm)
            {
                std::cout << "password err" << std::endl;
                root["error"] = ErrorCodes::PasswdErr;
                std::string jsonStr = root.toStyledString();
                beast::ostream(connection->response_.body()) << jsonStr;
                return true;
            }

            // 查找redis中email对应的验证码是否正确
            std::string verifyCode;
            bool getVerify =
                RedisMgr::getInstance()->get(codePrefix + email, verifyCode);
            if (!getVerify)
            {
                std::cout << "get verify code expired" << std::endl;
                root["error"] = ErrorCodes::VerifyExpired;
                std::string jsonStr = root.toStyledString();
                beast::ostream(connection->response_.body()) << jsonStr;
                return true;
            }

            if (verifyCode != srcRoot["verifycode"].asString())
            {
                std::cout << "verify code error" << std::endl;
                root["error"] = ErrorCodes::VerifyCodeErr;
                std::string jsonStr = root.toStyledString();
                beast::ostream(connection->response_.body()) << jsonStr;
                return true;
            }

            // 查找数据库判断用户是否存在mysql

            // 发送数据
            root["error"] = 0;
            root["email"] = email;
            root["user"] = name;
            root["passwd"] = passwd;
            root["confirm"] = confirm;
            root["verifycode"] = srcRoot["verifycode"].asString();
            std::string jsonStr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonStr;

            return true;
        });
}

bool LogicSystem::handleGet(std::string path,
                            std::shared_ptr<HttpConnection> con)
{
    if (getHandlers_.find(path) == getHandlers_.end())
    {
        return false;
    }

    getHandlers_[path](con);
    return true;
}

bool LogicSystem::handlePost(std::string path,
                             std::shared_ptr<HttpConnection> con)
{
    if (postHandlers_.find(path) == postHandlers_.end())
    {
        return false;
    }

    postHandlers_[path](con);
    return true;
}