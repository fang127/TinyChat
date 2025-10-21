#pragma once

#include "RedisConnPool.h"
#include "Singleton.h"

#include <memory>

class RedisMgr : public Singleton<RedisMgr>,
                 public std::enable_shared_from_this<RedisMgr>
{
    friend class Singleton<RedisMgr>;

public:
    ~RedisMgr();
    bool get(const std::string &key, std::string &value);
    bool set(const std::string &key, const std::string &value);
    bool auth(const std::string &password);
    bool lpush(const std::string &key, const std::string &value);
    bool lpop(const std::string &key, std::string &value);
    bool rpush(const std::string &key, const std::string &value);
    bool rpop(const std::string &key, std::string &value);
    bool hset(const std::string &key,
              const std::string &hkey,
              const std::string &value);
    bool hset(const char *key,
              const char *hkey,
              const char *hvalue,
              size_t hvaluelen);
    std::string hget(const std::string &key, const std::string &hkey);
    bool del(const std::string &key);
    bool existsKey(const std::string &key);
    void close();

private:
    RedisMgr();

    std::unique_ptr<RedisConnPool> pool_;
};