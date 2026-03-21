#include "service/MediaForwardService.h"
#include "domain/SessionManager.h"
#include "domain/RoomManager.h"
#include <sys/socket.h>
#include <netinet/in.h>

MediaForwardService &MediaForwardService::instance()
{
    static MediaForwardService inst;
    return inst;
}

void MediaForwardService::forwardAudio(uint32_t userId, const uint8_t *data, size_t len)
{
    // TODO: 根据 userId 查找房间，向其他成员下行 UDP 端口转发
    (void)userId; (void)data; (void)len;
}

void MediaForwardService::forwardVideo(uint32_t userId, const uint8_t *data, size_t len)
{
    // TODO: 同音频转发逻辑，区分摄像头流与屏幕共享流
    (void)userId; (void)data; (void)len;
}
