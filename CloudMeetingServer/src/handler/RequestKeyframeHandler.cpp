#include "handler/RequestKeyframeHandler.h"
#include "service/BroadcastService.h"
#include "domain/SessionManager.h"
#include "protocol/SignalType.h"
#include "common/Logger.h"
#include "json.hpp"

void RequestKeyframeHandler::handle(int fd, const std::string &payload)
{
    auto j = nlohmann::json::parse(payload, nullptr, false);
    if (j.is_discarded()) { Logger::warn("RequestKeyframe: invalid JSON"); return; }

    std::string targetUserId = j.value("target_user_id", "");
    if (targetUserId.empty()) return;

    ClientSession *requester = SessionManager::instance().findSessionByFd(fd);
    ClientSession *target    = SessionManager::instance().findSession(targetUserId);
    if (!requester || !target) return;

    // 转发 REQUEST_KEYFRAME 给目标发送方，告知谁在请求关键帧
    nlohmann::json fwd;
    fwd["user_id"] = requester->userId;
    BroadcastService::instance().sendTo(target->tcpFd, SignalType::REQUEST_KEYFRAME, fwd.dump());
}
