#pragma once
#include <string>

// 所有信令处理器的统一接口（命令模式抽象基类）
class ICommandHandler
{
public:
    virtual ~ICommandHandler() = default;
    // fd: 发送该信令的客户端 TCP fd
    // payload: 已解析的 JSON 字符串
    virtual void handle(int fd, const std::string &payload) = 0;
};
