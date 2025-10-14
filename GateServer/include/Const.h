#pragma once

#include "Singleton.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <hiredis/hiredis.h>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>s

enum ErrorCodes
{
    Success = 0,
    ErrorJson = 1001,      // json解析失败
    RPCFailed = 1002,      // RPC请求错误
    VerifyExpired = 1003,  // 验证码过去
    VerifyCodeErr = 1004,  // 验证码错误
    UserExist = 1005,      // 用户已存在
    PasswdErr = 1006,      // 密码错误
    EmailNotMatch = 1007,  // 邮箱不匹配
    PassedUpFailed = 1008, // 更新密码失败
    PasswdInvalid = 1009,  // 密码更新失败
};

const std::string codePrefix = "code_";