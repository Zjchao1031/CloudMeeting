#pragma once
#include "handler/ICommandHandler.h"

class ChatMessageHandler : public ICommandHandler
{
public:
    void handle(int fd, const std::string &payload) override;
};
