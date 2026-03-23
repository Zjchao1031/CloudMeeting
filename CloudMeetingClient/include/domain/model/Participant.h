#pragma once
#include <QString>
#include <QImage>

/**
 * @file Participant.h
 * @brief 定义会议参与者数据模型。
 */

/**
 * @struct Participant
 * @brief 描述单个参会用户的身份与媒体状态。
 */
struct Participant
{
    QString userId;             ///< 用户唯一标识。
    QString nickname;           ///< 用户显示昵称。
    QString avatarBase64;       ///< 用户头像 Base64 编码字符串。
    QImage  avatar;             ///< 用户头像图像（由 avatarBase64 解码得到）。
    bool    isHost        = false; ///< 是否为主持人。
    bool    cameraOn      = false; ///< 摄像头是否开启。
    bool    micOn         = false; ///< 麦克风是否开启。
    bool    screenShareOn = false; ///< 是否正在共享屏幕。
};
