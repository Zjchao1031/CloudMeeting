#include "service/RoomService.h"
#include "service/BroadcastService.h"
#include "domain/RoomManager.h"
#include "domain/SessionManager.h"
#include "common/IdGenerator.h"
#include "common/Logger.h"

RoomService &RoomService::instance()
{
    static RoomService inst;
    return inst;
}

std::string RoomService::createRoom(int fd, int maxMembers, bool hasPassword,
                                     const std::string &password,
                                     const std::string &nickname)
{
    std::string userId = IdGenerator::generateUserId();
    std::string roomId = RoomManager::instance().createRoom(
        maxMembers, hasPassword, password, userId);

    ClientSession session;
    session.userId    = userId;
    session.nickname  = nickname;
    session.roomId    = roomId;
    session.tcpFd     = fd;
    SessionManager::instance().addSession(session);

    Logger::info("Room created: " + roomId + " by " + nickname);
    return roomId;
}

int RoomService::joinRoom(int fd, const std::string &roomId,
                          const std::string &password,
                          const std::string &nickname)
{
    Room *room = RoomManager::instance().findRoom(roomId);
    if (!room)             return 1; // 房间不存在
    if (room->isFull())   return 3; // 已满
    if (room->hasPassword && room->password != password) return 2; // 密码错误

    std::string userId = IdGenerator::generateUserId();
    room->memberIds.insert(userId);

    ClientSession session;
    session.userId   = userId;
    session.nickname = nickname;
    session.roomId   = roomId;
    session.tcpFd    = fd;
    SessionManager::instance().addSession(session);

    Logger::info(nickname + " joined room " + roomId);
    return 0;
}

void RoomService::leaveRoom(int fd)
{
    ClientSession *session = SessionManager::instance().findSessionByFd(fd);
    if (!session) return;

    Room *room = RoomManager::instance().findRoom(session->roomId);
    if (room) {
        bool isHost = room->isHost(session->userId);
        room->memberIds.erase(session->userId);
        if (isHost || room->memberIds.empty()) {
            // 主持人退出：广播 ROOM_CLOSED，销毁房间
            BroadcastService::instance().broadcastToRoom(
                session->roomId, SignalType::ROOM_CLOSED, "{}", fd);
            RoomManager::instance().destroyRoom(session->roomId);
        } else {
            // 普通成员退出：广播 MEMBER_LEAVE
            std::string payload = R"({"user_id":")"
                + session->userId + R"("})";
            BroadcastService::instance().broadcastToRoom(
                session->roomId, SignalType::MEMBER_LEAVE, payload, fd);
        }
    }
    SessionManager::instance().removeSession(session->userId);
}
