#include "handler/JoinRoomHandler.h"
#include "service/RoomService.h"
#include "service/BroadcastService.h"
#include "domain/RoomManager.h"
#include "domain/SessionManager.h"
#include "protocol/SignalType.h"
#include "common/Logger.h"
#include "json.hpp"

void JoinRoomHandler::handle(int fd, const std::string &payload)
{
    auto j = nlohmann::json::parse(payload, nullptr, false);
    if (j.is_discarded()) { Logger::warn("JoinRoom: invalid JSON"); return; }

    std::string roomId   = j.value("room_id",  "");
    std::string nickname = j.value("nickname", "Unknown");
    std::string password = j.value("password", "");

    int code = RoomService::instance().joinRoom(fd, roomId, password, nickname);
    if (code != 0) {
        nlohmann::json ack;
        ack["code"] = code;
        BroadcastService::instance().sendTo(fd, SignalType::JOIN_ROOM_ACK, ack.dump());
        return;
    }

    ClientSession *self = SessionManager::instance().findSessionByFd(fd);
    if (!self) return;

    Room *room = RoomManager::instance().findRoom(roomId);
    if (!room) return;

    // 向新成员发送 JOIN_ROOM_ACK（含房间和自身信息）
    nlohmann::json ack;
    ack["code"]       = 0;
    ack["room_id"]    = roomId;
    ack["user_id"]    = self->userId;
    ack["is_host"]    = false;
    ack["host_id"]    = room->hostUserId;
    ack["numeric_id"] = self->numericId;
    BroadcastService::instance().sendTo(fd, SignalType::JOIN_ROOM_ACK, ack.dump());

    // 将已有成员信息逐条推送给新成员
    for (const auto &uid : room->memberIds) {
        if (uid == self->userId) continue;
        ClientSession *member = SessionManager::instance().findSession(uid);
        if (!member) continue;
        nlohmann::json mj;
        mj["user_id"]    = member->userId;
        mj["nickname"]   = member->nickname;
        mj["is_host"]    = room->isHost(uid);
        mj["numeric_id"] = member->numericId;
        mj["camera"]     = member->cameraOn;
        mj["mic"]        = member->micOn;
        mj["screen"]     = member->screenOn;
        BroadcastService::instance().sendTo(fd, SignalType::MEMBER_JOIN, mj.dump());
    }

    // 向房间内已有成员广播新成员加入
    nlohmann::json newMember;
    newMember["user_id"]    = self->userId;
    newMember["nickname"]   = self->nickname;
    newMember["is_host"]    = false;
    newMember["numeric_id"] = self->numericId;
    newMember["camera"]     = false;
    newMember["mic"]        = false;
    newMember["screen"]     = false;
    BroadcastService::instance().broadcastToRoom(
        roomId, SignalType::MEMBER_JOIN, newMember.dump(), fd);
}
