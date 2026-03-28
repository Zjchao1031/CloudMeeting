/**
 * @file AppContext.cpp
 * @brief 实现应用上下文单例。
 */
#include "app/AppContext.h"
#include "domain/service/UserProfileService.h"
#include "domain/service/ParticipantRepository.h"
#include "domain/service/MeetingController.h"
#include "domain/service/ChatService.h"
#include "network/NetworkFacade.h"
#include "media/MediaEngine.h"
#include "media/device/DeviceManager.h"
#include "network/MediaUdpClient.h"

AppContext &AppContext::instance()
{
    static AppContext ctx;
    return ctx;
}

AppContext::~AppContext()
{
    // 先停止所有媒体采集，确保工作线程退出、FFmpeg 资源释放。
    // unique_ptr 成员按声明逆序自动析构，析构顺序已在头文件中通过声明顺序保证。
    if (m_mediaEngine)  m_mediaEngine->stopAll();
}

void AppContext::setup()
{
    // 创建业务服务实例。
    m_profileService  = std::make_unique<UserProfileService>();
    m_participantRepo = std::make_unique<ParticipantRepository>();
    m_meetingCtrl     = std::make_unique<MeetingController>();
    m_chatService     = std::make_unique<ChatService>();

    // 创建网络门面。
    m_networkFacade   = std::make_unique<NetworkFacade>();

    // 注入依赖：将 NetworkFacade 注入各业务服务。
    m_meetingCtrl->setParticipantRepository(m_participantRepo.get());

    // 成员离开时移除其解码器——必须在 setNetworkFacade（即 MeetingController::onMemberLeaveFromNetwork
    // 连接）之前注册，确保此 lambda 先执行，此时仓库内还能查到该成员的 numericId。
    QObject::connect(m_networkFacade.get(), &NetworkFacade::memberLeft,
                     [this](const QString &userId) {
        auto *repo = m_participantRepo.get();
        if (repo) {
            const auto parts = repo->sortedParticipants();
            for (const auto &p : parts) {
                if (p.userId == userId && p.numericId != 0) {
                    m_mediaEngine->removeUserDecoder(QString::number(p.numericId));
                    return;
                }
            }
        }
        m_mediaEngine->removeUserDecoder(userId);
    });

    m_meetingCtrl->setNetworkFacade(m_networkFacade.get());
    m_chatService->setNetworkFacade(m_networkFacade.get());

    // 创建设备管理器并枚举设备。
    m_deviceManager = std::make_unique<DeviceManager>();
    m_deviceManager->enumerateDevices();

    // 创建媒体引擎并绑定网络门面。
    m_mediaEngine = std::make_unique<MediaEngine>();
    m_mediaEngine->setNetworkFacade(m_networkFacade.get(), QString{});

    // 进入会议时同步本地 numericId 到媒体引擎，并向服务器注册 UDP 地址。
    QObject::connect(m_meetingCtrl.get(), &MeetingController::meetingEntered,
                     m_mediaEngine.get(), [this](const RoomInfo &) {
        const quint32 nid = m_meetingCtrl->localNumericId();
        m_mediaEngine->setLocalNumericId(nid);
        auto *media = m_networkFacade->mediaClient();
        if (media) media->sendUdpRegistration(nid);
    });

    // 收到关键帧请求时强制编码器输出 IDR 帧。
    QObject::connect(m_networkFacade.get(), &NetworkFacade::keyframeRequested,
                     m_mediaEngine.get(),   &MediaEngine::forceVideoKeyFrame);

    // 离开/关闭会议时停止媒体采集。
    QObject::connect(m_meetingCtrl.get(), &MeetingController::meetingExited,
                     m_mediaEngine.get(),  &MediaEngine::stopAll);
    QObject::connect(m_meetingCtrl.get(), &MeetingController::roomClosed,
                     m_mediaEngine.get(),  &MediaEngine::stopAll);

    // 加载用户资料（昵称 + 头像 + 服务器配置）。
    m_profileService->load();

    // 将 profile.ini 中读取的服务器地址和端口注入会议控制器。
    m_meetingCtrl->setServerConfig(m_profileService->serverHost(),
                                   m_profileService->serverTcpPort());
}

UserProfileService*    AppContext::userProfileService()    const { return m_profileService.get(); }
ParticipantRepository* AppContext::participantRepository() const { return m_participantRepo.get(); }
MeetingController*     AppContext::meetingController()     const { return m_meetingCtrl.get(); }
ChatService*           AppContext::chatService()           const { return m_chatService.get(); }
NetworkFacade*         AppContext::networkFacade()         const { return m_networkFacade.get(); }
MediaEngine*           AppContext::mediaEngine()           const { return m_mediaEngine.get(); }
DeviceManager*         AppContext::deviceManager()         const { return m_deviceManager.get(); }
