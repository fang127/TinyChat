#pragma once

#include <map>
#include <string>

// 管理config.ini的key和value
struct SectionInfo
{
    SectionInfo() = default;
    ~SectionInfo() { sectionDatas_.clear(); }

    std::map<std::string, std::string> sectionDatas_;

    SectionInfo(const SectionInfo &src) { sectionDatas_ = src.sectionDatas_; }

    SectionInfo &operator=(const SectionInfo &src)
    {
        // 防止拷贝自己
        if (&src == this)
        {
            return *this;
        }
        sectionDatas_ = src.sectionDatas_;
        return *this;
    }

    std::string operator[](const std::string &key)
    {
        // 返回value
        if (sectionDatas_.find(key) == sectionDatas_.end())
        {
            return "";
        }
        return sectionDatas_[key];
    }
};

class ConfigMgr
{
public:
    ~ConfigMgr() { configMap_.clear(); }

    ConfigMgr(const ConfigMgr &ths) = delete;

    ConfigMgr &operator=(const ConfigMgr &ths) = delete;

    SectionInfo operator[](const std::string &section)
    {
        if (configMap_.find(section) == configMap_.end())
        {
            return SectionInfo();
        }

        return configMap_[section];
    }

    // 懒汉单例模式
    static ConfigMgr &getInstance()
    {
        static ConfigMgr cfgMgr;
        return cfgMgr;
    }

private:
    ConfigMgr();
    // key = Session name  value = SectionInfo
    std::map<std::string, SectionInfo> configMap_;
};