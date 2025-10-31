#include "LogicSystem.h"
#include "ChatGrpcClient.h"
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
    // 用户登录回包
    funcCallBacks_[MSG_CHAT_LOGIN] =
        std::bind(&LogicSystem::loginHandler, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);
    // 用户搜索好友回包
    funcCallBacks_[ID_SEARCH_USER_REQ] =
        std::bind(&LogicSystem::searchUserInfo, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);
    funcCallBacks_[ID_ADD_FRIEND_REQ] =
        std::bind(&LogicSystem::addFriendApply, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);
    funcCallBacks_[ID_AUTH_FRIEND_REQ] =
        std::bind(&LogicSystem::authFriendApply, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);
    funcCallBacks_[ID_TEXT_CHAT_MSG_REQ] =
        std::bind(&LogicSystem::dealChatTextMsg, this, std::placeholders::_1,
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

    std::string baseKey = std::string(USER_BASE_INFO) + std::to_string(uid);
    auto userInfo = std::make_shared<UserInfo>();
    success = getBaseInfo(baseKey, uid, userInfo);
    if (!success)
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
    std::vector<std::shared_ptr<ApplyInfo>> applyList;
    success = getFriendApplyInfo(uid, applyList);
    if (success)
    {
        for (auto &apply : applyList)
        {
            Json::Value obj;
            obj["name"] = apply->name_;
            obj["uid"] = apply->uid_;
            obj["nick"] = apply->nick_;
            obj["sex"] = apply->sex_;
            obj["status"] = apply->status_;

            obj["icon"] = apply->icon_;
            obj["desc"] = apply->desc_;
            value["apply list"].append(obj);
        }
    }
    // 获取好友列表
    std::vector<std::shared_ptr<UserInfo>> friendList;
    success = getFriendList(uid, friendList);
    for (auto &friendItem : friendList)
    {
        Json::Value obj;
        obj["name"] = friendItem->name_;
        obj["uid"] = friendItem->uid_;
        obj["icon"] = friendItem->icon_;
        obj["nick"] = friendItem->nick_;
        obj["sex"] = friendItem->sex_;
        obj["desc"] = friendItem->desc_;
        obj["back"] = friendItem->back_;
        value["friend list"].append(obj);
    }

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

void LogicSystem::searchUserInfo(std::shared_ptr<CSession> session,
                                 const short &msgId,
                                 const std::string &msgData)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msgData, root);
    auto uidStr = root["uid"].asString();

    // 设置返回的数据
    Json::Value value;

    Defer defer(
        [this, &value, session]()
        {
            std::string data = value.toStyledString();
            session->send(data, ID_SEARCH_USER_RSP);
        });

    // 用户传来的可能是name或者uid
    bool isDigit = isPureDigit(uidStr);
    if (isDigit)
    {
        // 是数字则说明是uid
        getUserByUid(uidStr, value);
    }
    else
    {
        // 不是通过name查询
        getUserByName(uidStr, value);
    }
}

void LogicSystem::addFriendApply(std::shared_ptr<CSession> session,
                                 const short &msgId,
                                 const std::string &msgData)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msgData, root);
    auto uid = root["uid"].asInt();
    auto applyName = root["applyName"].asString();
    auto backName = root["backName"].asString();
    auto toUid = root["toUid"].asInt();
    std::cout << "receive addFriendReq: " << root.toStyledString() << std::endl;
    Json::Value value;
    value["error"] = ErrorCodes::Success;
    Defer defer(
        [this, &value, session]()
        {
            auto data = value.toStyledString();
            session->send(data, ID_ADD_FRIEND_RSP);
        });

    // 先更新数据库
    MySQLMgr::getInstance()->addFriendApply(uid, toUid);

    // 查找redis，找到toUid在那个服务器
    auto toStr = std::to_string(toUid);
    auto key = USERIPPREFIX + toStr;
    std::string peerName = "";
    bool success = RedisMgr::getInstance()->get(key, peerName);
    if (!success)
    {
        return;
    }

    auto &cfg = ConfigMgr::getInstance();
    auto selfName = cfg["SelfServer"]["Name"];

    // 先查找自己的详细信息
    std::string keySelfInfo = std::string(USER_BASE_INFO) + std::to_string(uid);
    auto applyInfo = std::make_shared<UserInfo>();
    success = getBaseInfo(keySelfInfo, uid, applyInfo);

    // 通知对方服务器有申请消息
    // 如果在同一台服务器
    if (peerName == selfName)
    {
        // 在内存中获取对方的session
        auto session = UserMgr::getInstance()->getSession(toUid);
        // 对方在线
        if (session)
        {
            Json::Value response;
            response["error"] = ErrorCodes::Success;
            response["applyUid"] = uid;
            response["name"] = applyName;
            response["desc"] = "";
            if (success)
            {
                response["icon"] = applyInfo->icon_;
                response["sex"] = applyInfo->sex_;
                response["nick"] = applyInfo->nick_;
            }
            std::string data = response.toStyledString();
            session->send(data, ID_NOTIFY_ADD_FRIEND_REQ);
        }

        return;
    }

    // 使用grpc发送给对端
    message::AddFriendReq request;
    request.set_applyuid(uid);
    request.set_touid(toUid);
    request.set_name(applyName);
    request.set_desc("");
    if (success)
    {
        request.set_icon(applyInfo->icon_);
        request.set_sex(applyInfo->sex_);
        request.set_nick(applyInfo->nick_);
    }

    ChatGrpcClient::getInstance()->notifyAddFriend(peerName, request);
}

void LogicSystem::authFriendApply(std::shared_ptr<CSession> session,
                                  const short &msgId,
                                  const std::string &msgData)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msgData, root);
    auto fromUid = root["fromUid"].asInt();
    auto toUid = root["toUid"].asInt();
    auto backName = root["backName"].asString();
    std::cout << __func__ << __LINE__ << "auth friend: fromUid" << fromUid
              << " to " << toUid << std::endl;

    Json::Value value;
    value["error"] = ErrorCodes::Success;
    auto userInfo = std::make_shared<UserInfo>();
    std::string key = std::string(USER_BASE_INFO) + std::to_string(toUid);
    bool success = getBaseInfo(key, toUid, userInfo);
    if (success)
    {
        value["name"] = userInfo->name_;
        value["nick"] = userInfo->nick_;
        value["sex"] = userInfo->sex_;
        value["icon"] = userInfo->icon_;
        value["uid"] = userInfo->uid_;
    }
    else
    {
        value["error"] = ErrorCodes::UidInvalid;
    }

    Defer defer(
        [this, &value, session]()
        {
            auto data = value.toStyledString();
            session->send(data, ID_AUTH_FRIEND_RSP);
        });
    // 可以修复为事务，利用ACID特性
    // 更新数据库
    MySQLMgr::getInstance()->authFriendApply(fromUid, toUid);
    // 更新数据库添加好友
    MySQLMgr::getInstance()->addFriend(fromUid, toUid, backName);

    // 查询redis找到touid所在服务器
    auto uidStr = std::to_string(toUid);
    key = USERIPPREFIX + uidStr;
    std::string serverName = "";
    success = RedisMgr::getInstance()->get(key, serverName);
    if (!success)
    {
        return;
    }

    auto &cfg = ConfigMgr::getInstance();
    auto selfName = cfg["SelfServer"]["Name"];
    // 直接通知对方有认证通过消息
    if (serverName == selfName)
    {
        auto session = UserMgr::getInstance()->getSession(toUid);
        if (session)
        {
            // 在内存中则直接发送通知对方
            Json::Value notify;
            notify["error"] = ErrorCodes::Success;
            notify["fromUid"] = fromUid;
            notify["toUid"] = toUid;
            std::string baseKey = USER_BASE_INFO + std::to_string(fromUid);
            auto userInfo = std::make_shared<UserInfo>();
            bool getinfo = getBaseInfo(baseKey, fromUid, userInfo);
            if (getinfo)
            {
                notify["name"] = userInfo->name_;
                notify["nick"] = userInfo->nick_;
                notify["icon"] = userInfo->icon_;
                notify["sex"] = userInfo->sex_;
            }
            else
            {
                notify["error"] = ErrorCodes::UidInvalid;
            }

            std::string return_str = notify.toStyledString();
            session->send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
        }

        return;
    }
    // 发送grpc给对端
    message::AuthFriendReq request;
    request.set_fromuid(fromUid);
    request.set_touid(toUid);
    // 发送通知
    ChatGrpcClient::getInstance()->notifyAuthFriend(serverName, request);
}

void LogicSystem::dealChatTextMsg(std::shared_ptr<CSession> session,
                                  const short &msgId,
                                  const std::string &msgData)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msgData, root);

    auto uid = root["fromUid"].asInt();
    auto touid = root["toUid"].asInt();

    const Json::Value arrays = root["text_array"];

    Json::Value value;
    value["error"] = ErrorCodes::Success;
    value["text_array"] = arrays;
    value["fromUid"] = uid;
    value["toUid"] = touid;

    Defer defer(
        [this, &value, session]()
        {
            std::string data = value.toStyledString();
            session->send(data, ID_TEXT_CHAT_MSG_RSP);
        });

    // 查询redis 查找touid对应的server ip
    auto toUidStr = std::to_string(touid);
    auto key = USERIPPREFIX + toUidStr;
    std::string serverName = "";
    bool success = RedisMgr::getInstance()->get(key, serverName);
    if (!success)
    {
        return;
    }

    auto &cfg = ConfigMgr::getInstance();
    auto selfName = cfg["SelfServer"]["Name"];
    // 直接通知对方有认证通过消息
    if (serverName == selfName)
    {
        auto session = UserMgr::getInstance()->getSession(touid);
        if (session)
        {
            // 在内存中则直接发送通知对方
            std::string return_str = value.toStyledString();
            session->send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
        }

        return;
    }

    message::TextChatMsgReq request;
    request.set_fromuid(uid);
    request.set_touid(touid);
    for (const auto &text : arrays)
    {
        auto content = text["content"].asString();
        auto msgID = text["msgID"].asString();
        std::cout << "content is " << content << std::endl;
        std::cout << "msgID is " << msgID << std::endl;
        auto *textMsg = request.add_textmsgs();
        textMsg->set_msgid(msgID);
        textMsg->set_msgcontent(content);
    }

    // 发送通知
    ChatGrpcClient::getInstance()->notifyTextChatMsg(serverName, request,
                                                     value);
}

// 检测是否是数据
bool LogicSystem::isPureDigit(const std::string &uidStr)
{
    for (auto &ch : uidStr)
    {
        if (!isdigit(ch))
        {
            return false;
        }
    }
    return true;
}

void LogicSystem::getUserByUid(const std::string &uid, Json::Value &json)
{
    json["error"] = ErrorCodes::Success;
    std::string key = std::string(USER_BASE_INFO) + uid;
    std::string info;
    // 先查询redis
    bool success = RedisMgr::getInstance()->get(key, info);
    if (success)
    {
        Json::Reader reader;
        Json::Value root;
        reader.parse(info, root);
        std::cout << __func__ << "\n" << root.asString() << std::endl;

        json["uid"] = root["uid"].asInt();
        json["name"] = root["name"].asString();
        json["passwd"] = root["passwd"].asString();
        json["email"] = root["email"].asString();
        json["nick"] = root["nick"].asString();
        json["desc"] = root["desc"].asString();
        json["sex"] = root["sex"].asInt();
        json["icon"] = root["icon"].asString();

        return;
    }

    // redis如果没有，查找数据库
    std::shared_ptr<UserInfo> userInfo = nullptr;
    userInfo = MySQLMgr::getInstance()->getUser(stoi(uid));
    if (userInfo == nullptr)
    {
        json["error"] = ErrorCodes::UidInvalid;
        return;
    }
    // 存入redis中
    Json::Value root;
    root["uid"] = userInfo->uid_;
    root["name"] = userInfo->name_;
    root["passwd"] = userInfo->passwd_;
    root["email"] = userInfo->email_;
    root["nick"] = userInfo->nick_;
    root["desc"] = userInfo->desc_;
    root["sex"] = userInfo->sex_;
    root["icon"] = userInfo->icon_;

    RedisMgr::getInstance()->set(key, root.toStyledString());

    // 返回数据
    json["uid"] = userInfo->uid_;
    json["name"] = userInfo->name_;
    json["passwd"] = userInfo->passwd_;
    json["email"] = userInfo->email_;
    json["nick"] = userInfo->nick_;
    json["desc"] = userInfo->desc_;
    json["sex"] = userInfo->sex_;
    json["icon"] = userInfo->icon_;
}

void LogicSystem::getUserByName(const std::string &name, Json::Value &json)
{
    json["error"] = ErrorCodes::Success;
    std::string key = NAME_INFO + name;
    std::string info = "";
    // 先查询redis
    bool success = RedisMgr::getInstance()->get(key, info);
    if (success)
    {
        Json::Reader reader;
        Json::Value root;
        reader.parse(info, root);
        // std::cout << __func__ << "\n" << root.asString() << std::endl;

        json["uid"] = root["uid"].asInt();
        json["name"] = root["name"].asString();
        json["passwd"] = root["passwd"].asString();
        json["email"] = root["email"].asString();
        json["nick"] = root["nick"].asString();
        json["desc"] = root["desc"].asString();
        json["sex"] = root["sex"].asInt();
        json["icon"] = root["icon"].asString();

        return;
    }

    // redis如果没有，查找数据库
    std::shared_ptr<UserInfo> userInfo = nullptr;
    userInfo = MySQLMgr::getInstance()->getUser(name);
    if (userInfo == nullptr)
    {
        json["error"] = ErrorCodes::UidInvalid;
        return;
    }
    // 存入redis中
    Json::Value root;
    root["uid"] = userInfo->uid_;
    root["name"] = userInfo->name_;
    root["passwd"] = userInfo->passwd_;
    root["email"] = userInfo->email_;
    root["nick"] = userInfo->nick_;
    root["desc"] = userInfo->desc_;
    root["sex"] = userInfo->sex_;
    root["icon"] = userInfo->icon_;

    RedisMgr::getInstance()->set(key, root.toStyledString());

    // 返回数据
    json["uid"] = userInfo->uid_;
    json["name"] = userInfo->name_;
    json["passwd"] = userInfo->passwd_;
    json["email"] = userInfo->email_;
    json["nick"] = userInfo->nick_;
    json["desc"] = userInfo->desc_;
    json["sex"] = userInfo->sex_;
    json["icon"] = userInfo->icon_;
}

bool LogicSystem::getFriendApplyInfo(
    int uid,
    std::vector<std::shared_ptr<ApplyInfo>> &list)
{
    // 最多返回10条
    return MySQLMgr::getInstance()->getApplyList(uid, list, 0, 10);
}

bool LogicSystem::getFriendList(
    int uid,
    std::vector<std::shared_ptr<UserInfo>> &friendList)
{
    return MySQLMgr::getInstance()->getFriendList(uid, friendList);
}