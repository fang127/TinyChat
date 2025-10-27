#include "LogicSystem.h"
#include "ConfigMgr.h"
#include "Const.h"
#include "Defer.h"
#include "MySQLMgr.h"
#include "RedisMgr.h"
#include "UserMgr.h"

#include <iostream>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

LogicSystem::LogicSystem()
{
    initCallBack();
    workThread_ = std::thread(&LogicSystem::dealMessage, this);
}

LogicSystem::~LogicSystem() {}
// 初始化回调
void LogicSystem::initCallBack()
{
    funcCallBacks_[MSG_CHAT_LOGIN] =
        std::bind(&LogicSystem::loginHandler, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::postMsgToQue(std::shared_ptr<LogicNode> msg)
{
    std::unique_lock<std::mutex> lock(mutex_);
    msgQue_.push(msg);
    // 当msgQue为1时发送信号，表示可以有消费者取消息
    if (msgQue_.size() == 1)
    {
        lock.unlock();
        consume_.notify_one();
    }
}

// 处理消息队列的消息
void LogicSystem::dealMessage()
{
    while (1)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 判断队列为空则用条件变量阻塞等待，并释放锁
        while (msgQue_.empty() && !stop_)
        {
            consume_.wait(lock);
        }

        // 判断是否关闭
        if (stop_)
        {
            while (!msgQue_.empty())
            {
                auto msgNode = msgQue_.front();
                std::cout << "LogicSystem::dealMessage info {\nmsgId = "
                          << msgNode->recvNode_->msgId_ << std::endl;
                // 找到对应的回调
                auto it = funcCallBacks_.find(msgNode->recvNode_->msgId_);
                if (it == funcCallBacks_.end())
                {
                    // 找不到删除
                    msgQue_.pop();
                    continue;
                }
                // 找到执行
                it->second(msgNode->session_, msgNode->recvNode_->msgId_,
                           msgNode->recvNode_->data_);
                msgQue_.pop();
            }
            break;
        }

        // 如果stop为false，并且队伍中有数据
        auto msgNode = msgQue_.front();
        std::cout << "LogicSystem::dealMessage info {\nmsgId = "
                  << msgNode->recvNode_->msgId_ << "\n}" << std::endl;
        auto it = funcCallBacks_.find(msgNode->recvNode_->msgId_);
        if (it == funcCallBacks_.end())
        {
            msgQue_.pop();
            std::cout << "msg id [" << msgNode->recvNode_->msgId_
                      << "] handler not found" << std::endl;
            continue;
        }
        // 找到执行
        it->second(msgNode->session_, msgNode->recvNode_->msgId_,
                   msgNode->recvNode_->data_);
        msgQue_.pop();
    }
}

// 登录回调
// 要根据session传来的数据去redis查看token是否正确
// 还要去mysql中获取用户账号信息
void LogicSystem::loginHandler(std::shared_ptr<CSession> session,
                               const short &msgId,
                               const std::string &msgData)
{
    Json::Reader reader;
    Json::Value root;

    // 读取消息到root
    reader.parse(msgData, root);
    auto uid = root["uid"].asInt();
    auto token = root["token"].asString();
    std::cout << "User login info {\nuid = " << uid << "\ntoken = " << token
              << "\n}" << std::endl;

    Json::Value value;
    // Defer类确保函数结束触发session的send函数回包
    Defer defer(
        [this, &value, session]() -> void
        {
            std::string returnStr = value.toStyledString();
            session->send(returnStr, MSG_CHAT_LOGIN_RSP);
        });
    // 从redis获取用户token是否正确
    std::string uidStr = std::to_string(uid);
    std::string tokenKey = USERTOKENPREFIX + uidStr;
    std::string tokenValue = "";

    bool success = RedisMgr::getInstance()->get(tokenKey, tokenValue);
    if (!success)
    {
        value["error"] = ErrorCodes::UidInvalid;
        return;
    }

    if (tokenValue != token)
    {
        value["error"] = ErrorCodes::TokenInvalid;
        return;
    }

    value["error"] = ErrorCodes::Success;

    std::string baseKey = USER_BASE_INFO + uid;
    auto userInfo = std::make_shared<UserInfo>();
    success = getBaseInfo(baseKey, uid, userInfo);
    if (success)
    {
        value["error"] = ErrorCodes::TokenInvalid;
        return;
    }

    value["uid"] = uid;
    value["passwd"] = userInfo->passwd_;
    value["name"] = userInfo->name_;
    value["email"] = userInfo->email_;
    value["nick"] = userInfo->nick_;
    value["desc"] = userInfo->desc_;
    value["sex"] = userInfo->sex_;
    value["icon"] = userInfo->icon_;
    value["token"] = tokenValue;

    // 从mysql获取申请列表

    // 获取好友列表

    // 增加登录的chatServer的数量
    auto serverName = ConfigMgr::getInstance()["SelfServer"]["Name"];
    auto result = RedisMgr::getInstance()->hget(LOGIN_COUNT, serverName);
    int count = 0;
    if (!result.empty())
    {
        count = std::stoi(result);
    }

    ++count;
    // 将数量写入到redis
    auto countStr = std::to_string(count);
    RedisMgr::getInstance()->hset(LOGIN_COUNT, serverName, countStr);

    // session绑定用户uid
    session->setUserId(uid);
    // 为用户设置ip server的名字
    std::string ipKey = USERIPPREFIX + uidStr;
    RedisMgr::getInstance()->set(ipKey, serverName);
    // uid和session绑定，方便以后踢人操作
    UserMgr::getInstance()->setUserSession(uid, session);
}

bool LogicSystem::getBaseInfo(const std::string &key,
                              int uid,
                              std::shared_ptr<UserInfo> &userInfo)
{
    // 先查redis，没查到去查mysql
    std::string infoStr = "";
    bool success = RedisMgr::getInstance()->get(key, infoStr);
    if (success)
    {
        Json::Reader reader;
        Json::Value root;
        reader.parse(infoStr, root);
        userInfo->uid_ = root["uid"].asInt();
        userInfo->name_ = root["name"].asString();
        userInfo->passwd_ = root["passwd"].asString();
        userInfo->email_ = root["email"].asString();
        userInfo->nick_ = root["nick"].asString();
        userInfo->desc_ = root["desc"].asString();
        userInfo->sex_ = root["sex"].asInt();
        userInfo->icon_ = root["icon"].asString();
        std::cout << "user login uid is  " << userInfo->uid_ << " name  is "
                  << userInfo->name_ << " passwd is " << userInfo->passwd_
                  << " email is " << userInfo->email_ << std::endl;
    }
    else
    {
        std::shared_ptr<UserInfo> user = nullptr;
        user = MySQLMgr::getInstance()->getUser(uid);
        if (user == nullptr)
        {
            return false;
        }

        userInfo = user;

        // 将数据库内容写入redis缓存
        Json::Value redisRoot;
        redisRoot["uid"] = uid;
        redisRoot["passwd"] = userInfo->passwd_;
        redisRoot["name"] = userInfo->name_;
        redisRoot["email"] = userInfo->email_;
        redisRoot["nick"] = userInfo->nick_;
        redisRoot["desc"] = userInfo->desc_;
        redisRoot["sex"] = userInfo->sex_;
        redisRoot["icon"] = userInfo->icon_;
        RedisMgr::getInstance()->set(key, redisRoot.toStyledString());
    }
    return true;
}
