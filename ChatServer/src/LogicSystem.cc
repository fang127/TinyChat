#include "LogicSystem.h"
#include "Const.h"
#include "Defer.h"
#include "MySQLMgr.h"
#include "RedisMgr.h"
#include "StatusGrpcClient.h"

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
                it->second(msgNode->session_,
                           msgNode->recvNode_->msgId_,
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
        [this, &value, &session]() -> void
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
    value["uid"] = uid;
    value["token"] = tokenValue;
}