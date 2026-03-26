#include "handler/ChatMessageHandler.h"
#include "service/BroadcastService.h"
#include "domain/SessionManager.h"
#include "protocol/SignalType.h"
#include "common/TimeUtil.h"
#include "common/Logger.h"
#include "json.hpp"

void ChatMessageHandler::handle(int fd, const std::string &payload)
{
    auto j = nlohmann::json::parse(payload, nullptr, false);
    if (j.is_discarded()) { Logger::warn("ChatMessage: invalid JSON"); return; }

    std::string content = j.value("content", "");

    // 校验 UTF-8 字节长度不超过 1024
    if (content.size() > 1024) {
        Logger::warn("ChatMessage: content exceeds 1024 bytes, dropped");
        return;
    }

    ClientSession *session = SessionManager::instance().findSessionByFd(fd);
    if (!session || session->roomId.empty()) return;

    nlohmann::json broadcast;
    broadcast["user_id"]   = session->userId;
    broadcast["nickname"]  = session->nickname;
    broadcast["content"]   = content;
    broadcast["timestamp"] = TimeUtil::nowSeconds();

    BroadcastService::instance().broadcastToRoom(
        session->roomId, SignalType::CHAT_BROADCAST, broadcast.dump());
}
