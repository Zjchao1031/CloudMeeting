/**
 * @file UserProfileService.cpp
 * @brief 实现用户资料持久化服务。
 */
#include "domain/service/UserProfileService.h"
#include <QStandardPaths>
#include <QSettings>

void UserProfileService::load()
{
    QSettings s(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                 + "/profile.ini", QSettings::IniFormat);
    m_nickname = s.value("nickname", "用户").toString();
}

void UserProfileService::save()
{
    QSettings s(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                 + "/profile.ini", QSettings::IniFormat);
    s.setValue("nickname", m_nickname);
}

QString UserProfileService::nickname() const { return m_nickname; }
void    UserProfileService::setNickname(const QString &name) { m_nickname = name; }
QImage  UserProfileService::avatar() const { return m_avatar; }
void    UserProfileService::setAvatar(const QImage &img) { m_avatar = img; }
