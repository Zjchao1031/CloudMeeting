#pragma once
#include <QString>
#include <QImage>

/**
 * @file UserProfileService.h
 * @brief 声明用户资料持久化服务。
 */

/**
 * @class UserProfileService
 * @brief 负责加载、保存与维护用户昵称和头像信息。
 */
class UserProfileService
{
public:
    /**
     * @brief 从本地持久化存储加载用户资料。
     */
    void load();

    /**
     * @brief 将当前用户资料保存到本地持久化存储。
     */
    void save();

    /**
     * @brief 获取当前用户昵称。
     * @return 当前保存的昵称文本。
     */
    QString nickname() const;

    /**
     * @brief 设置当前用户昵称。
     * @param[in] name 待保存的昵称文本。
     */
    void setNickname(const QString &name);

    /**
     * @brief 获取当前用户头像。
     * @return 当前保存的头像图像。
     */
    QImage avatar() const;

    /**
     * @brief 设置当前用户头像。
     * @param[in] img 待保存的头像图像。
     */
    void setAvatar(const QImage &img);

private:
    QString m_nickname; ///< 当前用户昵称。
    QImage  m_avatar;   ///< 当前用户头像图像。
};
