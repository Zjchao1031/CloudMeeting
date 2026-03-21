#pragma once
#include <QString>
#include <QImage>

class UserProfileService
{
public:
    void load();
    void save();
    QString nickname() const;
    void    setNickname(const QString &name);
    QImage  avatar() const;
    void    setAvatar(const QImage &img);

private:
    QString m_nickname;
    QImage  m_avatar;
};
