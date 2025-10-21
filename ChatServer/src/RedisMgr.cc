#include "RedisMgr.h"
#include "ConfigMgr.h"

#include <iostream>
#include <string.h>

RedisMgr::RedisMgr()
{
    auto &gCfgMgr = ConfigMgr::getInstance();
    auto host = gCfgMgr["Redis"]["Host"];
    auto port = gCfgMgr["Redis"]["Port"];
    auto password = gCfgMgr["Redis"]["Passwd"];
    pool_.reset(new RedisConnPool(5, host, std::stoi(port), password));
}

RedisMgr::~RedisMgr() { close(); }

bool RedisMgr::get(const std::string &key, std::string &value)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(connect, "GET %s", key.c_str());
    if (reply == nullptr)
    {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING)
    {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);
        return false;
    }

    value = reply->str;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    std::cout << "Succeed to execute command [ GET " << key << " ]"
              << std::endl;
    return true;
}

bool RedisMgr::set(const std::string &key, const std::string &value)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(connect, "SET %s %s", key.c_str(),
                                            value.c_str());
    if (reply == nullptr)
    {
        std::cout << "Execute command [ SET " << key << " " << value
                  << " ] failed !" << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }

    if (!(reply->type == REDIS_REPLY_STATUS &&
          (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))
    {
        std::cout << "Execute command [ SET " << key << " " << value
                  << " ] failed !" << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }

    freeReplyObject(reply);
    std::cout << "Execute command [ SET " << key << "  " << value
              << " ] success ! " << std::endl;
    pool_->returnConnection(connect);

    return true;
}

bool RedisMgr::auth(const std::string &password)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply =
        (redisReply *)redisCommand(connect, "AUTH %s", password.c_str());
    if (reply->type == REDIS_REPLY_ERROR)
    {
        std::cout << "认证失败" << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }
    else
    {
        std::cout << "认证成功" << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return true;
    }
}

bool RedisMgr::lpush(const std::string &key, const std::string &value)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(connect, "LPUSH %s %s", key.c_str(),
                                            value.c_str());
    if (reply == nullptr)
    {
        std::cout << "Execute command [ LPUSH " << key << " " << value
                  << " ] failed !" << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0)
    {
        std::cout << "Execute command [ LPUSH " << key << " " << value
                  << " ] failed !" << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }

    std::cout << "Execute command [ LPUSH " << key << " " << value
              << " ] success !" << std::endl;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    return true;
}

bool RedisMgr::lpop(const std::string &key, std::string &value)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(connect, "LPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! "
                  << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ LPOP " << key << " ] success ! "
              << std::endl;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    return true;
}

bool RedisMgr::rpush(const std::string &key, const std::string &value)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(connect, "RPUSH %s %s", key.c_str(),
                                            value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value
                  << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value
                  << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }

    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] success ! " << std::endl;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    return true;
}

bool RedisMgr::rpop(const std::string &key, std::string &value)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(connect, "RPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! "
                  << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! "
              << std::endl;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    return true;
}

bool RedisMgr::hset(const std::string &key,
                    const std::string &hkey,
                    const std::string &value)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(
        connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
                  << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    return true;
}

bool RedisMgr::hset(const char *key,
                    const char *hkey,
                    const char *hvalue,
                    size_t hvaluelen)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    const char *argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;

    auto reply = (redisReply *)redisCommandArgv(connect, 4, argv, argvlen);

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
                  << hvalue << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << hvalue << " ] success ! " << std::endl;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    return true;
}

std::string RedisMgr::hget(const std::string &key, const std::string &hkey)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return "";
    }

    const char *argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    auto reply = (redisReply *)redisCommandArgv(connect, 3, argv, argvlen);
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL)
    {
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        std::cout << "Execut command [ HGet " << key << " " << hkey
                  << "  ] failure ! " << std::endl;
        return "";
    }

    std::string value = reply->str;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    std::cout << "Execut command [ HGet " << key << " " << hkey
              << " ] success ! " << std::endl;
    return value;
}

bool RedisMgr::del(const std::string &key)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(connect, "DEL %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ Del " << key << " ] failure ! "
                  << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    return true;
}

bool RedisMgr::existsKey(const std::string &key)
{
    auto connect = pool_->getConnection();
    if (connect == nullptr)
    {
        return false;
    }

    auto reply = (redisReply *)redisCommand(connect, "exists %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER ||
        reply->integer == 0)
    {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(reply);
        pool_->returnConnection(connect);

        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(reply);
    pool_->returnConnection(connect);

    return true;
}

void RedisMgr::close() { pool_->close(); }