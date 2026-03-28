#include "handler/CreateRoomHandler.h"
#include "service/RoomService.h"
#include "service/BroadcastService.h"
#include "domain/SessionManager.h"
#include "protocol/SignalType.h"
#include "common/Logger.h"
#include "json.hpp"

void CreateRoomHandler::handle(int fd, const std::string &payload)
{
    auto j = nlohmann::json::parse(payload, nullptr, false);
    if (j.is_discarded()) { Logger::warn("CreateRoom: invalid JSON"); return; }

    std::string nickname     = j.value("nickname",      "Unknown");
    std::string avatarBase64 = j.value("avatar_base64", "");
    int         maxMembers   = j.value("max_members",   10);
    bool        hasPassword  = j.value("has_password",  false);
    std::string password     = j.value("password",      "");

    std::string roomId = RoomService::instance().createRoom(
        fd, maxMembers, hasPassword, password, nickname, avatarBase64);

    ClientSession *session = SessionManager::instance().findSessionByFd(fd);
    if (!session) return;

    nlohmann::json ack;
    ack["success"]    = true;
    ack["room_id"]    = roomId;
    ack["user_id"]    = session->userId;
    ack["is_host"]    = true;
    ack["numeric_id"] = session->numericId;

    BroadcastService::instance().sendTo(fd, SignalType::CREATE_ROOM_ACK, ack.dump());
}
