#pragma once

#define MAX_LENGTH 1024 * 2

enum ErrorCodes
{
    Success = 0,
    ErrorJson = 1001,      // json解析失败
    RPCFailed = 1002,      // RPC请求错误
    VerifyExpired = 1003,  // 验证码过去
    VerifyCodeErr = 1004,  // 验证码错误
    UserExist = 1005,      // 用户已存在
    PasswdErr = 1006,      // 确认密码不一致
    EmailNotMatch = 1007,  // 邮箱不匹配
    PassedUpFailed = 1008, // 更新密码失败
    PasswdInvalid = 1009,  // 密码无效
    TokenInvalid = 1010,   // Token失效
    UidInvalid = 1011,     // uid无效
};

#define codePrefix "code_";
#define USERIPPREFIX "uip_"
#define USERTOKENPREFIX "utoken_"
#define IPCOUNTPREFIX "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT "logincount"
#define NAME_INFO "nameinfo_"
#define LOCK_PREFIX "lock_"
#define USER_SESSION_PREFIX "usession_"
#define LOCK_COUNT "lockcount"

// 头部总长度
#define HEAD_TOTAL_LEN 4
// 头部id长度
#define HEAD_ID_LEN 2
// 头部数据长度
#define HEAD_DATA_LEN 2
#define MAX_RECVQUE 10000
#define MAX_SENDQUE 1000

enum MSG_IDS
{
    MSG_CHAT_LOGIN = 1005,              // 用户登陆
    MSG_CHAT_LOGIN_RSP = 1006,          // 用户登陆回包
    ID_SEARCH_USER_REQ = 1007,          // 用户搜索请求
    ID_SEARCH_USER_RSP = 1008,          // 搜索用户回包
    ID_ADD_FRIEND_REQ = 1009,           // 申请添加好友请求
    ID_ADD_FRIEND_RSP = 1010,           // 申请添加好友回复
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,    // 通知用户添加好友申请
    ID_AUTH_FRIEND_REQ = 1013,          // 认证好友请求
    ID_AUTH_FRIEND_RSP = 1014,          // 认证好友回复
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015,   // 通知用户认证好友申请
    ID_TEXT_CHAT_MSG_REQ = 1017,        // 文本聊天信息请求
    ID_TEXT_CHAT_MSG_RSP = 1018,        // 文本聊天信息回复
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, // 通知用户文本聊天信息
    ID_NOTIFY_OFF_LINE_REQ = 1021,      // 通知用户下线
    ID_HEART_BEAT_REQ = 1023,           // 心跳请求
    ID_HEARTBEAT_RSP = 1024,            // 心跳回复
};