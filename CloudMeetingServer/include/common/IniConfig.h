#pragma once

#include <string>
#include <unordered_map>

/**
 * @file IniConfig.h
 * @brief 声明轻量级 INI 配置文件读取器。
 */

/**
 * @class IniConfig
 * @brief 解析 INI 格式配置文件，支持 [节] 和 key=value 条目。
 *
 * 使用方式：
 * @code
 *   IniConfig cfg;
 *   if (cfg.load("/path/to/server.ini")) {
 *       uint16_t port = cfg.getUInt16("Server", "tcpPort", 9000);
 *   }
 * @endcode
 */
class IniConfig
{
public:
    /**
     * @brief 从指定路径加载 INI 文件。
     * @param[in] filePath INI 文件的绝对路径。
     * @return 加载成功返回 true，文件不存在或无法打开返回 false。
     */
    bool load(const std::string &filePath);

    /**
     * @brief 读取字符串类型的配置项。
     * @param[in] section 配置节名称（区分大小写）。
     * @param[in] key     配置键名称（区分大小写）。
     * @param[in] def     键不存在时返回的默认值。
     * @return 配置值字符串，或默认值。
     */
    std::string getString(const std::string &section,
                          const std::string &key,
                          const std::string &def = "") const;

    /**
     * @brief 读取无符号 16 位整数类型的配置项。
     * @param[in] section 配置节名称。
     * @param[in] key     配置键名称。
     * @param[in] def     键不存在时返回的默认值。
     * @return 配置值，或默认值。
     */
    uint16_t getUInt16(const std::string &section,
                       const std::string &key,
                       uint16_t def = 0) const;

private:
    // 存储结构：section -> (key -> value)
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::string>> m_data;
};
