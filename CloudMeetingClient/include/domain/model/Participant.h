#pragma once
#include <QString>
#include <QImage>

struct Participant
{
    QString userId;
    QString nickname;
    QImage  avatar;
    bool    isHost       = false;
    bool    cameraOn     = false;
    bool    micOn        = false;
    bool    screenShareOn = false;
};
