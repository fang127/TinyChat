#include "RedisMgr.h"

bool RedisMgr::connect(const std::string &host, int port)
{
    this->connect_ = redisConnect(host.c_str(), port);
    if (connect_ == nullptr)
    {
        return false;
    }
    if (this->connect_ != nullptr && this->connect_->err)
    {
        std::cout << "connect error " << this->connect_->errstr << std::endl;
        return false;
    }

    return true;
}

bool RedisMgr::get(const std::string &key, std::string &value)
{
    reply_ = (redisReply *)redisCommand(connect_, "GET %s", key.c_str());
    if (reply_ == nullptr)
    {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        freeReplyObject(reply_);
        return false;
    }

    if (reply_->type != REDIS_REPLY_STRING)
    {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        freeReplyObject(reply_);
        return false;
    }

    value = reply_->str;
    freeReplyObject(reply_);

    std::cout << "Succeed to execute command [ GET " << key << " ]"
              << std::endl;
    return true;
}

bool RedisMgr::set(const std::string &key, const std::string &value)
{
    reply_ = (redisReply *)redisCommand(connect_, "SET %s %s", key.c_str(),
                                        value.c_str());
    if (reply_ == nullptr)
    {
        std::cout << "Execute command [ SET " << key << " " << value
                  << " ] failed !" << std::endl;
        freeReplyObject(reply_);

        return false;
    }

    if (!(this->reply_->type == REDIS_REPLY_STATUS &&
          (strcmp(this->reply_->str, "OK") == 0 ||
           strcmp(this->reply_->str, "ok") == 0)))
    {
        std::cout << "Execute command [ SET " << key << " " << value
                  << " ] failed !" << std::endl;
        freeReplyObject(reply_);

        return false;
    }

    freeReplyObject(reply_);
    std::cout << "Execute command [ SET " << key << "  " << value
              << " ] success ! " << std::endl;
    return true;
}

bool RedisMgr::auth(const std::string &password)
{
    reply_ = (redisReply *)redisCommand(connect_, "AUTH %s", password.c_str());
    if (reply_->type == REDIS_REPLY_ERROR)
    {
        std::cout << "认证失败" << std::endl;
        freeReplyObject(reply_);
        return false;
    }
    else
    {
        std::cout << "认证成功" << std::endl;
        freeReplyObject(reply_);
        return true;
    }
}

bool RedisMgr::lpush(const std::string &key, const std::string &value)
{
    reply_ = (redisReply *)redisCommand(connect_, "LPUSH %s %s", key.c_str(),
                                        value.c_str());
    if (reply_ == nullptr)
    {
        std::cout << "Execute command [ LPUSH " << key << " " << value
                  << " ] failed !" << std::endl;
        freeReplyObject(reply_);
        return false;
    }

    if (reply_->type != REDIS_REPLY_INTEGER || reply_->integer <= 0)
    {
        std::cout << "Execute command [ LPUSH " << key << " " << value
                  << " ] failed !" << std::endl;
        freeReplyObject(reply_);
        return false;
    }

    std::cout << "Execute command [ LPUSH " << key << " " << value
              << " ] success !" << std::endl;
    freeReplyObject(reply_);
    return true;
}

bool RedisMgr::lpop(const std::string &key, std::string &value)
{
    this->reply_ =
        (redisReply *)redisCommand(this->connect_, "LPOP %s ", key.c_str());
    if (reply_ == nullptr || reply_->type == REDIS_REPLY_NIL)
    {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! "
                  << std::endl;
        freeReplyObject(this->reply_);
        return false;
    }
    value = reply_->str;
    std::cout << "Execut command [ LPOP " << key << " ] success ! "
              << std::endl;
    freeReplyObject(this->reply_);
    return true;
}

bool RedisMgr::rpush(const std::string &key, const std::string &value)
{
    this->reply_ = (redisReply *)redisCommand(this->connect_, "RPUSH %s %s",
                                              key.c_str(), value.c_str());
    if (NULL == this->reply_)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value
                  << " ] failure ! " << std::endl;
        freeReplyObject(this->reply_);
        return false;
    }

    if (this->reply_->type != REDIS_REPLY_INTEGER || this->reply_->integer <= 0)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value
                  << " ] failure ! " << std::endl;
        freeReplyObject(this->reply_);
        return false;
    }

    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] success ! " << std::endl;
    freeReplyObject(this->reply_);
    return true;
}

bool RedisMgr::rpop(const std::string &key, std::string &value)
{
    this->reply_ =
        (redisReply *)redisCommand(this->connect_, "RPOP %s ", key.c_str());
    if (reply_ == nullptr || reply_->type == REDIS_REPLY_NIL)
    {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! "
                  << std::endl;
        freeReplyObject(this->reply_);
        return false;
    }
    value = reply_->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! "
              << std::endl;
    freeReplyObject(this->reply_);
    return true;
}

bool RedisMgr::hset(const std::string &key,
                    const std::string &hkey,
                    const std::string &value)
{
    this->reply_ =
        (redisReply *)redisCommand(this->connect_, "HSET %s %s %s", key.c_str(),
                                   hkey.c_str(), value.c_str());
    if (reply_ == nullptr || reply_->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
                  << value << " ] failure ! " << std::endl;
        freeReplyObject(this->reply_);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << value << " ] success ! " << std::endl;
    freeReplyObject(this->reply_);
    return true;
}

bool RedisMgr::hset(const char *key,
                    const char *hkey,
                    const char *hvalue,
                    size_t hvaluelen)
{
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
    this->reply_ =
        (redisReply *)redisCommandArgv(this->connect_, 4, argv, argvlen);
    if (reply_ == nullptr || reply_->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
                  << hvalue << " ] failure ! " << std::endl;
        freeReplyObject(this->reply_);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << hvalue << " ] success ! " << std::endl;
    freeReplyObject(this->reply_);
    return true;
}

std::string RedisMgr::hget(const std::string &key, const std::string &hkey)
{
    const char *argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    this->reply_ =
        (redisReply *)redisCommandArgv(this->connect_, 3, argv, argvlen);
    if (this->reply_ == nullptr || this->reply_->type == REDIS_REPLY_NIL)
    {
        freeReplyObject(this->reply_);
        std::cout << "Execut command [ HGet " << key << " " << hkey
                  << "  ] failure ! " << std::endl;
        return "";
    }

    std::string value = this->reply_->str;
    freeReplyObject(this->reply_);
    std::cout << "Execut command [ HGet " << key << " " << hkey
              << " ] success ! " << std::endl;
    return value;
}

bool RedisMgr::del(const std::string &key)
{
    this->reply_ =
        (redisReply *)redisCommand(this->connect_, "DEL %s", key.c_str());
    if (this->reply_ == nullptr || this->reply_->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ Del " << key << " ] failure ! "
                  << std::endl;
        freeReplyObject(this->reply_);
        return false;
    }
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
    freeReplyObject(this->reply_);
    return true;
}

bool RedisMgr::existsKey(const std::string &key)
{
    this->reply_ =
        (redisReply *)redisCommand(this->connect_, "exists %s", key.c_str());
    if (this->reply_ == nullptr || this->reply_->type != REDIS_REPLY_INTEGER ||
        this->reply_->integer == 0)
    {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(this->reply_);
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(this->reply_);
    return true;
}

void RedisMgr::close() { redisFree(connect_); }