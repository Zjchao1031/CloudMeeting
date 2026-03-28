#pragma once
#include <string>

class RoomService
{
public:
    static RoomService &instance();

    // 创建房间，返回 room_id
    std::string createRoom(int fd, int maxMembers, bool hasPassword,
                           const std::string &password, const std::string &nickname,
                           const std::string &avatarBase64 = "");
    // 加入房间，返回错误码：0=成功,1=不存在,2=密码错误,3=已满
    int joinRoom(int fd, const std::string &roomId,
                 const std::string &password, const std::string &nickname,
                 const std::string &avatarBase64 = "");
    // 离开房间（普通成员或主持人）
    void leaveRoom(int fd);

private:
    RoomService() = default;
};
