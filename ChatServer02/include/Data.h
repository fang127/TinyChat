#pragma once

#include <string>

// 用户信息
struct UserInfo
{
    UserInfo()
        : name_(""), passwd_(""), uid_(0), email_(""), nick_(""), desc_(""),
          sex_(0), icon_(""), back_("")
    {
    }
    std::string name_;
    std::string passwd_;
    int uid_;
    std::string email_;
    std::string nick_;
    std::string desc_;
    int sex_;
    std::string icon_;
    std::string back_;
};

struct ApplyInfo
{
    ApplyInfo(int uid,
              std::string name,
              std::string desc,
              std::string icon,
              std::string nick,
              int sex,
              int status)
        : uid_(uid), name_(name), desc_(desc), icon_(icon), nick_(nick),
          sex_(sex), status_(status)
    {
    }

    int uid_;
    std::string name_;
    std::string desc_;
    std::string icon_;
    std::string nick_;
    int sex_;
    int status_;
};