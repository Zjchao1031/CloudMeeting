#include "service/BroadcastService.h"
#include "service/RoomService.h"
#include "domain/RoomManager.h"
#include "domain/SessionManager.h"
#include "protocol/SignalCodec.h"
#include "common/Logger.h"
#include <sys/socket.h>

BroadcastService &BroadcastService::instance()
{
    static BroadcastService inst;
    return inst;
}

void BroadcastService::sendTo(int fd, SignalType type, const std::string &jsonPayload)
{
    auto buf = SignalCodec::encode(type, jsonPayload);
    ::send(fd, buf.data(), buf.size(), MSG_NOSIGNAL);
}

void BroadcastService::broadcastToRoom(const std::string &roomId, SignalType type,
                                        const std::string &jsonPayload, int excludeFd)
{
    Room *room = RoomManager::instance().findRoom(roomId);
    if (!room) return;
    auto buf = SignalCodec::encode(type, jsonPayload);
    for (const auto &uid : room->memberIds) {
        ClientSession *s = SessionManager::instance().findSession(uid);
        if (s && s->tcpFd != excludeFd)
            ::send(s->tcpFd, buf.data(), buf.size(), MSG_NOSIGNAL);
    }
}
