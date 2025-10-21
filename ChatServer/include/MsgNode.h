#pragma once

#include <string>

class LogicSystem;

class MsgNode
{
public:
    MsgNode(short maxlen);

    ~MsgNode();

    void clear();

    short curlen_;   // 当前读取的位置
    short totallen_; // 消息总长
    char *data_;     // 缓冲区
};

class RecvNode : public MsgNode
{
    friend class LogicSystem;

public:
    RecvNode(short maxlen, short msgId);

private:
    short msgId_; // 消息ID
};

class SendNode : public MsgNode
{
    friend class LogicSystem;

public:
    SendNode(const char *msg, short maxlen, short msgId);

private:
    short msgId_;
};