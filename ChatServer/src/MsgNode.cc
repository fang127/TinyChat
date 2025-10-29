#include "MsgNode.h"
#include "Const.h"

#include <boost/asio.hpp>
#include <iostream>
#include <string.h>

MsgNode::MsgNode(short maxlen)
    : curlen_(0), totallen_(maxlen), data_(new char[totallen_ + 1]())
{
    data_[totallen_] = '\0';
}

MsgNode::~MsgNode()
{
    std::cout << "MsgNode destruct" << std::endl;
    delete[] data_;
}

void MsgNode::clear()
{
    ::memset(data_, 0, totallen_);
    curlen_ = 0;
}

RecvNode::RecvNode(short maxlen, short msgId) : MsgNode(maxlen), msgId_(msgId)
{
}

SendNode::SendNode(const char *msg, short maxlen, short msgId)
    : MsgNode(maxlen + HEAD_TOTAL_LEN), msgId_(msgId)
{
    // 消息 = id + 消息体长度 + 消息体内容
    // 主机字节序转为网络字节序
    short msgIdHost =
        boost::asio::detail::socket_ops::host_to_network_short(msgId_);
    memcpy(data_, &msgIdHost, HEAD_ID_LEN);
    short maxlenHost =
        boost::asio::detail::socket_ops::host_to_network_short(maxlen);
    memcpy(data_ + HEAD_ID_LEN, &maxlenHost, HEAD_DATA_LEN);
    // 拷贝消息体到头部之后的缓冲区
    memcpy(data_ + HEAD_ID_LEN + HEAD_DATA_LEN, msg, maxlen);
    // totallen_ 已在构造时设置为 maxlen + HEAD_TOTAL_LEN
}