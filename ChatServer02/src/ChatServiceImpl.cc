#include "ChatServiceImpl.h"
#include "CSession.h"
#include "ConfigMgr.h"
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
    // value["desc"] = request->desc();
    // value["icon"] = request->icon();
    // value["sex"] = request->sex();
    // value["nick"] = request->nick();

    std::string data = value.toStyledString();
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
    return true;
}
