#pragma once
#include "domain/model/RoomInfo.h"

struct CreateRoomOptions
{
    int     maxMembers  = 10;
    bool    hasPassword = false;
    QString password;
    QString nickname;
};

struct JoinRoomOptions
{
    QString roomId;
    QString password;
    QString nickname;
};

class NetworkFacade;

class MeetingController
{
public:
    MeetingController();
    void setNetworkFacade(NetworkFacade *facade);

    void createRoom(const CreateRoomOptions &opts);
    void joinRoom(const JoinRoomOptions &opts);
    void leaveRoom();
    void onRoomClosed();

private:
    NetworkFacade *m_network = nullptr;
    RoomInfo       m_currentRoom;
};
