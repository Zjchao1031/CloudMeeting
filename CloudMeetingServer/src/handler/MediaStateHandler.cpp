#include "handler/MediaStateHandler.h"
#include "service/BroadcastService.h"
#include "domain/SessionManager.h"
#include "protocol/SignalType.h"
#include "common/Logger.h"
#include "json.hpp"

void MediaStateHandler::handle(int fd, const std::string &payload)
{
    auto j = nlohmann::json::parse(payload, nullptr, false);
    if (j.is_discarded()) { Logger::warn("MediaState: invalid JSON"); return; }

    ClientSession *session = SessionManager::instance().findSessionByFd(fd);
    if (!session) return;

    session->cameraOn = j.value("camera",       session->cameraOn);
    session->micOn    = j.value("microphone",   session->micOn);
    session->screenOn = j.value("screen_share", session->screenOn);

    nlohmann::json sync;
    sync["user_id"]       = session->userId;
    sync["camera"]        = session->cameraOn;
    sync["microphone"]    = session->micOn;
    sync["screen_share"]  = session->screenOn;

    BroadcastService::instance().broadcastToRoom(
        session->roomId, SignalType::MEDIA_STATE_SYNC, sync.dump(), fd);
}
