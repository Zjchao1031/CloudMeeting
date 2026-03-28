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

    std::string roomId       = j.value("room_id",      "");
    std::string nickname     = j.value("nickname",     "Unknown");
    std::string password     = j.value("password",     "");
    std::string avatarBase64 = j.value("avatar_base64", "");

    int code = RoomService::instance().joinRoom(fd, roomId, password, nickname, avatarBase64);
    if (code != 0) {
        nlohmann::json ack;
        ack["success"] = false;
        if (code == 1)      ack["error"] = "\u623f\u95f4\u4e0d\u5b58\u5728";
        else if (code == 2) ack["error"] = "\u5bc6\u7801\u9519\u8bef";
        else if (code == 3) ack["error"] = "\u623f\u95f4\u5df2\u6ee1";
        BroadcastService::instance().sendTo(fd, SignalType::JOIN_ROOM_ACK, ack.dump());
        return;
    }

    ClientSession *self = SessionManager::instance().findSessionByFd(fd);
    if (!self) return;

    Room *room = RoomManager::instance().findRoom(roomId);
    if (!room) return;

    // 向新成员发送 JOIN_ROOM_ACK（含房间和自身信息）
    nlohmann::json ack;
    ack["success"]       = true;
    ack["room_id"]       = roomId;
    ack["host_user_id"]  = room->hostUserId;
    ack["user_id"]       = self->userId;
    ack["numeric_id"]    = self->numericId;
    BroadcastService::instance().sendTo(fd, SignalType::JOIN_ROOM_ACK, ack.dump());

    // 将已有成员信息逐条推送给新成员（含媒体状态和 numeric_id）
    for (const auto &uid : room->memberIds) {
        if (uid == self->userId) continue;
        ClientSession *member = SessionManager::instance().findSession(uid);
        if (!member) continue;
        nlohmann::json mj;
        mj["user_id"]       = member->userId;
        mj["nickname"]      = member->nickname;
        mj["avatar_base64"] = member->avatarBase64;
        mj["is_host"]       = room->isHost(uid);
        mj["numeric_id"]    = member->numericId;
        mj["camera"]        = member->cameraOn;
        mj["microphone"]    = member->micOn;
        mj["screen_share"]  = member->screenOn;
        BroadcastService::instance().sendTo(fd, SignalType::MEMBER_JOIN, mj.dump());
    }

    // 向房间内已有成员广播新成员加入
    nlohmann::json newMember;
    newMember["user_id"]       = self->userId;
    newMember["nickname"]      = self->nickname;
    newMember["avatar_base64"] = self->avatarBase64;
    newMember["is_host"]       = false;
    newMember["numeric_id"]    = self->numericId;
    BroadcastService::instance().broadcastToRoom(
        roomId, SignalType::MEMBER_JOIN, newMember.dump(), fd);
}
