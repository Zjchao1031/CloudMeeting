#pragma once
#include "handler/ICommandHandler.h"

class HeartbeatHandler : public ICommandHandler
{
public:
    void handle(int fd, const std::string &payload) override;
};
