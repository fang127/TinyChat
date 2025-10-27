#include "ChatServiceImpl.h"
#include "CSession.h"
#include "MySQLMgr.h"
#include "RedisMgr.h"
#include "UserMgr.h"

#include <jsoncpp/json/json.h>

grpc::Status
ChatServiceImpl::NotifyAddFriend(grpc::ServerContext *context,
                                 const message::AddFriendReq *request,
                                 message::AddFriendRsp *response)
{
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
