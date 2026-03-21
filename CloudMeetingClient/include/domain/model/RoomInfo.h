#pragma once
#include <QString>

struct RoomInfo
{
    QString roomId;
    int     maxMembers  = 10;
    bool    hasPassword = false;
    bool    isHost      = false;
};
