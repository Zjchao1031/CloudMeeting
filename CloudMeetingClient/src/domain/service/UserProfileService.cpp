/**
 * @file UserProfileService.cpp
 * @brief 实现用户资料持久化服务。
 */
#include "domain/service/UserProfileService.h"
#include <QCoreApplication>
#include <QSettings>
#include <QBuffer>

namespace {
QString profileFilePath()
{
    return QCoreApplication::applicationDirPath() + "/profile.ini";
}
}

void UserProfileService::load()
{
    QSettings s(profileFilePath(), QSettings::IniFormat);

    // 加载服务器配置。
    m_serverHost     = s.value("Server/host",         "").toString();
    m_serverTcpPort  = static_cast<quint16>(s.value("Server/tcpPort",       0).toUInt());
    m_udpAudioUpPort = static_cast<quint16>(s.value("Server/udpAudioUpPort", 0).toUInt());
    m_udpVideoUpPort = static_cast<quint16>(s.value("Server/udpVideoUpPort", 0).toUInt());

    m_nickname = s.value("nickname", "用户").toString();

    // 从持久化存储加载头像 Base64 数据。
    QString b64 = s.value("avatarBase64").toString();
    if (!b64.isEmpty()) {
        QByteArray raw = QByteArray::fromBase64(b64.toUtf8());
        QImage img;
        if (img.loadFromData(raw, "PNG")) {
            m_avatar       = img;
            m_avatarBase64 = b64;
        }
    }

    // 未设置头像时加载内置默认头像资源。
    if (m_avatar.isNull()) {
        QImage defaultAvatar(":/avatars/default.jpg");
        if (!defaultAvatar.isNull()) {
            m_avatar = cropAndScale(defaultAvatar);
            // 为默认头像也生成 Base64 缓存。
            QByteArray ba;
            QBuffer buf(&ba);
            buf.open(QIODevice::WriteOnly);
            m_avatar.save(&buf, "PNG");
            m_avatarBase64 = QString::fromUtf8(ba.toBase64());
        }
    }
}

void UserProfileService::save()
{
    QSettings s(profileFilePath(), QSettings::IniFormat);

    // 保存服务器配置。
    s.setValue("Server/host",         m_serverHost);
    s.setValue("Server/tcpPort",      m_serverTcpPort);
    s.setValue("Server/udpAudioUpPort", m_udpAudioUpPort);
    s.setValue("Server/udpVideoUpPort", m_udpVideoUpPort);

    s.setValue("nickname", m_nickname);
    s.setValue("avatarBase64", m_avatarBase64);
}

QString UserProfileService::nickname() const { return m_nickname; }
void    UserProfileService::setNickname(const QString &name) { m_nickname = name; }
QImage  UserProfileService::avatar() const { return m_avatar; }

void UserProfileService::setAvatar(const QImage &img)
{
    if (img.isNull()) return;

    // 裁剪为正方形并缩放至 64x64 像素。
    m_avatar = cropAndScale(img);

    // 同步更新 Base64 缓存。
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    m_avatar.save(&buf, "PNG");
    m_avatarBase64 = QString::fromUtf8(ba.toBase64());
}

QString UserProfileService::avatarBase64() const { return m_avatarBase64; }

QString UserProfileService::serverHost() const { return m_serverHost; }
void    UserProfileService::setServerHost(const QString &host) { m_serverHost = host; }
quint16 UserProfileService::serverTcpPort() const { return m_serverTcpPort; }
void    UserProfileService::setServerTcpPort(quint16 port) { m_serverTcpPort = port; }
quint16 UserProfileService::udpAudioUpPort() const { return m_udpAudioUpPort; }
void    UserProfileService::setUdpAudioUpPort(quint16 port) { m_udpAudioUpPort = port; }
quint16 UserProfileService::udpVideoUpPort() const { return m_udpVideoUpPort; }
void    UserProfileService::setUdpVideoUpPort(quint16 port) { m_udpVideoUpPort = port; }

QImage UserProfileService::cropAndScale(const QImage &src)
{
    if (src.isNull()) return src;

    // 居中裁剪为正方形。
    int side = qMin(src.width(), src.height());
    int ox   = (src.width()  - side) / 2;
    int oy   = (src.height() - side) / 2;
    QImage cropped = src.copy(ox, oy, side, side);

    // 缩放至 64x64 像素。
    return cropped.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
