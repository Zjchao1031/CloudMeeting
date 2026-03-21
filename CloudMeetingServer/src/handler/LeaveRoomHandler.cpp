#include "handler/LeaveRoomHandler.h"
#include "service/RoomService.h"
#include "common/Logger.h"

void LeaveRoomHandler::handle(int fd, const std::string &payload)
{
    // TODO: 调用 RoomService::leaveRoom，广播 MEMBER_LEAVE 或 ROOM_CLOSED
    (void)fd; (void)payload;
    Logger::info("LeaveRoomHandler::handle called");
}
