#include "handler/CreateRoomHandler.h"
#include "service/RoomService.h"
#include "common/Logger.h"

void CreateRoomHandler::handle(int fd, const std::string &payload)
{
    // TODO: 解析 payload JSON，调用 RoomService::createRoom
    (void)fd; (void)payload;
    Logger::info("CreateRoomHandler::handle called");
}
