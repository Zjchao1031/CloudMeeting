#include "service/MediaForwardService.h"
#include "domain/SessionManager.h"
#include "domain/RoomManager.h"
#include "common/Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>

MediaForwardService &MediaForwardService::instance()
{
    static MediaForwardService inst;
    return inst;
}

void MediaForwardService::setAudioDnFd(int fd) { m_audioDnFd = fd; }
void MediaForwardService::setVideoDnFd(int fd) { m_videoDnFd = fd; }

void MediaForwardService::forwardAudio(uint32_t userId, const uint8_t *data, size_t len,
                                        const sockaddr_in &srcAddr)
{
    // 更新发送方的 UDP 音频地址（用于下行转发回自己）
    SessionManager::instance().updateUdpAudioAddr(userId, srcAddr);

    ClientSession *sender = SessionManager::instance().findSessionByNumericId(userId);
    if (!sender || sender->roomId.empty() || m_audioDnFd < 0) return;

    Room *room = RoomManager::instance().findRoom(sender->roomId);
    if (!room) return;

    // 向同房间内其他成员转发（原样转发，保留包头）
    for (const auto &uid : room->memberIds) {
        if (uid == sender->userId) continue;
        ClientSession *member = SessionManager::instance().findSession(uid);
        if (!member || !member->udpAudioKnown) continue;
        ::sendto(m_audioDnFd, data, len, 0,
                 reinterpret_cast<const sockaddr*>(&member->udpAudioAddr),
                 sizeof(member->udpAudioAddr));
    }
}

void MediaForwardService::forwardVideo(uint32_t userId, const uint8_t *data, size_t len,
                                        const sockaddr_in &srcAddr)
{
    SessionManager::instance().updateUdpVideoAddr(userId, srcAddr);

    ClientSession *sender = SessionManager::instance().findSessionByNumericId(userId);
    if (!sender || sender->roomId.empty() || m_videoDnFd < 0) return;

    Room *room = RoomManager::instance().findRoom(sender->roomId);
    if (!room) return;

    for (const auto &uid : room->memberIds) {
        if (uid == sender->userId) continue;
        ClientSession *member = SessionManager::instance().findSession(uid);
        if (!member || !member->udpVideoKnown) continue;
        ::sendto(m_videoDnFd, data, len, 0,
                 reinterpret_cast<const sockaddr*>(&member->udpVideoAddr),
                 sizeof(member->udpVideoAddr));
    }
}
