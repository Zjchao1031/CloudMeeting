#include "handler/RequestKeyframeHandler.h"
#include "common/Logger.h"

void RequestKeyframeHandler::handle(int fd, const std::string &payload)
{
    // TODO: 通知对应发送方编码一帧 IDR 关键帧
    (void)fd; (void)payload;
    Logger::info("RequestKeyframeHandler::handle called");
}
