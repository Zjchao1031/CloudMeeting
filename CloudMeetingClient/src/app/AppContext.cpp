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
    m_meetingCtrl->setNetworkFacade(m_networkFacade.get());
    m_chatService->setNetworkFacade(m_networkFacade.get());

    // 创建设备管理器并枚举设备。
    m_deviceManager = std::make_unique<DeviceManager>();
    m_deviceManager->enumerateDevices();

    // 创建媒体引擎并绑定网络门面。
    m_mediaEngine = std::make_unique<MediaEngine>();
    m_mediaEngine->setNetworkFacade(m_networkFacade.get(), QString{});

    // 进入会议时同步本地 userId 到媒体引擎。
    QObject::connect(m_meetingCtrl.get(), &MeetingController::meetingEntered,
                     m_mediaEngine.get(), [this](const RoomInfo &) {
        m_mediaEngine->setLocalUserId(m_meetingCtrl->localUserId());
    });

    // 成员离开时移除其对应的视频解码器。
    QObject::connect(m_networkFacade.get(), &NetworkFacade::memberLeft,
                     m_mediaEngine.get(),   &MediaEngine::removeUserDecoder);

    // 离开/关闭会议时停止媒体采集。
    QObject::connect(m_meetingCtrl.get(), &MeetingController::meetingExited,
                     m_mediaEngine.get(),  &MediaEngine::stopAll);
    QObject::connect(m_meetingCtrl.get(), &MeetingController::roomClosed,
                     m_mediaEngine.get(),  &MediaEngine::stopAll);

    // 加载用户资料（昵称 + 头像）。
    m_profileService->load();
}

UserProfileService*    AppContext::userProfileService()    const { return m_profileService.get(); }
ParticipantRepository* AppContext::participantRepository() const { return m_participantRepo.get(); }
MeetingController*     AppContext::meetingController()     const { return m_meetingCtrl.get(); }
ChatService*           AppContext::chatService()           const { return m_chatService.get(); }
NetworkFacade*         AppContext::networkFacade()         const { return m_networkFacade.get(); }
MediaEngine*           AppContext::mediaEngine()           const { return m_mediaEngine.get(); }
DeviceManager*         AppContext::deviceManager()         const { return m_deviceManager.get(); }
