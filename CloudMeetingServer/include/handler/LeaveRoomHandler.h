#pragma once
#include "handler/ICommandHandler.h"

class LeaveRoomHandler : public ICommandHandler
{
public:
    void handle(int fd, const std::string &payload) override;
};
