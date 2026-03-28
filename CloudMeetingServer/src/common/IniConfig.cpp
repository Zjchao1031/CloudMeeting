/**
 * @file IniConfig.cpp
 * @brief 实现轻量级 INI 配置文件读取器。
 */
#include "common/IniConfig.h"
#include <fstream>
#include <sstream>
#include <algorithm>

bool IniConfig::load(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string currentSection;
    std::string line;
    while (std::getline(file, line)) {
        // 去除首尾空白。
        auto ltrim = [](std::string &s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                [](unsigned char c){ return !std::isspace(c); }));
        };
        auto rtrim = [](std::string &s) {
            s.erase(std::find_if(s.rbegin(), s.rend(),
                [](unsigned char c){ return !std::isspace(c); }).base(), s.end());
        };
        ltrim(line); rtrim(line);

        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            // 解析节名。
            currentSection = line.substr(1, line.size() - 2);
        } else {
            // 解析 key=value。
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            std::string key   = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            rtrim(key); ltrim(value);
            m_data[currentSection][key] = value;
        }
    }
    return true;
}

std::string IniConfig::getString(const std::string &section,
                                  const std::string &key,
                                  const std::string &def) const
{
    auto sit = m_data.find(section);
    if (sit == m_data.end()) return def;
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return def;
    return kit->second;
}

uint16_t IniConfig::getUInt16(const std::string &section,
                               const std::string &key,
                               uint16_t def) const
{
    std::string val = getString(section, key);
    if (val.empty()) return def;
    try {
        unsigned long n = std::stoul(val);
        if (n > 65535) return def;
        return static_cast<uint16_t>(n);
    } catch (...) {
        return def;
    }
}
