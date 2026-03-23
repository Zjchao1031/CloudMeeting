#pragma once
#include <QString>
#include <QImage>
#include <QByteArray>

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
     * @brief 设置当前用户头像（自动裁剪为 64x64 正方形）。
     * @param[in] img 待保存的头像图像。
     */
    void setAvatar(const QImage &img);

    /**
     * @brief 获取当前用户头像的 Base64 编码字符串。
     * @return PNG 格式头像的 Base64 编码。
     */
    QString avatarBase64() const;

private:
    /**
     * @brief 将头像裁剪为正方形并缩放至 64x64 像素。
     * @param[in] src 原始头像图像。
     * @return 裁剪缩放后的头像图像。
     */
    static QImage cropAndScale(const QImage &src);

    QString m_nickname;      ///< 当前用户昵称。
    QImage  m_avatar;        ///< 当前用户头像图像（64x64）。
    QString m_avatarBase64;  ///< 头像的 Base64 编码缓存。
};
