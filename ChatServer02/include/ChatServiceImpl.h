#pragma once

#include "Data.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include <mutex>

class ChatServiceImpl final : public message::ChatService::Service
{
public:
    ChatServiceImpl() = default;
    ~ChatServiceImpl() = default;
    grpc::Status NotifyAddFriend(grpc::ServerContext *context,
                                 const message::AddFriendReq *request,
                                 message::AddFriendRsp *response);
    grpc::Status ReplyAddFriend(grpc::ServerContext *context,
                                const message::ReplyFriendReq *request,
                                message::ReplyFriendRsp *response);
    grpc::Status SendChatMsg(grpc::ServerContext *context,
                             const message::SendChatMsgReq *request,
                             message::SendChatMsgRsp *response);
    grpc::Status NotifyAuthFriend(grpc::ServerContext *context,
                                  const message::AuthFriendReq *request,
                                  message::AuthFriendRsp *response);
    grpc::Status NotifyTextChatMsg(grpc::ServerContext *context,
                                   const message::TextChatMsgReq *request,
                                   message::TextChatMsgRsp *response);

    bool getBaseInfo(std::string baseKey,
                     int uid,
                     std::shared_ptr<UserInfo> &userInfo);

private:
};