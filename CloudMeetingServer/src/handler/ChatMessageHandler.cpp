#include "handler/ChatMessageHandler.h"
#include "service/BroadcastService.h"
#include "common/Logger.h"

void ChatMessageHandler::handle(int fd, const std::string &payload)
{
    // TODO: 解析消息，广播 CHAT_BROADCAST 给同房间成员
    (void)fd; (void)payload;
    Logger::info("ChatMessageHandler::handle called");
}
