#include "handler/LeaveRoomHandler.h"
#include "service/RoomService.h"
#include "common/Logger.h"

void LeaveRoomHandler::handle(int fd, const std::string &payload)
{
    (void)payload;
    // RoomService::leaveRoom 内部已处理广播 MEMBER_LEAVE / ROOM_CLOSED
    RoomService::instance().leaveRoom(fd);
}
