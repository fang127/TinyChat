#include "ChatServiceImpl.h"
#include "CSession.h"
#include "Defer.h"
#include "MySQLMgr.h"
#include "RedisMgr.h"
#include "UserMgr.h"

#include <jsoncpp/json/json.h>

grpc::Status
ChatServiceImpl::NotifyAddFriend(grpc::ServerContext *context,
                                 const message::AddFriendReq *request,
                                 message::AddFriendRsp *response)
{
    // 查找用户是否在线
    auto toUid = request->touid();
    auto session = UserMgr::getInstance()->getSession(toUid);

    Defer defer(
        [request, response]()
        {
            response->set_error(ErrorCodes::Success);
            response->set_applyuid(request->applyuid());
            response->set_touid(request->touid());
        });

    // 用户不在内存直接返回
    if (!session)
    {
        return grpc::Status::OK;
    }

    // 在内存中则直接发送通知对方
    Json::Value value;
    value["error"] = ErrorCodes::Success;
    value["applyUid"] = request->applyuid();
    value["name"] = request->name();
    value["desc"] = request->desc();
    value["icon"] = request->icon();
    value["sex"] = request->sex();
    value["nick"] = request->nick();

    std::string data = value.toStyledString();
    std::cout << __func__ << __FILE__ << std::endl;
    session->send(data, ID_NOTIFY_ADD_FRIEND_REQ);

    return grpc::Status::OK;
}

grpc::Status
ChatServiceImpl::ReplyAddFriend(grpc::ServerContext *context,
                                const message::ReplyFriendReq *request,
                                message::ReplyFriendRsp *response)
{
    return grpc::Status::OK;
}

grpc::Status
ChatServiceImpl::SendChatMsg(grpc::ServerContext *context,
                             const message::SendChatMsgReq *request,
                             message::SendChatMsgRsp *response)
{
    return grpc::Status::OK;
}

grpc::Status
ChatServiceImpl::NotifyAuthFriend(grpc::ServerContext *context,
                                  const message::AuthFriendReq *request,
                                  message::AuthFriendRsp *response)
{
    // 查找用户是否在本服务器
    auto toUid = request->touid();
    auto fromUid = request->fromuid();
    auto session = UserMgr::getInstance()->getSession(toUid);

    Defer defer(
        [request, response]()
        {
            response->set_error(ErrorCodes::Success);
            response->set_fromuid(request->fromuid());
            response->set_touid(request->touid());
        });

    // 用户不在内存中则直接返回
    if (session == nullptr)
    {
        return grpc::Status::OK;
    }

    // 在内存中则直接发送通知对方
    Json::Value value;
    value["error"] = ErrorCodes::Success;
    value["fromUid"] = request->fromuid();
    value["toUid"] = request->touid();

    std::string key = USER_BASE_INFO + std::to_string(fromUid);
    auto userInfo = std::make_shared<UserInfo>();
    bool getInfo = getBaseInfo(key, fromUid, userInfo);
    if (getInfo)
    {
        value["name"] = userInfo->name_;
        value["nick"] = userInfo->nick_;
        value["icon"] = userInfo->icon_;
        value["sex"] = userInfo->sex_;
    }
    else
    {
        value["error"] = ErrorCodes::UidInvalid;
    }

    std::string data = value.toStyledString();

    session->send(data, ID_NOTIFY_AUTH_FRIEND_REQ);
    return grpc::Status::OK;
}

grpc::Status
ChatServiceImpl::NotifyTextChatMsg(grpc::ServerContext *context,
                                   const message::TextChatMsgReq *request,
                                   message::TextChatMsgRsp *response)
{
    return grpc::Status::OK;
}

bool ChatServiceImpl::getBaseInfo(std::string baseKey,
                                  int uid,
                                  std::shared_ptr<UserInfo> &userInfo)
{
    // 先查redis，没查到去查mysql
    std::string infoStr = "";
    bool success = RedisMgr::getInstance()->get(baseKey, infoStr);
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
        RedisMgr::getInstance()->set(baseKey, redisRoot.toStyledString());
    }
    return true;
}
