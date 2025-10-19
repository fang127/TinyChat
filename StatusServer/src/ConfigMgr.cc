#include "ConfigMgr.h"

ConfigMgr::ConfigMgr()
{
    // 拼接地址
    boost::filesystem::path currentPath = boost::filesystem::current_path();
    boost::filesystem::path configPath = currentPath / "config.ini";
    std::cout << "Config path : " << configPath << std::endl;

    // 处理ini文件
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(configPath.string(), pt);

    for (const auto &sectionPair : pt)
    {
        const std::string &sectionName = sectionPair.first;
        const boost::property_tree::ptree &sectionTree = sectionPair.second;
        std::map<std::string, std::string> sectionConfig;
        for (const auto &keyValuePair : sectionTree)
        {
            const std::string &key = keyValuePair.first;
            const std::string &value =
                keyValuePair.second.get_value<std::string>();
            sectionConfig[key] = value;
        }

        SectionInfo sectionInfo;
        sectionInfo.sectionDatas_ = std::move(sectionConfig);
        configMap_[sectionName] = sectionInfo;
    }

    // 输出所有的section和key-value对
    for (const auto &section : configMap_)
    {
        const std::string &sectionName = section.first;
        SectionInfo sectionConfig = section.second;
        std::cout << "[" << sectionName << "]" << std::endl;
        for (const auto &keyValuePair : sectionConfig.sectionDatas_)
        {
            std::cout << keyValuePair.first << "=" << keyValuePair.second
                      << std::endl;
        }
    }
}