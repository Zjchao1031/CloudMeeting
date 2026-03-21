#include "handler/MediaStateHandler.h"
#include "service/BroadcastService.h"
#include "common/Logger.h"

void MediaStateHandler::handle(int fd, const std::string &payload)
{
    // TODO: 更新 ClientSession 媒体状态，广播 MEDIA_STATE_SYNC
    (void)fd; (void)payload;
    Logger::info("MediaStateHandler::handle called");
}
