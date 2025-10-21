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
class CSession : std::enable_shared_from_this<CSession>
{
public:
    CSession(boost::asio::io_context &ioc, ChatServer *server);

    ~CSession();

    boost::asio::ip::tcp::socket &getSocket();

    std::string getUserId();

    void setUserId(const std::string &userId);

    std::string getSessionId();

    std::shared_ptr<CSession> getSharedSelf();

    void start();

    void send(char *msg, short maxlen, short msgId);

    void send(const std::string &msg, short msgId);

    void close();

    // 读取消息头
    void asyncReadHead(int len);

    // 读取完整消息
    void asyncReadFull(std::size_t maxlen,
                       std::function<void(const boost::system::error_code &,
                                          std::size_t)> handler);

    // 读取指定长度
    void asyncReadLen(std::size_t readlen,
                      std::size_t totalLen,
                      std::function<void(const boost::system::error_code &,
                                         std::size_t)> handler);

    // 读取消息体
    void asyncReadBody(int totalLen);

private:
    boost::asio::ip::tcp::socket socket_;           // 套接字
    std::string sessionId_;                         // 连接id
    std::string userId_;                            // 用户id
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