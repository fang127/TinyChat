#pragma once

#include "Const.h"
#include "MsgNode.h"

#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

class ChatServer;

// 一个客户端的连接分配一个Session，即Session管理一个连接
// 需要处理该连接上的读写事件，以及连接断开
class CSession : public std::enable_shared_from_this<CSession>
{
public:
    CSession(boost::asio::io_context &ioc, ChatServer *server);

    ~CSession();

    boost::asio::ip::tcp::socket &getSocket();

    int getUserId();

    void setUserId(const int &userId);

    std::string getSessionId();

    std::shared_ptr<CSession> getSharedSelf();

    void start();

    void send(char *msg, short maxlen, short msgId);

    void send(const std::string &msg, short msgId);

    void close();

private:
    // 读取消息头
    void asyncReadHead(int len);

    // 读取消息体
    void asyncReadBody(int totalLen);

    void handleWrite(const boost::system::error_code &error,
                     std::shared_ptr<CSession> sharedSelf);

    boost::asio::ip::tcp::socket socket_;           // 套接字
    std::string sessionId_;                         // 连接id
    int userId_;                                    // 用户id
    ChatServer *server_;                            // 主线程服务器
    std::atomic<bool> close_;                       // 是否关闭连接
    char data_[MAX_LENGTH];                         // 本地缓冲区
    std::queue<std::shared_ptr<SendNode>> sendQue_; // 要发送的消息队列
    std::mutex sendLock_;                           // 发生消息队列锁
    std::shared_ptr<RecvNode> recvMsgNode_;         // 接收到的消息体
    std::shared_ptr<MsgNode> recvHeadNode_;         // 接收的消息头步信息
    std::atomic<std::time_t> lastRecvTime_;         // 上次接收数据的时间
    std::mutex sessionMutex_;                       // session锁
};

class LogicNode
{
    friend class LogicSystem;

public:
    LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);

    ~LogicNode() = default;

private:
    std::shared_ptr<CSession> session_;
    std::shared_ptr<RecvNode> recvNode_;
};