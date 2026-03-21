#pragma once
#include "handler/ICommandHandler.h"

class RequestKeyframeHandler : public ICommandHandler
{
public:
    void handle(int fd, const std::string &payload) override;
};
