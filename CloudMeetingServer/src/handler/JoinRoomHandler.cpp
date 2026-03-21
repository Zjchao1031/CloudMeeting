#include "handler/JoinRoomHandler.h"
#include "service/RoomService.h"
#include "common/Logger.h"

void JoinRoomHandler::handle(int fd, const std::string &payload)
{
    // TODO: 解析 payload JSON，调用 RoomService::joinRoom
    (void)fd; (void)payload;
    Logger::info("JoinRoomHandler::handle called");
}
