#include "handler/HeartbeatHandler.h"
#include "domain/SessionManager.h"
#include "service/BroadcastService.h"
#include "common/Logger.h"

void HeartbeatHandler::handle(int fd, const std::string &payload)
{
    // 更新心跳时间戳，回复 HEARTBEAT_ACK
    (void)payload;
    auto *session = SessionManager::instance().findSessionByFd(fd);
    if (session)
        SessionManager::instance().updateHeartbeat(session->userId);
    BroadcastService::instance().sendTo(fd, SignalType::HEARTBEAT_ACK, "{}");
}
