#pragma once

#include "CSession.h"
#include "Data.h"
#include "MsgNode.h"
#include "Singleton.h"

#include <atomic>
#include <boost/asio.hpp>
#include <condition_variable>
#include <functional>
#include <jsoncpp/json/value.h>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
// 处理已连接的读写事件
// 所有的连接都要发给该类处理，因此必须考虑高并发的情况
// 采用生产者-消费者模式
// 使用消息队列 + 多线程
class LogicSystem : public Singleton<LogicSystem>
{
    using FuncCallBack = std::function<
        void(std::shared_ptr<CSession>, const short &, const std::string &)>;

public:
    LogicSystem();
    ~LogicSystem();

    void postMsgToQue(std::shared_ptr<LogicNode> msg);

private:
    // 初始化回调
    void initCallBack();

    // 处理消息队列的消息
    void dealMessage();

    // 登录回调
    void loginHandler(std::shared_ptr<CSession> session,
                      const short &msgId,
                      const std::string &msgData);
    // 搜索好友回包
    void searchUserInfo(std::shared_ptr<CSession> session,
                        const short &msgId,
                        const std::string &msgData);
    // 添加好友消息的回包
    void addFriendApply(std::shared_ptr<CSession> session,
                        const short &msgId,
                        const std::string &msgData);

    // 获取用户基本信息
    bool getBaseInfo(const std::string &key,
                     int uid,
                     std::shared_ptr<UserInfo> &userInfo);
    // 判断是否全是数字
    bool isPureDigit(const std::string &uidStr);
    // 通过uid查询用户
    void getUserByUid(const std::string &uid, Json::Value &json);
    // 通过name查询用户
    void getUserByName(const std::string &name, Json::Value &json);
    // 获取好友申请信息
    bool getFriendApplyInfo(int uid,
                            std::vector<std::shared_ptr<ApplyInfo>> &list);

    std::queue<std::shared_ptr<LogicNode>> msgQue_; // 消息队列
    std::mutex mutex_;                              // 消息队列锁
    std::condition_variable consume_;               // 消费者
    std::atomic<bool> stop_;                        // 状态
    std::map<short, FuncCallBack> funcCallBacks_;   // 回调函数
    std::thread workThread_;                        // 工作线程
};